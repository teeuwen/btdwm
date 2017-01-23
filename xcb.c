/*
 *
 * © 2006-2010 Anselm R Garbe <anselm@garbe.us>
 * © 2006-2007 Sander van Dijk <a dot h dot vandijk at gmail dot com>
 * © 2006-2009 Jukka Salmi <jukka at salmi dot ch>
 * © 2007-2009 Premysl Hruby <dfenze at gmail dot com>
 * © 2007-2009 Szabolcs Nagy <nszabolcs at gmail dot com>
 * © 2007-2009 Christof Musik <christof at sendfax dot de>
 * © 2007-2008 Enno Gottox Boland <gottox at s01 dot de>
 * © 2007-2008 Peter Hartlich <sgkkr at hartlich dot com>
 * © 2008 Martin Hurton <martin dot hurton at gmail dot com>
 * © 2008 Neale Pickett <neale dot woozle dot org>
 * © 2009 Mate Nagy <mnagy@port70.net>
 * © 2012-2015 Julian Goldsmith <julian@juliangoldsmith.com>
 * © 2016-2017 Bastiaan Teeuwen <bastiaan.teeuwen170@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xinerama.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "btdwm.h"
#include "keysym.h"

#define BUTTONMASK	(XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE)
#define MOUSEMASK	(BUTTONMASK | XCB_EVENT_MASK_POINTER_MOTION)
#define CLEANMASK(m)	(m & ~(numlockmask | XCB_MOD_MASK_LOCK))

xcb_connection_t *conn;
static xcb_screen_t *screen;
xcb_window_t root;

static xcb_generic_error_t *err;

static unsigned int numlockmask;

xcb_atom_t atoms[ATOM_MAX];
static xcb_cursor_t cursor[CUR_MAX];
static xcb_key_symbols_t *syms;

static int (*xcb_handlers[XCB_NO_OPERATION]) (xcb_generic_event_t *);

static void _testerr(const char* file, const int line)
{
	if (!err)
		return;

	fprintf(stderr, "%s:%d - request returned error %i (%s)\n",
			file, line, (int) err->error_code,
			xcb_event_get_error_label(err->error_code));

	free(err);

	exit(1);
}
#define testerr() _testerr(__FILE__, __LINE__);

static inline void _testcookie(xcb_void_cookie_t cookie, const char* file, const int line)
{
	err = xcb_request_check(conn, cookie);
	_testerr(file, line);
}
#define testcookie(cookie) _testcookie(cookie, __FILE__, __LINE__);

void bars_update(void)
{
	xcb_visualtype_t *visual;
	xcb_depth_iterator_t di;
	xcb_visualtype_iterator_t vi;
	struct monitor *m;

	uint32_t values[] = {
		1,
		XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_EXPOSURE,
		cursor[CUR_NORMAL]
	};

	for (di = xcb_screen_allowed_depths_iterator(screen); di.rem;
			xcb_depth_next(&di)) {
		for (vi = xcb_depth_visuals_iterator(di.data); vi.rem;
				xcb_visualtype_next(&vi)) {
			if (screen->root_visual == vi.data->visual_id) {
				visual = vi.data;
				break;
			}
		}
	}

	if (!visual)
		die("unable to retrieve visual type\n");

	for (m = mons; m; m = m->next) {
		m->barwin = xcb_generate_id(conn);
		xcb_create_window(conn, XCB_COPY_FROM_PARENT, m->barwin, root,
				m->x, (m->showbar) ? 0 : -16, m->w, 16, 0,
				XCB_WINDOW_CLASS_INPUT_OUTPUT,
				screen->root_visual, XCB_CW_OVERRIDE_REDIRECT |
				XCB_CW_EVENT_MASK | XCB_CW_CURSOR, values);
		xcb_map_window(conn, m->barwin);

		/* TODO Configure bar height in config.h */
		m->barsur = cairo_xcb_surface_create(conn, m->barwin, visual,
				m->w, 16);
		if (cairo_surface_status(m->barsur) != 0)
			die("unable to create cairo surface\n");
		m->barcr = cairo_create(m->barsur);
	}
}

void numlockmask_update(void)
{
	xcb_get_modifier_mapping_reply_t* reply;
	xcb_keycode_t *codes, target, *temp;
	unsigned int i, j;

	reply = xcb_get_modifier_mapping_reply(conn,
			xcb_get_modifier_mapping(conn), &err);
	testerr();

	codes = xcb_get_modifier_mapping_keycodes(reply);

	if (!(temp = xcb_key_symbols_get_keycode(syms, K_NUML)))
		return;

	target = *temp;
	free(temp);

	for (i = 0; i < 8; i++)
		for (j = 0; j < reply->keycodes_per_modifier; j++)
			if (codes[i * reply->keycodes_per_modifier + j] ==
					target)
				numlockmask = (1 << i);

	free(reply);
}

