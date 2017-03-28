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

/* Redo includes */

#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xinerama.h>

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "btdwm.h"
#include "msg.h"

static char ttime[MAX_NAME];
static char tdate[MAX_NAME];

xcb_connection_t *conn;
static int screen_w, screen_h;
struct monitor *mons;
struct monitor *selmon;
xcb_window_t root;

xcb_cursor_t cursor[CUR_MAX];

xcb_atom_t wmatom[WMLast], netatom[NetLast];

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
	if (c->mon->clients)
		c->mon->clients->prev = c;

	c->next = c->mon->clients;
	c->mon->clients = c;
}

void detach(struct client *c)
{
	struct client *i;

	if (c == c->mon->clients)
		c->mon->clients = c->next;

	if (c->prev)
		c->prev->next = c->next;
	if (c->next)
		c->next->prev = c->prev;

	if (c == c->mon->client) {
		for (i = c->mon->clients; i && !ISVISIBLE(i); i = i->next);
		c->mon->client = i;
	}
}

static void reattach(struct client *c)
{
	if (c == c->mon->clients)
		return;

	if (c->prev)
		c->prev->next = c->next;
	if (c->next)
		c->next->prev = c->prev;

	c->mon->clients->prev = c;

	c->prev = NULL;
	c->next = c->mon->clients;

	c->mon->clients = c;
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
	if (applysizehints(c, &x, &y, &w, &h, interact))
		client_resize(c, x, y, w, h);
}

/* TODO Fill entire struct */
struct monitor *mon_alloc(void)
{
	struct monitor *m;
	int i;

	m = calloc(1, sizeof(struct monitor));
	if (!m)
		die("unable to allocate monitor\n");

	m->tags = 1;
	for (i = 0; i < LENGTH(m->layouts); i++)
		m->layouts[i] = tags[i].layout;
	m->showbar = 1;

	return m;
}

struct monitor *mon_get(xcb_window_t w)
{
	int x, y;
	struct client *c;
	struct monitor *m;

	curpos_get(0, &x, &y);

	if (w == root)
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

	focus(NULL);

	if (m)
		mon_arrange(m);
	else
		for (m = mons; m; m = m->next)
			mon_arrange(m);
}

void bar_draw(struct monitor *m)
{
	struct client *c;
	unsigned int i, occ = 0;
	int x, w, cx;

	for (c = m->clients; c; c = c->next)
		occ |= c->tags;

	x = 0;

	for (i = 0; i < tags_len; i++) {
		w = TEXTW(m->barcr, tags[i].name);

		text_draw(m->barcr, x, 0, w, 16, tags[i].name,
				(m->tags & 1 << i) ? PLT_FOCUS : PLT_INACTIVE);

		if ((m == selmon && selmon->client &&
				selmon->client->tags & 1 << i) || occ & 1 << i) {
			if (m == selmon && selmon->client && selmon->client->tags
					& 1 << i)
				status_draw(m->barcr, x, 0, w, PLT_FOCUS);
			else if (occ & 1 << i)
				status_draw(m->barcr, x, 0, w, PLT_ACTIVE);
			/* TODO PLT_URGENT */
		}

		x += w;
	}

	cx = x;

	if (m == selmon) {
		w = TEXTW(m->barcr, "Unavail.");
		x = m->w - w - 15;

		gradient_draw(m->barcr, x, 0, 15, 16, PLT_CENTER, PLT_INACTIVE);
		text_draw(m->barcr, x + 15, 0, w, 16, "Unavail.", PLT_INACTIVE);

		w = x - cx - 15;
	} else {
		w = m->w - x;
	}

	gradient_draw(m->barcr, cx, 0, 15, 16, PLT_INACTIVE, PLT_CENTER);
	rectangle_draw(m->barcr, cx + 15, 0, w, 16, PLT_CENTER);

	w = TEXTW(m->barcr, tdate);
	x = m->w / 2 - (w + TEXTW(m->barcr, ttime)) / 2;
	text_draw(m->barcr, x, 0, w, 16, tdate, PLT_CENLIGHT);

	x += w - textnw(m->barcr, " ", 1) - 2;
	w = TEXTW(m->barcr, ttime);
	text_draw(m->barcr, x, 0, w, 16, ttime, PLT_CENTER);

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
		selmon = mon_get(root);
	}

	return res;
}

