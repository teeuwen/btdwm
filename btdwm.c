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
 * © 2016-2017 Bastiaan Teeuwen <bastiaan@mkcl.nl>
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
#include <xcb/xcb_icccm.h>

#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "btdwm.h"
#include "msg.h"

struct monitor *mons;
struct monitor *selmon;

static int screen_w, screen_h;

/* TODO Show __FILE__ as well */
void die(const char *errstr, ...)
{
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);

	xcb_disconnect(conn);
	exit(1);
}

void attach(struct client *c)
{
	struct client *cn;

	if (c->mon->client) {
		c->prev = c->mon->client;
		c->next = c->mon->client->next;

		if (c->mon->client->next)
			c->mon->client->next->prev = c;
		c->mon->client->next = c;
	} else {
		for (cn = c->mon->clients; cn && cn->next; cn = cn->next);

		if (cn)
			cn->next = c;
		else
			c->mon->clients = c;

		c->prev = cn;
		c->next = NULL;
	}

	if (c->mon->stack)
		c->mon->stack->sprev = c;

	c->sprev = NULL;
	c->snext = c->mon->stack;
	c->mon->stack = c;
}

void detach(struct client *c)
{
	struct client *i;

	if (c->prev)
		c->prev->next = c->next;
	if (c->next)
		c->next->prev = c->prev;

	if (c == c->mon->clients)
		c->mon->clients = c->next;

	if (c == c->mon->client) {
		for (i = c->mon->clients; i && !ISVISIBLE(i); i = i->next);
		c->mon->client = i;
	}

	if (c->sprev)
		c->sprev->snext = c->snext;
	if (c->snext)
		c->snext->sprev = c->sprev;

	if (c == c->mon->stack)
		c->mon->stack = c->snext;
}

static void reattach(struct client *c)
{
	if (c == c->mon->stack)
		return;

	if (c->sprev)
		c->sprev->snext = c->snext;
	if (c->snext)
		c->snext->sprev = c->sprev;

	if (c->mon->stack)
		c->mon->stack->sprev = c;

	c->sprev = NULL;
	c->snext = c->mon->stack;
	c->mon->stack = c;
}

void client_resize(struct client *c, int x, int y, int w, int h)
{
	uint32_t val[] = { x, y, w, h };

	xcb_configure_window(conn, c->win,
			XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
			XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
			val);

	c->oldx = c->x;
	c->x = x;
	c->oldy = c->y;
	c->y = y;
	c->oldw = c->w;
	c->w = w;
	c->oldh = c->h;
	c->h = h;
	configure(c);

	xcb_flush(conn);
}

void resize(struct client *c, int x, int y, int w, int h, int interact)
{
	struct monitor *m = c->mon;
	int baseismin;

	/* Set minimum possible */
	w = MAX(1, w);
	h = MAX(1, h);

	if (interact) {
		if (x > screen_w)
			x = screen_w - c->w;
		if (y > screen_h)
			y = screen_h - c->h;
		if (x + w < 0)
			x = 0;
		if (y + h < 0)
			y = 0;
	} else {
		if (x > m->x + m->w)
			x = m->x + m->w - c->w;
		if (y > m->y + m->h)
			y = m->y + m->h - c->h;
		if (x + w < m->x)
			x = m->x;
		if (y + h < m->y)
			y = m->y;
	}

	if (h < BAR_HEIGHT)
		h = BAR_HEIGHT;
	if (w < BAR_HEIGHT)
		w = BAR_HEIGHT;

	if (ISFLOATING(c) || !c->mon->layouts[c->mon->tag]->arrange) {
		baseismin = c->basew == c->minw && c->baseh == c->minh;

		/* Temporarily remove base dimensions */
		if (!baseismin) {
			w -= c->basew;
			h -= c->baseh;
		}

		/* Adjust for aspect limits */
		if (c->mina > 0 && c->maxa > 0) {
			if (c->maxa < (double) w / h)
				w = h * c->maxa + 0.5;
			else if (c->mina < (double) h / w)
				h = w * c->mina + 0.5;
		}

		/* Increment calculation requires this */
		if (baseismin) {
			w -= c->basew;
			h -= c->baseh;
		}

		/* Adjust for increment value */
		if (c->incw)
			w -= w % c->incw;
		if (c->inch)
			h -= h % c->inch;

		/* Restore base dimensions */
		w += c->basew;
		h += c->baseh;
		w = MAX(w, c->minw);
		h = MAX(h, c->minh);

		if (c->maxw)
			w = MIN(w, c->maxw);
		if (c->maxh)
			h = MIN(h, c->maxh);
	}

	if (x != c->x || y != c->y || w != c->w || h != c->h)
		client_resize(c, x, y, w, h);
}