static void title_update(struct client *c)
{
	if (!textprop_get(c->win, atoms[ATOM_NAME], c->name, sizeof(c->name)))
		if (!textprop_get(c->win,
				XCB_ATOM_WM_NAME, c->name, sizeof(c->name)))
			strcpy(c->name, "broken");
}

void configure(struct client *c)
{
	xcb_configure_notify_event_t ce;

	ce.response_type = XCB_CONFIGURE_NOTIFY;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = 0;
	ce.above_sibling = 0;
	ce.override_redirect = 0;

	xcb_send_event(conn, 0, c->win, XCB_EVENT_MASK_STRUCTURE_NOTIFY,
			(const char *) &ce);
}

void scan(void)
{
	unsigned int i, num;
	xcb_window_t *wins = NULL;

	xcb_query_tree_reply_t *query_reply = xcb_query_tree_reply(conn, xcb_query_tree(conn, root), &err);
	num = query_reply->children_len;
	wins = xcb_query_tree_children(query_reply);

	for (i = 0; i < num; i++) {
		xcb_get_window_attributes_reply_t *ga_reply =
			xcb_get_window_attributes_reply(conn, xcb_get_window_attributes(conn, wins[i]), &err);
		testerr();

		if (ga_reply->override_redirect)
			continue;

		xcb_window_t trans_reply = 0;
		xcb_icccm_get_wm_transient_for_reply(conn, xcb_icccm_get_wm_transient_for(conn, wins[i]), &trans_reply, &err);
		testerr();
		
		if (trans_reply != 0)
			continue;

		if (ga_reply->map_state == XCB_MAP_STATE_VIEWABLE ||
			atom_get(wins[i], atoms[ATOM_STATE]) == XCB_ICCCM_WM_STATE_ICONIC)
			manage(wins[i]);

		free(ga_reply);
	}

	for (i = 0; i < num; i++) {
		xcb_get_window_attributes_reply_t *ga_reply =
			xcb_get_window_attributes_reply(conn, xcb_get_window_attributes(conn, wins[i]), &err);
		testerr();

		xcb_window_t trans_reply = 0;
		xcb_icccm_get_wm_transient_for_reply(conn,
				xcb_icccm_get_wm_transient_for(conn,
				wins[i]), &trans_reply, &err);
		testerr();

		if (trans_reply && (ga_reply->map_state ==
				XCB_MAP_STATE_VIEWABLE ||
				atom_get(wins[i], atoms[ATOM_STATE]) ==
				XCB_ICCCM_WM_STATE_ICONIC))
			manage(wins[i]);

		free(ga_reply);
	}

	if (query_reply)
		free(query_reply);

	xcb_flush(conn);
}

static void rules_apply(struct client *c)
{
	xcb_icccm_get_wm_class_reply_t ch;
	struct monitor *m;
	const struct rule *r;
	const char *class, *instance;
	unsigned int i;

	c->floating = c->tags = 0;

	if (xcb_icccm_get_wm_class_reply(conn, xcb_icccm_get_wm_class(conn,
			c->win), &ch, 0)) {
		class = ch.class_name ? ch.class_name : "broken";
		instance = ch.instance_name ? ch.instance_name : "broken";
		for (i = 0; i < rules_len; i++) {
			r = &rules[i];
			if ((!r->title || strstr(c->name, r->title)) &&
					(!r->class ||
					strstr(class, r->class)) &&
					(!r->instance ||
					strstr(instance, r->instance))) {
				c->floating = r->floating;
				c->tags |= r->tags;

				for (m = mons; m && m->id != r->monitor;
						m = m->next);

				if (m)
					c->mon = m;
			}
		}

		xcb_icccm_get_wm_class_reply_wipe(&ch);
	}

	/* TODO Confirm */
	if (!c->tags)
		c->tags = c->mon->tags;
	/* c->tags = c->tags & TAGMASK() ? c->tags & TAGMASK() : c->mon->tagset; */
}