static void time_update(int sig)
{
	/* xcb_generic_event_t e; */
	time_t t;
	struct tm *lt;

	time(&t);
	lt = localtime(&t);

	snprintf(tdate, MAX_NAME, "%04d-%02d-%02d",
			1900 + lt->tm_year, 1 + lt->tm_mon, lt->tm_mday);

	snprintf(ttime, MAX_NAME, "%02d:%02d %s",
			lt->tm_hour - (lt->tm_hour > 12 ? 12 : 0), lt->tm_min,
			(lt->tm_hour > 12 ? "PM" : "AM"));

	selmon->redrawbar = 1;

	/* FIXME Not working */
	/* e.response_type = 1;
	xcb_send_event(conn, 0, selmon->barwin, 0, (const char *) &e); */

	signal(SIGALRM, &time_update);
	alarm(tdelay);
}

/* ************************************************************************** */

int applysizehints(struct client *c, int *x, int *y, int *w, int *h, int interact)
{
	struct monitor *m = c->mon;
	int baseismin;

	/* Set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);

	if (interact) {
		if (*x > screen_w)
			*x = screen_w - c->w;
		if (*y > screen_h)
			*y = screen_h - c->h;
		if (*x + *w < 0)
			*x = 0;
		if (*y + *h < 0)
			*y = 0;
	} else {
		if (*x > m->x + m->w)
			*x = m->x + m->w - c->w;
		if (*y > m->y + m->h)
			*y = m->y + m->h - c->h;
		if (*x + *w < m->x)
			*x = m->x;
		if (*y + *h < m->y)
			*y = m->y;
	}

	if (*h < 16)
		*h = 16;
	if (*w < 16)
		*w = 16;

	if (c->isfloating || !c->mon->layouts[c->mon->tag]->arrange) {
		baseismin = c->basew == c->minw && c->baseh == c->minh;

		/* Temporarily remove base dimensions */
		if (!baseismin) {
			*w -= c->basew;
			*h -= c->baseh;
		}

		/* Adjust for aspect limits */
		if (c->mina > 0 && c->maxa > 0) {
			if (c->maxa < (double) *w / *h)
				*w = *h * c->maxa + 0.5;
			else if (c->mina < (double) *h / *w)
				*h = *w * c->mina + 0.5;
		}

		/* Increment calculation requires this */
		if (baseismin) {
			*w -= c->basew;
			*h -= c->baseh;
		}

		/* Adjust for increment value */
		if (c->incw)
			*w -= *w % c->incw;
		if (c->inch)
			*h -= *h % c->inch;

		/* Restore base dimensions */
		*w += c->basew;
		*h += c->baseh;
		*w = MAX(*w, c->minw);
		*h = MAX(*h, c->minh);

		if (c->maxw)
			*w = MIN(*w, c->maxw);
		if (c->maxh)
			*h = MIN(*h, c->maxh);
	}

	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

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
	for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
	return c;
}

struct monitor *ptrtomon(int x, int y)
{
	struct monitor *m;

	for (m = mons; m; m = m->next)
		if (INRECT(x, y, m->x, m->y + ((m->showbar) ? 16 : 0),
				m->w, m->h - ((m->showbar) ? 16 : 0)))
			return m;
	return selmon;
}

void focus(struct client *c)
{
	if (!c || !ISVISIBLE(c))
		for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);

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
	} else {
		xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT,
				root, XCB_CURRENT_TIME);
	}

	selmon->client = c;
}

int curpos_get(xcb_window_t w, int *x, int *y)
{
	if (!w)
		w = root;

	xcb_query_pointer_reply_t *reply;

	reply = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, w), 0);

	*x = reply->root_x;
	*y = reply->root_y;

	free(reply);

	return 1;
}

int isprotodel(struct client *c)
{
	xcb_icccm_get_wm_protocols_reply_t proto_reply;
	int i, ret = 0;

	if (xcb_icccm_get_wm_protocols_reply(conn,
			xcb_icccm_get_wm_protocols_unchecked(conn,
			c->win, wmatom[WMProtocols]), &proto_reply, 0)) {
		for (i = 0; i < proto_reply.atoms_len; i++)
			if (proto_reply.atoms[i] == wmatom[WMDelete])
				ret = 1;

		xcb_icccm_get_wm_protocols_reply_wipe(&proto_reply);
	}

	return ret;
}