/* TODO Fill entire struct */
struct monitor *mon_alloc(void)
{
	struct monitor *m;
	unsigned int i;

	if (!(m = calloc(1, sizeof(struct monitor))))
		die("unable to allocate monitor\n");

	m->tags = 1;
	for (i = 0; i < LENGTH(m->layouts); i++) {
		m->layouts[i] = tags[i].layout;
		m->mfact[i] = 0.5;
	}

	m->flags |= MF_SHOWBAR;

	return m;
}

struct monitor *mon_get(xcb_window_t w)
{
	int x, y;
	struct client *c;
	struct monitor *m;

	curpos_get(0, &x, &y);

	if (w == screen->root)
		return ptrtomon(x, y);

	for (m = mons; m; m = m->next)
		if (w == m->barwin)
			return m;

	if ((c = client_get(w)))
		return c->mon;

	return selmon;
}

void mon_send(struct client *c, struct monitor *m)
{
	if (c->mon == m)
		return;

	unfocus(c, 1);
	detach(c);

	c->mon = m;
	c->tags = m->tags;

	attach(c);
	focus(NULL);
	arrange(NULL);
}

static void mon_arrange(struct monitor *m)
{
	if (m->layouts[m->tag]->arrange)
		m->layouts[m->tag]->arrange(m);

	restack(m);
}

void arrange(struct monitor *m)
{
	if (m)
		showhide(m->clients);
	else
		for (m = mons; m; m = m->next)
			showhide(m->clients);

	if (m)
		mon_arrange(m);
	else
		for (m = mons; m; m = m->next)
			mon_arrange(m);
}

void focus(struct client *c)
{
	if (!c || !ISVISIBLE(c))
		for (c = selmon->stack;
				c && !ISVISIBLE(c); c = c->snext);

	if (selmon->client)
		unfocus(selmon->client, 0);

	if (c) {
		if (c->mon != selmon)
			selmon = c->mon;

		reattach(c);
		urgent_clear(c);
		buttons_grab(c, 1);

		xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
				c->win, XCB_CURRENT_TIME);
		xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
				atoms[ATOM_ACTIVE], XCB_ATOM_WINDOW,
				32, 1, (const char *) &c->win);

	} else {
		xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
				screen->root, XCB_CURRENT_TIME);
		xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
				atoms[ATOM_ACTIVE], XCB_ATOM_WINDOW,
				32, 0, (const char *) 0);
	}

	selmon->client = c;

	xcb_flush(conn);
}

void bar_draw(struct monitor *m)
{
	struct client *c;
	int i, x = 0, w, ca = 0, cu = 0;

	for (c = m->clients; c; c = c->next) {
		ca |= c->tags;

		if (ISURGENT(c))
			cu |= c->tags;
	}

	xcb_copy_area(conn, m->bgpix, m->barwin, m->gc, 0, 0, 0, 0,
			m->w, BAR_HEIGHT);

	rectangle_draw(m, m->barcr, x, 0, m->w, BAR_HEIGHT, PLT_NORMAL);

	for (i = 0; i < tags_len; i++)
		x += textw(m->barcr, tags[i].name) + 8;

	x = m->w / 2 - x / 2;

	for (i = 0; i < tags_len; i++) {
		w = textw(m->barcr, tags[i].name) + 8;

		text_draw(m, m->barcr, x, 0, w, BAR_HEIGHT, tags[i].name,
				(m->tags & 1 << i) ? PLT_NORMAL : PLT_LIGHT);

		if ((m == selmon && m->client && m->client->tags & 1 << i) ||
				ca & 1 << i) {
			if (cu & 1 << i)
				status_draw(m, x, 0, w, PLT_URGENT);
			else if (m == selmon && m->client &&
					m->client->tags & 1 << i)
				status_draw(m, x, 0, w, PLT_NORMAL);
			else if (ca & 1 << i)
				status_draw(m, x, 0, w, PLT_ACTIVE);
		}

		x += w;
	}

	xcb_flush(conn);
}