void manage(xcb_window_t w)
{
	struct client *c, *t = NULL;
	xcb_window_t trans_reply = 0;
	xcb_window_t trans = 0;
	xcb_get_geometry_reply_t *geom_reply;

	c = (struct client *) calloc(1, sizeof(struct client));
	if (!c)
		die("unable to allocate client\n");

	c->win = w;
	title_update(c);
	
	xcb_icccm_get_wm_transient_for_reply(conn,
			xcb_icccm_get_wm_transient_for(conn, w),
			&trans_reply, &err);
	testerr();

	if (trans_reply)
		t = client_get(trans_reply);

	if (t) {
		c->mon = t->mon;
		c->tags = t->tags;
	} else {
		c->mon = selmon;
		rules_apply(c);
	}

	geom_reply = xcb_get_geometry_reply(conn,
			xcb_get_geometry(conn, w), &err);
	testerr();

	c->x = c->oldx = geom_reply->x + c->mon->x;
	c->y = c->oldy =
		geom_reply->y + c->mon->y + (c->mon->showbar ? 16 : 0);
	c->w = c->oldw = geom_reply->width;
	c->h = c->oldh = geom_reply->height;

	if (c->w == c->mon->w && c->h == c->mon->h) {
		c->floating = 1;

		c->x = c->mon->x;
		c->y = c->mon->y;
	} else {
		if (c->x + c->w > c->mon->x + c->mon->w)
			c->x = c->mon->x + c->mon->w - c->w;
		if (c->y + c->h > c->mon->y + c->mon->h)
			c->y = c->mon->y + c->mon->h - c->h;

		c->x = MAX(c->x, c->mon->x);
		c->y = MAX(c->y, (c->mon->showbar &&
				(c->x + (c->w / 2) >= c->mon->x) &&
				(c->x + (c->w / 2) < c->mon->x + c->mon->w)) ?
				16 : c->mon->y);
	}
	
	uint32_t cw_values[] = {
		XCB_EVENT_MASK_ENTER_WINDOW |
		XCB_EVENT_MASK_FOCUS_CHANGE |
		XCB_EVENT_MASK_PROPERTY_CHANGE |
		XCB_EVENT_MASK_STRUCTURE_NOTIFY
	};
	xcb_change_window_attributes(conn, w, XCB_CW_EVENT_MASK, cw_values);

	attach(c);
	configure(c);
	windowtype_update(c);
	updatesizehints(c);
	buttons_grab(c, 0);

	if (!c->floating)
		c->floating = c->oldstate = trans != 0 || c->fixed;

	uint32_t config_values[] = {
		c->x + 2 * screen->width_in_pixels,
		c->y,
		c->w,
		c->h,
		XCB_STACK_MODE_ABOVE
	};

	xcb_configure_window(conn, c->win, XCB_CONFIG_WINDOW_X |
			XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
			XCB_CONFIG_WINDOW_HEIGHT | (c->floating ?
			XCB_CONFIG_WINDOW_STACK_MODE : 0), config_values);
	xcb_map_window(conn, c->win);

	setclientstate(c, XCB_ICCCM_WM_STATE_NORMAL);
	arrange(c->mon);
}

void unmanage(struct client *c, int destroyed)
{
	struct monitor *m = c->mon;

	detach(c);

	if (!destroyed) {
		xcb_grab_server(conn);
		xcb_ungrab_button_checked(conn, XCB_BUTTON_INDEX_ANY, c->win,
			XCB_GRAB_ANY);
		setclientstate(c, XCB_ICCCM_WM_STATE_WITHDRAWN);
		xcb_flush(conn);
		xcb_ungrab_server(conn);
	}

	free(c);
	focus(NULL);
	arrange(m);
}

void client_kill(void)
{
	xcb_client_message_event_t ev;

	if (!selmon->client)
		return;

	if (isprotodel(selmon->client)) {
		ev.response_type = XCB_CLIENT_MESSAGE;
		ev.window = selmon->client->win;
		ev.format = 32;
		ev.data.data32[0] = atoms[ATOM_DELETE];
		ev.data.data32[1] = XCB_TIME_CURRENT_TIME;
		ev.type = atoms[ATOM_WM];

		testcookie(xcb_send_event_checked(conn, 0, selmon->client->win,
				XCB_EVENT_MASK_NO_EVENT, (const char *) &ev));
	} else {
		xcb_grab_server(conn);
		xcb_set_close_down_mode(conn, XCB_CLOSE_DOWN_DESTROY_ALL);
		xcb_kill_client(conn, selmon->client->win);
		xcb_ungrab_server(conn);
		xcb_flush(conn);
	}
}

