#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "btdwm.h"
#include "libnotify.h"

static NotifyNotification *msg_layout;

void movemouse(const Arg *arg)
{
	client_move_mouse(arg, 1);
}

void resizemouse(const Arg *arg)
{
	client_move_mouse(arg, 0);
}

void focusmon(const Arg *arg)
{
	struct monitor *m = NULL;
	int x, y;

	if (!mons->next)
		return;

	if ((m = dirtomon(arg->i)) == selmon)
		return;

	curpos_get(0, &x, &y);

	/* FIXME Not working! */
	xcb_warp_pointer(conn, 0, root, 0, 0, 0, 0,
			m->x + (x / selmon->w * m->w),
			m->y + (y / selmon->h * m->h));

	unfocus(selmon->client, 1);
	selmon = m;
	focus(NULL);
}

void focusstack(const Arg *arg)
{
	struct client *c = NULL, *i;

	if (!selmon->client)
		return;

	if (arg->i < 0) {
		for (i = selmon->clients; i != selmon->client; i = i->next)
			if (ISVISIBLE(i))
				c = i;
	} else {
		for (c = selmon->client->next; c && !ISVISIBLE(c); c = c->next);
	}

	if (!c) {
		if (arg->i < 0)
			for (c = selmon->client->next; c && !ISVISIBLE(c);
					c = c->next);
		else
			for (i = selmon->clients; i != selmon->client;
					i = i->next)
				if (ISVISIBLE(i))
					c = i;
	}

	if (c) {
		/* TODO Message */

		focus(c);
		restack(selmon);
	}
}

void killclient(const Arg *arg)
{
	client_kill();
}

void spawn(const Arg *arg)
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

void tagmon(const Arg *arg)
{
	if (!arg || !selmon->client || !mons->next)
		return;

	mon_send(selmon->client, dirtomon(arg->i));
}

void togglebar(const Arg *arg)
{
	selmon->showbar ^= 1;

	uint32_t values[] = {
		selmon->x,
		(selmon->showbar) ? 0 : -16,
		selmon->w, 16
	};

	xcb_configure_window(conn, selmon->barwin, XCB_CONFIG_WINDOW_X |
			XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH |
			XCB_CONFIG_WINDOW_HEIGHT, values);

	arrange(selmon);
}

void togglefloating(const Arg *arg)
{
	if (!selmon->client)
		return;

	selmon->client->isfloating =
		!selmon->client->isfloating || selmon->client->isfixed;

	if (selmon->client->isfloating)
		resize(selmon->client, selmon->client->x, selmon->client->y,
				selmon->client->w, selmon->client->h, 0);

	arrange(selmon);
}

void viewtag(const Arg *arg)
{
	if (selmon->tags == (1 << arg->i))
		return;

	selmon->tags = 1 << arg->i;
	selmon->tag = arg->i;

	arrange(selmon);
}

void toggletag(const Arg *arg)
{
	if (!(selmon->tags ^ (1 << arg->i)))
		return;

	selmon->tags ^= 1 << arg->i;

	arrange(selmon);
}

void moveclient(const Arg *arg)
{
	if (!selmon->client)
		return;

	selmon->client->tags = 1 << arg->i;
	arrange(selmon);
}

void setlayout(const Arg *arg)
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
	msg_layout = msg_update(msg_layout,
			selmon->layouts[selmon->tag]->symbol,
			selmon->layouts[selmon->tag]->name, 1);

	if (selmon->client)
		arrange(selmon);
	else
		bar_draw(selmon);
}

void setmfact(const Arg *arg)
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
void zoom(const Arg *arg)
{
	struct client *c = selmon->client;

	if (!selmon->lt[selmon->clientlt]->arrange ||
			selmon->lt[selmon->clientlt]->arrange == monocle ||
			(selmon->client && selmon->client->isfloating))
		return;
	if (c == nexttiled(selmon->clients))
		if (!c || !(c = nexttiled(c->next)))
			return;

	detach(c);
	attach(c);
	focus(c);
	arrange(c->mon);
}
#endif