void bars_draw(void)
{
	struct monitor *m;

	for (m = mons; m; m = m->next)
		bar_draw(m);
}

struct client *client_get(xcb_window_t w)
{
	struct monitor *m;
	struct client *c;

	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			if (c->win == w)
				return c;

	return NULL;
}

int geom_update(int w, int h) {
	int res;

	screen_w = w;
	screen_h = h;

	if ((res = xinerama_init()) < 0) {
		if (!mons)
			mons = mon_alloc();

		if (mons->w != w || mons->h != h) {
			res = 1;
			mons->w = w;
			mons->h = h;
		}
	}

	if (res) {
		selmon = mons;
		selmon = mon_get(screen->root);
	}

	return res;
}

/* ************************************************************************** */

struct monitor *dirtomon(int dir)
{
	struct monitor *m = NULL;

	if (dir > 0) {
		if (!(m = selmon->next))
			m = mons;
	} else {
		if (selmon == mons)
			for (m = mons; m->next; m = m->next);
		else
			for (m = mons; m->next != selmon; m = m->next);
	}

	return m;
}

struct client *nexttiled(struct client *c)
{
	while (c && (!ISVISIBLE(c) || ISFLOATING(c) || ISFULLSCREEN(c)))
		c = c->next;

	return c;
}

struct monitor *ptrtomon(int x, int y)
{
	struct monitor *m;

	for (m = mons; m; m = m->next)
		if (INRECT(x, y, m->x, m->y + (SHOWBAR(m) ? BAR_HEIGHT : 0),
				m->w, m->h - (SHOWBAR(m) ? BAR_HEIGHT : 0)))
			return m;
	return selmon;
}

int curpos_get(xcb_window_t w, int *x, int *y)
{
	xcb_query_pointer_reply_t *reply;

	if (!w)
		w = screen->root;

	reply = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, w), 0);

	if (w == screen->root) {
		*x = reply->root_x;
		*y = reply->root_y;
	} else {
		*x = reply->win_x;
		*y = reply->win_y;
	}

	free(reply);

	return 1;
}

int isprotodel(struct client *c)
{
	xcb_icccm_get_wm_protocols_reply_t proto_reply;
	unsigned int i;
	int ret = 0;

	if (xcb_icccm_get_wm_protocols_reply(conn,
			xcb_icccm_get_wm_protocols_unchecked(conn,
			c->win, atoms[ATOM_WM]), &proto_reply, 0)) {
		for (i = 0; i < proto_reply.atoms_len; i++)
			if (proto_reply.atoms[i] == atoms[ATOM_DELETE])
				ret = 1;

		xcb_icccm_get_wm_protocols_reply_wipe(&proto_reply);
	}

	return ret;
}

void restack(struct monitor *m)
{
	struct client *c;

	uint32_t normal[] = { 0, XCB_STACK_MODE_BELOW };
	uint32_t ontop[] = { XCB_STACK_MODE_ABOVE };

	bar_draw(m);

	if (!m->client)
		return;

	normal[0] = m->barwin;

	for (c = m->stack; c; c = c->snext) {
		if (!ISVISIBLE(c))
			continue;

		if (ISONTOP(c))
			xcb_configure_window(conn, c->win,
					XCB_CONFIG_WINDOW_STACK_MODE, ontop);
		else
			xcb_configure_window(conn, c->win,
					XCB_CONFIG_WINDOW_SIBLING |
					XCB_CONFIG_WINDOW_STACK_MODE, normal);

		normal[0] = c->win;
	}

	if (ISFULLSCREEN(m->client))
		xcb_configure_window(conn, m->client->win,
				XCB_CONFIG_WINDOW_STACK_MODE, ontop);
	else
		xcb_configure_window(conn, m->barwin,
				XCB_CONFIG_WINDOW_STACK_MODE, ontop);
}

void setclientstate(struct client *c, long state)
{
	long data[] = { state, XCB_ATOM_NONE };

	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, c->win,
			atoms[ATOM_STATE], atoms[ATOM_STATE], 32, 2,
			(const char *) data);
}

xcb_atom_t atom_add(const char *name)
{
	return xcb_intern_atom_reply(conn, xcb_intern_atom(conn,
			0, strlen(name), name), 0)->atom;
}