void client_move_mouse(const Arg *arg, int move)
{
	xcb_motion_notify_event_t* e;
	struct monitor *m;
	struct client *c;
	int x, y, ox, oy, ow, oh;
	unsigned int cur;
	int active = 1;

	if (!(c = selmon->client) || !curpos_get(c->win, &x, &y))
		return;

	/* restack(selmon); */
	ox = c->x;
	oy = c->y;
	ow = c->w;
	oh = c->h;

	if (move) {
		cur = CUR_MOVE;
	} else {
		/* FIXME Not always reliable */
		x = x < (ow / 2);
		y = y < (oh / 2);

		if (x && y)
			cur = CUR_RESIZE_TL;
		else if (y)
			cur = CUR_RESIZE_TR;
		else if (x)
			cur = CUR_RESIZE_BL;
		else
			cur = CUR_RESIZE_BR;

		xcb_warp_pointer(conn, 0, c->win, 0, 0, 0, 0,
				x ? 0 : ow - 1, y ? 0 : oh - 1);
	}

	free(xcb_grab_pointer_reply(conn, xcb_grab_pointer(conn, 0, root,
			MOUSEMASK, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, 0,
			cursor[cur], XCB_TIME_CURRENT_TIME), &err));
	if (err)
		return;

	do {
		while (!(e = (void *) xcb_wait_for_event(conn)));

		switch (e->response_type & ~0x80) {
		case XCB_CONFIGURE_REQUEST:
		case XCB_MAP_REQUEST:
			xcb_handlers[e->response_type & ~0x80]
					((xcb_generic_event_t *) e);
			break;
		case XCB_KEY_PRESS:
		case XCB_KEY_RELEASE:
		case XCB_BUTTON_PRESS:
		case XCB_BUTTON_RELEASE:
			active = 0;
		case XCB_MOTION_NOTIFY:
			/* FIXME Check at the very start, why again? */
			if (!c->floating &&
					selmon->layouts[selmon->tag]->arrange)
				break;

			/* TODO Adhere to incw and inch */
			if (move)
				resize(c, e->event_x + ox - x,
						e->event_y + oy - y,
						c->w, c->h, 1);
			else
				resize(c, x ? e->event_x : c->x,
						y ? e->event_y : c->y,
						x ? ox + ow - e->event_x :
						e->event_x - ox + 1,
						y ? oy + oh - e->event_y :
						e->event_y - oy + 1, 1);
		}

		free(e);
		xcb_flush(conn);
	} while (active);

	xcb_ungrab_pointer(conn, XCB_TIME_CURRENT_TIME);
	xcb_flush(conn);

	if ((m = ptrtomon(c->x + c->w / 2, c->y + c->h / 2)) != selmon) {
		mon_send(c, m);
		selmon = m;
		focus(NULL);
	}
}

void buttons_grab(struct client *c, int focused)
{
	uint32_t val[] = {
		0,
		XCB_MOD_MASK_LOCK,
		numlockmask,
		numlockmask | XCB_MOD_MASK_LOCK
	};

	unsigned int i, j;

	numlockmask_update();

	xcb_ungrab_button(conn, XCB_BUTTON_INDEX_ANY, c->win, XCB_GRAB_ANY);

	if (focused) {
		for (i = 0; i < buttons_len; i++)
			if (buttons[i].click == CLICK_CLIENT)
				for (j = 0; j < LENGTH(val); j++)
					xcb_grab_button(conn, 0, c->win, BUTTONMASK, XCB_GRAB_MODE_SYNC,
						XCB_GRAB_MODE_ASYNC, 0, XCB_CURSOR_NONE,
						buttons[i].button, buttons[i].mask | val[j]);
	} else {
		xcb_grab_button(conn, 0, c->win, BUTTONMASK, XCB_GRAB_MODE_ASYNC,
			XCB_GRAB_MODE_SYNC, 0, XCB_CURSOR_NONE,
			XCB_BUTTON_INDEX_ANY, XCB_BUTTON_MASK_ANY);
	}
}