void restack(struct monitor *m) {
	struct client *c;

	bar_draw(m);

	if (!m->client)
		return;

	if (m->client->isfloating || !m->layouts[m->tag]->arrange) {
		uint32_t values[] = { XCB_STACK_MODE_ABOVE };
		xcb_configure_window(conn, m->client->win, XCB_CONFIG_WINDOW_STACK_MODE, values);
	}

	if (m->layouts[m->tag]->arrange) {
		uint32_t values[] = { m->barwin, XCB_STACK_MODE_BELOW };
		for (c = m->clients; c; c = c->next)
			if (!c->isfloating && ISVISIBLE(c)) {
				xcb_configure_window(conn, c->win, XCB_CONFIG_WINDOW_SIBLING |
					XCB_CONFIG_WINDOW_STACK_MODE, values);
				values[0] = c->win;
			}
	}

	xcb_flush(conn);
}

void setclientstate(struct client *c, long state)
{
	long data[] = { state, XCB_ATOM_NONE };

	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, c->win,
			wmatom[WMState], wmatom[WMState], 32, 2,
			(unsigned char*) data);
}

#define NUM_ATOMS (WMLast + NetLast)

xcb_atom_t atom_add(const char *name)
{
	return xcb_intern_atom_reply(conn, xcb_intern_atom(conn,
			0, strlen(name), name), 0)->atom;
}

void showhide(struct client *c)
{
	if (!c)
		return;

	if (ISVISIBLE(c)) {
		uint32_t values[] = { c->x, c->y };

		xcb_configure_window(conn, c->win, XCB_CONFIG_WINDOW_X |
				XCB_CONFIG_WINDOW_Y, values);

		if (!c->mon->layouts[c->mon->tag]->arrange || c->isfloating)
			resize(c, c->x, c->y, c->w, c->h, 0);
		showhide(c->next);
	} else {
		showhide(c->next);
		uint32_t values[] = { c->x + 2 * screen_w, c->y };
		xcb_configure_window(conn, c->win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
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
	printf("called\n");
	xcb_atom_t wtype = atom_get(c->win, netatom[NetWMWindowType]);
	printf("%d\n", wtype);

	if (wtype == netatom[NetWMWindowType])
		c->isfloating = 1;

	/* TODO Fullscreen windows */
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

	c->isfixed = c->minw && c->maxw && c->minh && c->maxh &&
			c->minw == c->maxw && c->minh == c->maxh;
}

void updatewmhints(struct client *c)
{
	xcb_icccm_wm_hints_t wmh;

	if (!xcb_icccm_get_wm_hints_reply(conn, xcb_icccm_get_wm_hints(conn,
			c->win), &wmh, 0))
		return;

	if (c == selmon->client && wmh.flags & XCB_ICCCM_WM_HINT_X_URGENCY) {
		wmh.flags &= ~XCB_ICCCM_WM_HINT_X_URGENCY;
		xcb_icccm_set_wm_hints(conn, c->win, &wmh);
	}
}

void setup(void)
{
	xcb_cursor_context_t *ctx;

	wmatom[WMProtocols] = atom_add("WM_PROTOCOLS");
	wmatom[WMDelete] = atom_add("WM_DELETE_WINDOW");
	wmatom[WMState] = atom_add("WM_STATE");

	netatom[NetSupported] = atom_add("_NET_WM_SUPPORTED");
	netatom[NetWMName] = atom_add("_NET_WM_NAME");
	netatom[NetWMState] = atom_add("_NET_WM_STATE");
	netatom[NetWMWindowType] = atom_add("_NET_WM_WINDOW_TYPE");
	netatom[NetWMFullscreen] = atom_add("_NET_WM_STATE_FULLSCREEN");

	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, root,
			netatom[NetSupported], XCB_ATOM, 32, NetLast,
			(unsigned char *) netatom);

	xcb_cursor_context_new(conn, screen, &ctx);
	cursor[CUR_NORMAL] = xcb_cursor_load_cursor(ctx, "left_ptr");
	cursor[CUR_MOVE] = xcb_cursor_load_cursor(ctx, "fleur");
	cursor[CUR_RESIZE_TL] = xcb_cursor_load_cursor(ctx, "top_left_corner");
	cursor[CUR_RESIZE_TR] = xcb_cursor_load_cursor(ctx, "top_right_corner");
	cursor[CUR_RESIZE_BL] =
		xcb_cursor_load_cursor(ctx, "bottom_left_corner");
	cursor[CUR_RESIZE_BR] =
		xcb_cursor_load_cursor(ctx, "bottom_right_corner");

	time_update(0);
	bars_update();

	events_init();
	keys_grab();
}

void quit(const Arg *arg)
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
	msg_init();

	setup();
	scan();

	run();

	return 0;
}