void atom_init(void)
{
	struct monitor *m;

	atoms[ATOM_WM] = atom_add("WM_PROTOCOLS");
	atoms[ATOM_DELETE] = atom_add("WM_DELETE_WINDOW");
	atoms[ATOM_STATE] = atom_add("WM_STATE");
	atoms[ATOM_NET] = atom_add("_NET_SUPPORTED");
	atoms[ATOM_WMCHECK] = atom_add("_NET_SUPPORTING_WM_CHECK");
	atoms[ATOM_NAME] = atom_add("_NET_WM_NAME");
	atoms[ATOM_NETSTATE] = atom_add("_NET_WM_STATE");
	atoms[ATOM_NETSTATE_FULLSCR] = atom_add("_NET_WM_STATE_FULLSCREEN");
	atoms[ATOM_NETSTATE_ONTOP] = atom_add("_NET_WM_STATE_ABOVE");
	atoms[ATOM_NETSTATE_MODAL] = atom_add("_NET_WM_STATE_MODAL");
	atoms[ATOM_ACTIVE] = atom_add("_NET_ACTIVE_WINDOW");
	atoms[ATOM_TYPE] = atom_add("_NET_WM_WINDOW_TYPE");
	atoms[ATOM_TYPE_DIALOG] = atom_add("_NET_WM_WINDOW_TYPE_DIALOG");
	atoms[ATOM_TYPE_DOCK] = atom_add("_NET_WM_WINDOW_TYPE_DOCK");
	atoms[ATOM_TYPE_NOTIFICATION] =
		atom_add("_NET_WM_WINDOW_TYPE_NOTIFICATION");
	atoms[ATOM_TYPE_SPLASH] = atom_add("_NET_WM_WINDOW_TYPE_SPLASH");
	atoms[ATOM_OPACITY] = atom_add("_NET_WM_WINDOW_OPACITY");

	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
			atoms[ATOM_NET], XCB_ATOM_ATOM, 32, ATOM_MAX - ATOM_NET,
			(const char *) &atoms[ATOM_NET]);
	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
			atoms[ATOM_NAME], atom_add("UTF8_STRING"),
			8, strlen("btdwm"), (const char *) "btdwm");
	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, screen->root,
			atoms[ATOM_WMCHECK], XCB_ATOM_WINDOW,
			32, 1, (const char *) &mons->barwin);

	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, mons->barwin,
			atoms[ATOM_WMCHECK], XCB_ATOM_WINDOW,
			32, 1, (const char *) &mons->barwin);
	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, mons->barwin,
			atoms[ATOM_NAME], atom_add("UTF8_STRING"),
			8, strlen("btdwm"), (const char *) "btdwm");

	for (m = mons; m; m = m->next)
		xcb_change_property(conn, XCB_PROP_MODE_REPLACE,
				m->barwin, atoms[ATOM_TYPE],
				XCB_ATOM_ATOM, 32, 1,
				(const char *) &atoms[ATOM_TYPE_DOCK]);

}

int atom_check(xcb_window_t w, xcb_atom_t prop, xcb_atom_t target)
{
	xcb_get_property_reply_t *reply;
	xcb_atom_t *atom;
	unsigned int i;
	int res = 0;

	reply = xcb_get_property_reply(conn, xcb_get_property(conn, 0, w, prop,
			XCB_ATOM_ATOM, 0, UINT32_MAX), 0);

	if (reply && (atom = (xcb_atom_t *) xcb_get_property_value(reply))) {
		for (i = 0; i < xcb_get_property_value_length(reply) /
				sizeof(xcb_atom_t); i++) {
			if (atom[i] == target) {
				res = 1;
				break;
			}
		}
	}

	free(reply);

	return res;
}

void showhide(struct client *c)
{
	if (!c)
		return;

	if (ISVISIBLE(c)) {
		uint32_t values[] = { c->x, c->y };

		xcb_configure_window(conn, c->win, XCB_CONFIG_WINDOW_X |
				XCB_CONFIG_WINDOW_Y, values);

		if (!c->mon->layouts[c->mon->tag]->arrange || ISFLOATING(c) ||
				ISFULLSCREEN(c))
			resize(c, c->x, c->y, c->w, c->h, 0);

		showhide(c->next);
	} else {
		showhide(c->next);
		uint32_t values[] = { c->x + 2 * screen_w, c->y };
		xcb_configure_window(conn, c->win, XCB_CONFIG_WINDOW_X |
				XCB_CONFIG_WINDOW_Y, values);
	}
}