void keys_grab(void)
{
	unsigned int i, j;
	xcb_keycode_t *code;

	numlockmask_update();
	uint32_t val[] = {
		0,
		XCB_MOD_MASK_LOCK,
		numlockmask,
		numlockmask | XCB_MOD_MASK_LOCK
	};

	if (!syms)
		syms = xcb_key_symbols_alloc(conn);

	xcb_ungrab_key(conn, XCB_GRAB_ANY, root, XCB_MOD_MASK_ANY);
	for (i = 0; i < keys_len; i++) {
		code = xcb_key_symbols_get_keycode(syms, keys[i].keysym);
		if (code)
			for (j = 0; j < LENGTH(val); j++)
				xcb_grab_key(conn, 1, root, keys[i].mod | val[j],
					*code, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
		free(code);
	}
}

int textprop_get(xcb_window_t w, xcb_atom_t atom, char *text, unsigned int size)
{
	xcb_icccm_get_text_property_reply_t reply;
	if (!xcb_icccm_get_text_property_reply(conn,
			xcb_icccm_get_text_property(conn, w, atom),
			&reply, &err))
		return 0;

	if (err) {
		testerr();
		return 0;
	}

	/* TODO Encoding (UTF-8) */

	if (!reply.name || !reply.name_len)
		return 0;

	strncpy(text, reply.name, MIN(reply.name_len + 1, size));
	text[MIN(reply.name_len + 1, size) - 1] = '\0';
	xcb_icccm_get_text_property_reply_wipe(&reply);
	
	return 1;
}

void urgent_clear(struct client *c)
{
	xcb_icccm_wm_hints_t wmh;

	if (!c->urgent || !xcb_icccm_get_wm_hints_reply(conn,
			xcb_icccm_get_wm_hints_unchecked(conn, c->win),
			&wmh, 0))
		return;

	c->urgent = 0;

	wmh.flags &= ~XCB_ICCCM_WM_HINT_X_URGENCY;
	xcb_icccm_set_wm_hints(conn, c->win, &wmh);
}

static int buttonpress(xcb_generic_event_t *_e)
{
	xcb_button_press_event_t *e = (void *) _e;
	struct monitor *m;
	struct client *c;
	unsigned int i = 0, x = 0, click = 0;
	Arg arg = { 0 };

	if ((m = mon_get(e->event)) && m != selmon) {
		unfocus(selmon->client, 1);
		selmon = m;
		focus(0);
	}

	if (e->event == selmon->barwin) {
		do {
			x += TEXTW(selmon->barcr, tags[i].name);
		} while (e->event_x >= x && ++i < tags_len);

		if (i < tags_len) {
			click = CLICK_TAGS;
			arg.i = i;
		}
	} else if ((c = client_get(e->event))) {
		focus(c);

		if (c->floating || !selmon->layouts[selmon->tag]->arrange)
			restack(c->mon);

		click = CLICK_CLIENT;
	}

	for (i = 0; i < buttons_len; i++)
		if (click == buttons[i].click && buttons[i].func &&
				buttons[i].button == e->detail &&
				CLEANMASK(buttons[i].mask) ==
				CLEANMASK(e->state))
			buttons[i].func(click == CLICK_CLIENT &&
					buttons[i].arg.i == 0 ?
					&arg : &buttons[i].arg);

	return 0;
}

static int clientmessage(xcb_generic_event_t *_e)
{
	uint32_t values[] = { XCB_STACK_MODE_ABOVE };

	xcb_client_message_event_t *e = (void *) _e;
	struct client *c;

	if (!(c = client_get(e->window)))
		return 0;

	if (e->type == atoms[ATOM_NETSTATE] && 
			e->data.data32[1] == atoms[ATOM_FULLSCREEN]) {
		if (e->data.data32[0]) {
			xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
					e->window, atoms[ATOM_NETSTATE],
					XCB_ATOM, 32, 1, (const void *)
					&atoms[ATOM_FULLSCREEN]);
			c->oldstate = c->floating;
			c->floating = 1;
			client_resize(c, c->mon->x, c->mon->y,
					c->mon->w, c->mon->h);
			xcb_configure_window(conn, c->win,
					XCB_CONFIG_WINDOW_STACK_MODE, values);
		} else {
			xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
					e->window, atoms[ATOM_NETSTATE],
					XCB_ATOM, 32, 0, (const void *) 0);
			c->floating = c->oldstate;
			c->x = c->oldx;
			c->y = c->oldy;
			c->w = c->oldw;
			c->h = c->oldh;
			client_resize(c, c->x, c->y, c->w, c->h);
			arrange(c->mon);
		}
	} else if (e->type == atoms[ATOM_ACTIVE]) {
		if (!ISVISIBLE(c)) /* XXX Set urgent or something maybe? */
			return 0;

		focus(c);
	}

	return 0;
}

