/*
 *
 * btdwm
 * src/include/functions.h
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

#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

union arg {
	int		i;
	double		d;
	const char	**s;
};

struct monitor;

struct layout {
	const char	*name;
	void		(*arrange) (struct monitor *);
};

struct tag {
	const char	*name;
	struct layout	*layout;
};

struct rule {
	const char	*class;
	const char	*instance;
	const char	*title;

	int		floating;
	int		transparent;
};

struct hook {
	int		event;
	char		**cmd;
};

#define TAGKEYS(k,t) \
	K_SUPER,		k,	viewtag,	{ .i = t } }, \
	{ K_SUPER | K_CTRL,	k,	toggletag,	{ .i = t } }, \
	{ K_SUPER | K_SHIFT,	k,	sendtag,	{ .i = t } \

#define SHCMD(cmd) "/usr/bin/env", "sh", "-c", cmd

struct key {
	unsigned int	mod;
	xcb_keysym_t	keysym;
	void		(*func) (const union arg *);
	const union arg	arg;
};

struct button {
	unsigned int	mask;
	unsigned int	button;
	unsigned int	click;
	void		(*func) (const union arg *);
	const union arg	arg;
};

void focusclient(const union arg *arg);
void focusmon(const union arg *arg);
void killclient(const union arg *arg);
/* void moveclient(const union arg *arg); */
void moveclientm(const union arg *arg);
void quit(const union arg *arg);
/* void resizeclient(const union arg *arg); */
void resizeclientm(const union arg *arg);
void sendmon(const union arg *arg);
void sendtag(const union arg *arg);
void setlayout(const union arg *arg);
void setmfact(const union arg *arg);
void spawn(const union arg *arg);
void togglebar(const union arg *arg);
void togglefloating(const union arg *arg);
void togglefullscr(const union arg *arg);
void toggleontop(const union arg *arg);
void togglesticky(const union arg *arg);
void toggletag(const union arg *arg);
void viewtag(const union arg *arg);

#endif