void unfocus(struct client *c, int setfocus)
{
	if (!c)
		return;

	buttons_grab(c, 0);

	if (setfocus)
		xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
				c->win, XCB_CURRENT_TIME);
}

void windowtype_update(struct client *c)
{
	if (atom_check(c->win, atoms[ATOM_TYPE], atoms[ATOM_TYPE_DIALOG]) ||
			atom_check(c->win, atoms[ATOM_TYPE],
			atoms[ATOM_TYPE_SPLASH]) || atom_check(c->win,
			atoms[ATOM_NETSTATE], atoms[ATOM_NETSTATE_MODAL]))
		c->flags |= CF_FLOATING;

	if (atom_check(c->win, atoms[ATOM_NETSTATE],
			atoms[ATOM_NETSTATE_ONTOP]))
		c->flags |= CF_ONTOP;
}

void updatesizehints(struct client *c)
{
	xcb_size_hints_t hints;

	if (!xcb_icccm_get_wm_normal_hints_reply(conn,
			xcb_icccm_get_wm_normal_hints(conn, c->win), &hints, 0))
		hints.flags = XCB_ICCCM_SIZE_HINT_P_SIZE;

	if (hints.flags & XCB_ICCCM_SIZE_HINT_BASE_SIZE) {
		c->basew = hints.base_width;
		c->baseh = hints.base_height;

		c->minw = hints.base_width;
		c->minh = hints.base_height;
	} else if (hints.flags & XCB_ICCCM_SIZE_HINT_P_MIN_SIZE) {
		c->basew = hints.min_width;
		c->baseh = hints.min_height;

		c->minw = hints.min_width;
		c->minh = hints.min_height;
	} else {
		c->basew = c->baseh = 0;

		c->minw = c->minh = 0;
	}

	if (hints.flags & XCB_ICCCM_SIZE_HINT_P_MAX_SIZE) {
		c->maxw = hints.max_width;
		c->maxh = hints.max_height;
	} else {
		c->maxw = c->maxh = 0;
	}

	if (hints.flags & XCB_ICCCM_SIZE_HINT_P_RESIZE_INC) {
		c->incw = hints.width_inc;
		c->inch = hints.height_inc;
	} else {
		c->incw = c->inch = 0;
	}

	if (hints.flags & XCB_ICCCM_SIZE_HINT_P_ASPECT) {
		c->mina = (double) hints.min_aspect_den / hints.min_aspect_num;
		c->maxa = (double) hints.max_aspect_num / hints.max_aspect_den;
	} else {
		c->mina = c->maxa = 0.0;
	}

	c->flags = (c->minw && c->maxw && c->minh && c->maxh &&
			c->minw == c->maxw && c->minh == c->maxh) ?
			c->flags | CF_FIXED : c->flags & ~CF_FIXED;
}

void updatewmhints(struct client *c)
{
	xcb_icccm_wm_hints_t wmh;

	if (!xcb_icccm_get_wm_hints_reply(conn, xcb_icccm_get_wm_hints(conn,
			c->win), &wmh, 0))
		return;

	if (c == selmon->client && (wmh.flags & XCB_ICCCM_WM_HINT_X_URGENCY)) {
		wmh.flags &= ~XCB_ICCCM_WM_HINT_X_URGENCY;
		xcb_icccm_set_wm_hints(conn, c->win, &wmh);
	} else {
		c->flags = (wmh.flags & XCB_ICCCM_WM_HINT_X_URGENCY) ?
				c->flags | CF_URGENT : c->flags & ~CF_URGENT;
	}
}

void setup(void)
{
	bar_init();

	events_init();
	keys_grab();
}

void quit(const union arg *arg)
{
	font_quit();
	mon_quit();
	xcb_quit();
}

static void sigchld(int unused)
{
	while (0 < waitpid(-1, 0, WNOHANG));
}

static void sigint(int unused)
{
	exit(0);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, sigint);
	signal(SIGCHLD, sigchld);

	xcb_init();
	mon_init();
	font_init();
	cur_init();
	msg_init();

	setup();
	atom_init();

	scan();

	run();

	return 0;
}