static int configurerequest(xcb_generic_event_t *_e)
{
	xcb_configure_request_event_t *e = (xcb_configure_request_event_t *) _e;
	xcb_params_configure_window_t params;
	uint32_t mask = 0;
	struct monitor *m;
	struct client *c;

	if ((c = client_get(e->window))) {
		if (c->floating || !selmon->layouts[selmon->tag]->arrange) {
			m = c->mon;
			if (e->value_mask & XCB_CONFIG_WINDOW_X)
			       c->x = m->x + e->x;
			if (e->value_mask & XCB_CONFIG_WINDOW_Y)
			       c->y = m->y + e->y;
			if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH)
			       c->w = e->width;
			if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT)
				c->h = e->height;
			if ((c->x + c->w) > m->x + m->w && c->floating)
				c->x = m->x + (m->w / 2 - c->w / 2);
			if ((c->y + c->h) > m->y + m->h && c->floating)
				c->y = m->y + (m->h / 2 - c->h / 2);

			if ((e->value_mask & (XCB_CONFIG_WINDOW_X |
					XCB_CONFIG_WINDOW_Y)) &&
					!(e->value_mask &
					(XCB_CONFIG_WINDOW_WIDTH |
					XCB_CONFIG_WINDOW_HEIGHT)))
				configure(c);

			if (ISVISIBLE(c)) {
				uint32_t values[] = { c->x, c->y, c->w, c->h };
				testcookie(xcb_configure_window_checked(conn,
					c->win, XCB_CONFIG_WINDOW_X |
					XCB_CONFIG_WINDOW_Y |
					XCB_CONFIG_WINDOW_WIDTH |
					XCB_CONFIG_WINDOW_HEIGHT, values));
			}
		} else {
			configure(c);
		}
	} else {
		if (e->value_mask & XCB_CONFIG_WINDOW_X)
		       XCB_AUX_ADD_PARAM(&mask, &params, x, e->x);
		if (e->value_mask & XCB_CONFIG_WINDOW_Y)
		       XCB_AUX_ADD_PARAM(&mask, &params, y, e->y);
		if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH)
		       XCB_AUX_ADD_PARAM(&mask, &params, width, e->width);
		if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT)
		       XCB_AUX_ADD_PARAM(&mask, &params, height, e->height);
		if (e->value_mask & XCB_CONFIG_WINDOW_SIBLING)
		       XCB_AUX_ADD_PARAM(&mask, &params, sibling, e->sibling);
		if (e->value_mask & XCB_CONFIG_WINDOW_STACK_MODE)
			XCB_AUX_ADD_PARAM(&mask, &params, stack_mode,
					e->stack_mode);

		xcb_aux_configure_window(conn, e->window, mask, &params);
	}

	xcb_flush(conn);

	return 0;
}

static int configurenotify(xcb_generic_event_t *_e)
{
	xcb_configure_notify_event_t *e = (void *) _e;

	if (e->window != root)
		return 0;

	if (geom_update(e->width, e->height))
		arrange(0);

	xcb_flush(conn);

	return 0;
}

static int destroynotify(xcb_generic_event_t *_e)
{
	xcb_destroy_notify_event_t *e = (void *) _e;
	struct client *c;

	if ((c = client_get(e->window)))
		unmanage(c, 1);

	return 0;
}

static int enternotify(xcb_generic_event_t *_e)
{
	xcb_enter_notify_event_t *e = (void *) _e;
	struct monitor *m;
	struct client *c;

	c = client_get(e->event);

	if ((c && (c->floating || !c->mon->layouts[c->mon->tag]->arrange)) ||
			((e->mode != XCB_NOTIFY_MODE_NORMAL ||
			e->detail == XCB_NOTIFY_DETAIL_INFERIOR) &&
			e->event != root))
		return 0;

	if ((m = mon_get(e->event)) && m != selmon) {
		unfocus(selmon->client, 1);
		selmon = m;
	}

	focus(c);

	return 0;
}

static int expose(xcb_generic_event_t *_e)
{
	struct monitor *m;
	xcb_expose_event_t *e = (void *) _e;

	if (e->count == 0 && (m = mon_get(e->window)))
		bar_draw(m);

	return 0;
}

static int focusin(xcb_generic_event_t *_e)
{
	xcb_focus_in_event_t *e = (void *) _e;

	if (selmon->client && e->event != selmon->client->win)
		xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
				selmon->client->win, XCB_CURRENT_TIME);

	return 0;
}

static int keypress(xcb_generic_event_t *e)
{
	xcb_key_press_event_t *ev;
	xcb_keysym_t keysym;
	unsigned int i;

	ev = (xcb_key_press_event_t*) e;
	keysym = xcb_key_press_lookup_keysym(syms, ev, 0);

	for (i = 0; i < keys_len; i++)
		if (keysym == keys[i].keysym && CLEANMASK(keys[i].mod) ==
				CLEANMASK(ev->state) && keys[i].func)
			keys[i].func(&(keys[i].arg));

	return 0;
}

