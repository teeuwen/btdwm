#ifndef _DWM_H
#define _DWH_H

#include <cairo/cairo-xcb.h>

#include <stdio.h>
#include <stdlib.h>

/* To avoid conflicts with limits.h */
#define MAX_NAME	256

/* FIXME */
#define INRECT(x,y,rx,ry,rw,rh)	((x) >= (rx) && (x) < (rx) + (rw) && (y) >= (ry) && (y) < (ry) + (rh))
#define ISVISIBLE(c)		((c->tags & c->mon->tags))
#define LENGTH(x)		(sizeof(x) / sizeof(x[0]))
#ifndef MAX
#   define MAX(a, b)		((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#   define MIN(a, b)		((a) < (b) ? (a) : (b))
#endif
#define TEXTW(c, x)		(textnw(c, x, strlen(x)) + 8)

/* EWMH atoms */
#define NetSupported	0
#define NetWMName	1
#define NetWMState	2
#define NetWMWindowType	3
#define NetWMFullscreen	4
#define NetLast		5

/* Default atoms */
#define WMProtocols	0
#define WMDelete	1
#define WMState		2
#define WMLast		3

/* Click events */
#define ClkTagBar	1
#define ClkClientWin	2

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

typedef union {
	int		i;
	unsigned int	ui;
	double		f;
	const void	*v;
} Arg;

struct monitor {
	int		id;
	int		x, y, w, h;

	xcb_window_t	barwin;
	cairo_surface_t	*barsur;
	cairo_t		*barcr;
	int		showbar, redrawbar;

#if 0
	struct tag	*tags[LENGTH(tags);
	unsigned int	tag;
#else
	unsigned int	tags;
	unsigned int	tag;
	struct layout	*layouts[7];
	/* unsigned int	tags[LENGTH(tags)]; FIXME FIXME FIXME FIXME FIXME */

	struct client	*clients;
	struct client	*client;
#endif	

	struct monitor	*next;
};

#if 0
struct tag {
	const struct tagi	*info;

	const struct layouti	*dlayout;
	double			mfact;

	struct client	*clients;
	struct client	*client;
};
#endif

struct client {
	struct monitor	*mon;
	xcb_window_t	win;

	char		name[MAX_NAME];
	int		isfixed, isfloating, oldstate;

	unsigned int	tags;

	int		x, y, w, h;
	double		mina, maxa;
	int		oldx, oldy, oldw, oldh;
	int		basew, baseh, incw, inch, maxw, maxh, minw, minh;

	struct client	*prev, *next;
};

/* struct layouti { */
struct layout {
	const char	*symbol;
	const char	*name;
	void		(*arrange) (struct monitor *);

	double		mfact;
};

/* struct tagi { */
struct tag {
	const char	*name;
	struct layout	*layout;
};

/* struct rulei { */
struct rule {
	const char	*class;
	const char	*instance;
	const char	*title;

	unsigned int	tags;
	int		isfloating;
	int		monitor;
};

#define TAGKEYS(k,t) \
	K_SUPER,		k,	viewtag,	{ .i = t } }, \
	{ K_SUPER | K_CTRL,	k,	toggletag,	{ .i = t } }, \
	{ K_SUPER | K_SHIFT,	k,	moveclient,	{ .i = t } \

#define SHCMD(cmd) "/bin/sh", "-c", cmd

/* struct keyi { */
struct key {
	unsigned int	mod;
	xcb_keysym_t	keysym;
	void		(*func) (const Arg *);
	const Arg	arg;
};

/* struct buttoni { */
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
extern struct monitor *mons;
extern struct monitor *selmon;
extern xcb_window_t root;
extern xcb_screen_t *screen; /* XXX TEMP */

extern xcb_cursor_t cursor[];

extern xcb_atom_t wmatom[], netatom[];
extern struct dc dc;

void attach(struct client *c);
void detach(struct client *c);
void restack(struct monitor *m);

void client_resize(struct client *c, int x, int y, int w, int h);
void resize(struct client *c, int x, int y, int w, int h, int interact);

void arrange(struct monitor *m);

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
void bars_update(void);

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
int textprop_get(xcb_window_t w, xcb_atom_t atom, char *text, unsigned int size);

xcb_atom_t atom_add(const char *name);
xcb_atom_t atom_get(xcb_window_t w, xcb_atom_t atom);

void urgent_clear(struct client *c);

int xinerama_init(void);
void events_init(void);
void font_init(void);
void mon_init(void);
void xcb_init(void);

void font_quit(void);
void mon_quit(void);
void xcb_quit(void);

void setup(void);
void run(void);

void quit(const Arg *arg);
void die(const char *errstr, ...);

/* **** */

int applysizehints(struct client *c, int *x, int *y, int *w, int *h, int interact);
struct monitor *dirtomon(int dir);
void focus(struct client *c);
int isprotodel(struct client *c);
struct client *nexttiled(struct client *c);
struct monitor *ptrtomon(int x, int y);
void setclientstate(struct client *c, long state);
void showhide(struct client *c);
int textnw(cairo_t *cr, const char *text, unsigned int len);
void unfocus(struct client *c, int setfocus);

void windowtype_update(struct client *c);
void updatesizehints(struct client *c);
void updatewmhints(struct client *c);

#endif
