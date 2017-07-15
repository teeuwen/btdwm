/*
 *
 * btdwm
 * src/functions.c
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

#include <btdwm.h>

void moveclientm(const union arg *arg)
{
	client_move_mouse(arg, 1);
}

void resizeclientm(const union arg *arg)
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
	restack(m);
}

void focusclient(const union arg *arg)
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
			event_trigger(E_CLIENT, c->name);

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

		/* if (selmon->client)
			selmon->flags |= MF_PTRLOCK; */

		setsid();
		execvp(((char **) arg->s)[0], (char **) arg->s);

		fprintf(stderr, "failed to execvp %s", ((char **) arg->s)[0]);
		exit(0);
	}
}

void sendmon(const union arg *arg)
{
	if (!arg || !selmon->client || !mons->next)
		return;

	mon_send(selmon->client, dirtomon(arg->i));

	if (arg->i < -1 || arg->i > 1)
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
	if (arg->i < 0) {
		if (arg->i == -2) {
			if (selmon->tag > 0)
				selmon->tag--;
			else
				selmon->tag = (unsigned int) LENGTH(tags) - 1;
		} else if (arg->i == -1) {
			if (selmon->tag + 1 < (unsigned int) LENGTH(tags))
				selmon->tag++;
			else
				selmon->tag = 0;
		} else {
			return;
		}

		selmon->tags = 1 << selmon->tag;
	} else {
		if (selmon->tags == (unsigned int) (1 << arg->i))
			return;

		selmon->tags = 1 << arg->i;
		selmon->tag = arg->i;
	}

	/* TODO Readd to end of stack */

	focus(NULL);
	arrange(selmon);

	if (selmon->client)
		selmon->flags |= MF_PTRLOCK;
}

void toggletag(const union arg *arg)
{
	if (!(selmon->tags ^ (1 << arg->i)))
		return;

	selmon->tags ^= 1 << arg->i;

	focus(NULL);
	arrange(selmon);
}

void sendtag(const union arg *arg)
{
	if (!selmon->client)
		return;

	selmon->client->tags = 1 << arg->i;

	focus(NULL);
	arrange(selmon);
}

void setlayout(const union arg *arg)
{
	int i;

	if (arg->i < 0) {
		for (i = 0; i < (int) LENGTH(layouts) &&
				&layouts[i] != selmon->layouts[selmon->tag];
				i++);

		if (arg->i == -2) {
			if (++i + arg->i < 0)
				i += 3;
		} else if (arg->i == -1) {
			if ((i += 2) + arg->i > (int) LENGTH(layouts) - 1)
				i -= 3;
		} else {
			return;
		}

		selmon->layouts[selmon->tag] = &layouts[i + arg->i];
	} else if (arg->i < (int) LENGTH(layouts)) {
		selmon->layouts[selmon->tag] = &layouts[arg->i];
	}

	event_trigger(E_LAYOUT, selmon->layouts[selmon->tag]->name);

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

	f = arg->d < 1.0 ? arg->d + selmon->mfacts[selmon->tag] : arg->d - 1.0;
	if (f < 0.1 || f > 0.9)
		return;

	selmon->mfacts[selmon->tag] = f;
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