static int mappingnotify(xcb_generic_event_t *e)
{
	xcb_mapping_notify_event_t *ev = (xcb_mapping_notify_event_t*)e;

	xcb_refresh_keyboard_mapping(syms, ev);

	if (ev->request == XCB_MAPPING_NOTIFY)
		keys_grab();

	return 0;
}

static int maprequest(xcb_generic_event_t *e)
{
	xcb_map_request_event_t *ev = (xcb_map_request_event_t*)e;

	xcb_get_window_attributes_reply_t *ga_reply =
			xcb_get_window_attributes_reply(conn,
			xcb_get_window_attributes(conn, ev->window), &err);
	testerr();

	if (!ga_reply)
		return 0;

	if (ga_reply->override_redirect)
		return 0;

	if (!client_get(ev->window))
		manage(ev->window);

	free(ga_reply);

	return 0;
}

static int unmapnotify(xcb_generic_event_t *_e)
{
	xcb_unmap_notify_event_t *e = (xcb_unmap_notify_event_t *) _e;
	struct client *c;

	if ((c = client_get(e->window)))
		unmanage(c, 0);

	return 0;
}

static int propertynotify(xcb_generic_event_t *_e)
{
	xcb_get_property_reply_t* reply;
	xcb_property_notify_event_t *e;
	xcb_window_t trans = 0;
	struct client *c;

	e = (void *) _e;

	if (e->state == XCB_PROPERTY_DELETE || !(c = client_get(e->window)))
		return 0;

	if (e->atom == XCB_ATOM_WM_NORMAL_HINTS) {
		updatesizehints(c);
	} else if (e->atom == XCB_ATOM_WM_HINTS) {
		updatewmhints(c);
		bars_draw();
	} else if (e->atom == XCB_ATOM_WM_NAME || atoms[ATOM_NAME]) {
		title_update(c);

		if (c == c->mon->client)
			bar_draw(c->mon);
	} else if (e->atom == atoms[ATOM_TYPE]) {
		windowtype_update(c);
	} else if (e->atom == XCB_ATOM_WM_TRANSIENT_FOR) {
		reply = xcb_get_property_reply(conn,
				xcb_icccm_get_wm_transient_for(conn, c->win),
				&err);
		testerr();

		xcb_icccm_get_wm_transient_for_from_reply( &trans, reply);

		if (trans && !c->floating &&
				(c->floating = (client_get(trans) != NULL)))
			arrange(c->mon);
	}

	return 0;
}

static int (*xcb_handlers[XCB_NO_OPERATION]) (xcb_generic_event_t *) = {
	[XCB_BUTTON_PRESS] =		&buttonpress,
	[XCB_CLIENT_MESSAGE] =		&clientmessage,
	[XCB_CONFIGURE_REQUEST] =	&configurerequest,
	[XCB_CONFIGURE_NOTIFY] =	&configurenotify,
	[XCB_DESTROY_NOTIFY] =		&destroynotify,
	[XCB_ENTER_NOTIFY] =		&enternotify,
	[XCB_EXPOSE]			&expose,
	[XCB_FOCUS_IN]			&focusin,
	[XCB_KEY_PRESS] =		&keypress,
	[XCB_MAPPING_NOTIFY] =		&mappingnotify,
	[XCB_MAP_REQUEST] =		&maprequest,
	[XCB_PROPERTY_NOTIFY] =		&propertynotify,
	[XCB_UNMAP_NOTIFY] =		&unmapnotify,
	[XCB_NONE] =			NULL
};

void events_init(void)
{
	uint32_t cw_values[] =  {
		XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
		XCB_EVENT_MASK_BUTTON_PRESS |
		XCB_EVENT_MASK_ENTER_WINDOW |
		XCB_EVENT_MASK_LEAVE_WINDOW |
		XCB_EVENT_MASK_STRUCTURE_NOTIFY |
		XCB_EVENT_MASK_PROPERTY_CHANGE,
		cursor[CUR_NORMAL]
	};

	testcookie(xcb_change_window_attributes_checked(conn, root,
			XCB_CW_EVENT_MASK | XCB_CW_CURSOR, cw_values));
}

