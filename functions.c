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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "btdwm.h"
#include "msg.h"

static NotifyNotification *msg;

void movemouse(const union arg *arg)
{
	client_move_mouse(arg, 1);
}

void resizemouse(const union arg *arg)
{
	client_move_mouse(arg, 0);
}

void focusmon(const union arg *arg)
{
	struct monitor *m = NULL;
	int x, y;

	if (!mons->next)
		return;

	if ((m = dirtomon(arg->i)) == selmon)
		return;

	curpos_get(0, &x, &y);

	xcb_warp_pointer(conn, 0, screen->root, 0, 0, 0, 0,
			m->x + (((double) (x - selmon->x) / selmon->w) * m->w),
			m->y + (((double) (y - selmon->y) / selmon->h) * m->h));

	unfocus(selmon->client, 1);
	selmon = m;
	focus(NULL);
}

void focusstack(const union arg *arg)
{
	struct client *c;

	if (!selmon->client)
		return;

	if (arg->i < 0)
		for (c = selmon->client->prev; c && !ISVISIBLE(c);
				c = c->prev);
	else
		for (c = selmon->client->next; c && !ISVISIBLE(c);
				c = c->next);

	if (!c) {
		if (arg->i < -1) {
			for (c = selmon->client->next; c && c->next;
					c = c->next);
			for (; c && !ISVISIBLE(c); c = c->prev);
		} else if (arg->i > 1) {
			for (c = selmon->clients; c && !ISVISIBLE(c);
					c = c->next);
		} else {
			return;
		}
	}

	if (c) {
		/* TODO Icon */
		if (arg->i < -1 || arg->i > 1)
			msg = msg_update(msg, c->name, NULL, 500);

		focus(c);
		restack(c->mon);
	}
}

void killclient(const union arg *arg)
{
	client_kill();
}

void spawn(const union arg *arg)
{
	if (!arg)
		return;

	if (fork() == 0) {
		if (conn)
			close(xcb_get_file_descriptor(conn));

		setsid();
		execvp(((char **) arg->v)[0], (char **) arg->v);

		fprintf(stderr, "failed to execvp %s", ((char **) arg->v)[0]);
		exit(0);
	}
}

void tagmon(const union arg *arg)
{
	if (!arg || !selmon->client || !mons->next)
		return;

	mon_send(selmon->client, dirtomon(arg->i));

	focusmon(arg);
}

void togglebar(const union arg *arg)
{
	selmon->flags ^= MF_SHOWBAR;

	uint32_t values[] = {
		selmon->x,
		SHOWBAR(selmon) ? 0 : -BAR_HEIGHT,
		selmon->w, BAR_HEIGHT
	};

	xcb_configure_window(conn, selmon->barwin, XCB_CONFIG_WINDOW_X |
			XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
			XCB_CONFIG_WINDOW_HEIGHT, values);

	arrange(selmon);
}

void togglefloating(const union arg *arg)
{
	if (!selmon->client)
		return;

	selmon->client->flags =
		(!ISFLOATING(selmon->client) || ISFIXED(selmon->client)) ?
				selmon->client->flags | CF_FLOATING :
				selmon->client->flags & ~CF_FLOATING;

	if (ISFLOATING(selmon->client))
		resize(selmon->client, selmon->client->x, selmon->client->y,
				selmon->client->w, selmon->client->h, 0);

	arrange(selmon);
}

void toggleontop(const union arg *arg)
{
	if (!selmon->client)
		return;

	selmon->client->flags ^= CF_ONTOP;

	arrange(selmon);
}

void togglesticky(const union arg *arg)
{
	if (!selmon->client)
		return;

	selmon->client->flags ^= CF_STICKY;

	arrange(selmon);
}

void viewtag(const union arg *arg)
{
	if (selmon->tags == (unsigned int) (1 << arg->i))
		return;

	selmon->tags = 1 << arg->i;
	selmon->tag = arg->i;

	arrange(selmon);

	if (selmon->client)
		selmon->flags |= MF_NEWFOCUS;
}

void toggletag(const union arg *arg)
{
	if (!(selmon->tags ^ (1 << arg->i)))
		return;

	selmon->tags ^= 1 << arg->i;

	arrange(selmon);
}

void moveclient(const union arg *arg)
{
	if (!selmon->client)
		return;

	selmon->client->tags = 1 << arg->i;
	arrange(selmon);
}

void setlayout(const union arg *arg)
{
	int i;

	/* Feel like a cheap hack... */
	for (i = 0; i < layouts_len; i++)
		if (&layouts[i] == selmon->layouts[selmon->tag])
			break;

	if (i + arg->i < 0)
		i += 3;
	if (i + arg->i > layouts_len - 1)
		i -= 3;

	selmon->layouts[selmon->tag] = &layouts[i + arg->i];
	msg = msg_update(msg,
			selmon->layouts[selmon->tag]->symbol,
			selmon->layouts[selmon->tag]->name, 500);

	if (selmon->client)
		arrange(selmon);
	else
		bar_draw(selmon);
}

void setmfact(const union arg *arg)
{
	double f;

	if (!arg || !selmon->layouts[selmon->tag]->arrange)
		return;

	f = arg->f < 1.0 ? arg->f +
			selmon->layouts[selmon->tag]->mfact : arg->f - 1.0;
	if (f < 0.1 || f > 0.9)
		return;

	selmon->layouts[selmon->tag]->mfact = f;
	arrange(selmon);
}

#if 0
void zoom(const union arg *arg)
{
	struct client *c = selmon->client;

	if (!selmon->lt[selmon->clientlt]->arrange ||
			selmon->lt[selmon->clientlt]->arrange == monocle ||
			(selmon->client && selmon->client->floating))
		return;
	if (c == nexttiled(selmon->clients))
		if (!c || !(c = nexttiled(c->next)))
			return;

	reattach(c);
	focus(c);
	arrange(c->mon);
}
#endif
