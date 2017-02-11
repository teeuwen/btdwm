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

#ifndef _DWM_H
#define _DWH_H

#include <cairo/cairo-xcb.h>

/* TODO Move to config.h */
#define BAR_HEIGHT	16

/* FIXME */
#define INRECT(x,y,rx,ry,rw,rh)	((x) >= (rx) && (x) < (rx) + (rw) && (y) >= (ry) && (y) < (ry) + (rh))
#define ISVISIBLE(c)		(((c->tags & c->mon->tags) | c->sticky))
#define LENGTH(x)		(sizeof(x) / sizeof(x[0]))
#ifndef MAX
#   define MAX(a, b)		((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#   define MIN(a, b)		((a) < (b) ? (a) : (b))
#endif

#define ATOM_WM			0
#define ATOM_DELETE		1
#define ATOM_STATE		2
#define ATOM_NET		3
#define ATOM_NAME		4
#define ATOM_NETSTATE		5
#define ATOM_ACTIVE		6
#define ATOM_TYPE		7
#define ATOM_TYPE_DIALOG	8
#define ATOM_TYPE_SPLASH	9
#define ATOM_FULLSCREEN		10
#define ATOM_MODAL		11
#define ATOM_MAX		12

#define CLICK_TAGS	1
#define CLICK_CLIENT	2

#define CUR_NORMAL	0
#define CUR_MOVE	1
#define CUR_RESIZE_TL	2
#define CUR_RESIZE_TR	3
#define CUR_RESIZE_BL	4
#define CUR_RESIZE_BR	5
#define CUR_MAX		6

#define PLT_INACTIVE	0
#define PLT_ACTIVE	1
#define PLT_FOCUS	2
#define PLT_URGENT	3
#define PLT_CENLIGHT	4
#define PLT_CENTER	5

#define COLOR_BGNORMAL	0
#define COLOR_BGFOCUS	1
#define COLOR_BGCENTER	2
#define COLOR_FGNORMAL	3
#define COLOR_FGLIGHT	4
#define COLOR_STACTIVE	5
#define COLOR_STFOCUS	6
#define COLOR_STURGENT	7
#define COLOR_MAX	8

/* To avoid conflicts with limits.h */
#define MAX_NAME	256

typedef union {
	int		i, b;
	double		f;
	const void	*v;
} Arg;

struct monitor {
	int		id;
	int		x, y, w, h;

	xcb_pixmap_t	bgpix;
	xcb_gcontext_t	gc;

	xcb_window_t	barwin;
	cairo_surface_t	*barsur;
	cairo_t		*barcr;
	int		showbar, redrawbar;

	unsigned int	tags;
	unsigned int	tag;
	struct layout	*layouts[7];
	/* unsigned int	tags[LENGTH(tags)]; FIXME FIXME FIXME FIXME FIXME */

	struct client	*clients;
	struct client	*stack;
	struct client	*client;

	struct monitor	*next;
};

struct client {
	struct monitor	*mon;
	xcb_window_t	win;

	char		name[MAX_NAME];
	unsigned int	tags;

	int		x, y, w, h;
	int		oldx, oldy, oldw, oldh;
	int		fullscreen, fixed, floating, sticky, urgent;

	double		mina, maxa;
	int		basew, baseh, incw, inch, maxw, maxh, minw, minh;

	struct client	*prev, *sprev;
	struct client	*next, *snext;
};

struct layout {
	const char	*symbol;
	const char	*name;
	void		(*arrange) (struct monitor *);

	double		mfact;
};

struct tag {
	const char	*name;
	struct layout	*layout;
};

struct rule {
	const char	*class;
	const char	*instance;
	const char	*title;

	unsigned int	tags;
	int		floating;
	int		monitor;
};

#define TAGKEYS(k,t) \
	K_SUPER,		k,	viewtag,	{ .i = t } }, \
	{ K_SUPER | K_CTRL,	k,	toggletag,	{ .i = t } }, \
	{ K_SUPER | K_SHIFT,	k,	moveclient,	{ .i = t } \

#define SHCMD(cmd) "/bin/sh", "-c", cmd

struct key {
	unsigned int	mod;
	xcb_keysym_t	keysym;
	void		(*func) (const Arg *);
	const Arg	arg;
};

struct button {
	unsigned int	click;
	unsigned int	mask;
	unsigned int	button;
	void		(*func) (const Arg *arg);
	const Arg	arg;
};

extern const char font_desc[];
extern const char bg_normal[];
extern const char bg_focus[];
extern const char bg_center[];
extern const char fg_normal[];
extern const char fg_light[];
extern const char status_active[];
extern const char status_focus[];
extern const char status_urgent[];
extern struct layout layouts[];
extern const int layouts_len;
extern const struct tag tags[];
extern const int tags_len;
extern const struct rule rules[];
extern const int rules_len;
extern const struct key keys[];
extern const int keys_len;
extern const struct button buttons[];
extern const int buttons_len;
extern const unsigned int tdelay;

extern xcb_connection_t *conn;
extern xcb_screen_t *screen;

extern xcb_atom_t atoms[];

extern struct monitor *mons;
extern struct monitor *selmon;

void attach(struct client *c);
void detach(struct client *c);

void arrange(struct monitor *m);
void focus(struct client *c);
void restack(struct monitor *m);

void client_resize(struct client *c, int x, int y, int w, int h);
void resize(struct client *c, int x, int y, int w, int h, int interact);

struct monitor *mon_alloc(void);
struct monitor *mon_get(xcb_window_t w);
void mon_send(struct client *c, struct monitor *m);

void configure(struct client *c);
void scan(void);
void manage(xcb_window_t w);
void unmanage(struct client *c, int destroyed);

struct client *client_get(xcb_window_t w);
void client_kill(void);
void client_move_mouse(const Arg *arg, int move);


void bar_draw(struct monitor *m);
void bars_draw(void);

void gradient_draw(cairo_t *cr, int x, int y, int w, int h,
		int palette1, int palette2);
void rectangle_draw(cairo_t *cr, int x, int y, int w, int h, int palette);
void status_draw(cairo_t *cr, int x, int y, int w, int palette);
void text_draw(cairo_t *cr, int x, int y, int w, int h,
		const char *text, int palette);

int geom_update(int w, int h);
void numlockmask_update(void);

void buttons_grab(struct client *c, int focused);
void keys_grab(void);

int curpos_get(xcb_window_t w, int *x, int *y);
int textprop_get(xcb_window_t w, xcb_atom_t atom,
		char *text, unsigned int size);

xcb_atom_t atom_add(const char *name);
int atom_check(xcb_window_t w, xcb_atom_t prop, xcb_atom_t target);
void atom_init(void);

void urgent_clear(struct client *c);

void bar_init(void);
void cur_init(void);
void events_init(void);
void font_init(void);
void mon_init(void);
void xcb_init(void);
int xinerama_init(void);

void font_quit(void);
void mon_quit(void);
void xcb_quit(void);

void setup(void);
void run(void);

void quit(const Arg *arg);
void die(const char *errstr, ...);

/* **** */

int isprotodel(struct client *c);
struct monitor *dirtomon(int dir);
struct client *nexttiled(struct client *c);
struct monitor *ptrtomon(int x, int y);
void setclientstate(struct client *c, long state);
void showhide(struct client *c);
int textw(cairo_t *cr, const char *text);
void unfocus(struct client *c, int setfocus);

void windowtype_update(struct client *c);
void updatesizehints(struct client *c);
void updatewmhints(struct client *c);

#endif