int xinerama_init(void)
{
	xcb_xinerama_is_active_reply_t *xia;
	xcb_xinerama_query_screens_reply_t *xiq;
	xcb_xinerama_screen_info_t *xii;
	struct monitor *m;
	int i, l, n, res = 0;

	xia = xcb_xinerama_is_active_reply(conn,
			xcb_xinerama_is_active(conn), &err);
	testerr();

	if (!xia || !xia->state)
		return -1;

	xiq = xcb_xinerama_query_screens_reply(conn,
			xcb_xinerama_query_screens_unchecked(conn),
			&err);
	testerr();

	xii = xcb_xinerama_query_screens_screen_info(xiq);
	l = xcb_xinerama_query_screens_screen_info_length(xiq);

	for (n = 0, m = mons; m; m = m->next, n++);

	for (i = 0; i < (l - n); i++) {
		for (m = mons; m && m->next; m = m->next);

		if (m)
			m->next = mon_alloc();
		else
			mons = mon_alloc();
	}

	for (i = 0, m = mons; i < l && m; m = m->next, i++) {
		res = 1;
		m->id = i;
		m->x = xii[i].x_org;
		m->y = xii[i].y_org;
		m->w = xii[i].width;
		m->h = xii[i].height;
	}

	free(xiq);
	free(xia);

	return res;
}

void mon_init(void)
{
	geom_update(screen->width_in_pixels, screen->height_in_pixels);
}

void mon_quit(void)
{
	struct monitor *m;

	m = mons;

	while (m) {
		xcb_unmap_window(conn, m->barwin);
		xcb_destroy_window(conn, m->barwin);

		cairo_surface_destroy(m->barsur);
		cairo_destroy(m->barcr);

		while (m->clients)
			unmanage(m->clients, 0);

		m = m->next;
		free(m);
	}
}

void cur_init(void)
{
	xcb_cursor_context_t *ctx;

	xcb_cursor_context_new(conn, screen, &ctx);
	cursor[CUR_NORMAL] = xcb_cursor_load_cursor(ctx, "left_ptr");
	cursor[CUR_MOVE] = xcb_cursor_load_cursor(ctx, "fleur");
	cursor[CUR_RESIZE_TL] = xcb_cursor_load_cursor(ctx, "top_left_corner");
	cursor[CUR_RESIZE_TR] = xcb_cursor_load_cursor(ctx, "top_right_corner");
	cursor[CUR_RESIZE_BL] =
		xcb_cursor_load_cursor(ctx, "bottom_left_corner");
	cursor[CUR_RESIZE_BR] =
		xcb_cursor_load_cursor(ctx, "bottom_right_corner");
}

void run(void)
{
	xcb_generic_event_t *e;
	xcb_generic_error_t *error;
	
	while ((e = xcb_wait_for_event(conn))) {
		if (e->response_type & ~0x80) {
			if ((e->response_type & ~0x80) == XCB_NONE &&
					selmon->redrawbar) {
				selmon->redrawbar = 0;
				bar_draw(selmon);
			} else if (xcb_handlers[e->response_type & ~0x80]) {
				xcb_handlers[e->response_type & ~0x80](e);
			}
		} else {
			error = (void *) e;

			if (error->error_code != 3) {
				fprintf(stderr, "%s, previous request returned error %i (%s) [%i, %i], res. %i, type. %i\n",
					__FILE__,
					(int) error->error_code,
					xcb_event_get_error_label(error->error_code),
					(uint32_t) error->major_code,
					(uint32_t) error->minor_code,
					(uint32_t) error->resource_id,
					(uint32_t) error->response_type & ~0x80);
			}
		}

		free(e);
	}
}

void xcb_init(void)
{
	uint32_t values[] = { XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT };

	if (!(conn = xcb_connect(0, 0)))
		die("unable to open display\n");

	screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
	if (!screen)
		die("unable to open screen\n");
	root = screen->root;

	err = xcb_request_check(conn, xcb_change_window_attributes_checked(conn,
			root, XCB_CW_EVENT_MASK, values));
	if (err) {
		free(err);
		die("another window manager is already running\n");
	}
}

void xcb_quit(void)
{
	xcb_ungrab_key(conn, XCB_GRAB_ANY, root, XCB_MOD_MASK_ANY);
	xcb_free_cursor(conn, cursor[CUR_NORMAL]);
	xcb_free_cursor(conn, cursor[CUR_MOVE]);
	xcb_free_cursor(conn, cursor[CUR_RESIZE_TL]);
	xcb_free_cursor(conn, cursor[CUR_RESIZE_TR]);
	xcb_free_cursor(conn, cursor[CUR_RESIZE_BL]);
	xcb_free_cursor(conn, cursor[CUR_RESIZE_BR]);

	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
			XCB_INPUT_FOCUS_POINTER_ROOT, XCB_CURRENT_TIME);

	xcb_flush(conn);
	xcb_disconnect(conn);

	exit(0);
}
