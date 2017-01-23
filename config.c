/*
 * TODO
 * Relocate in header file
 * Set NET_ACTIVE_WINDOW
 */

#include "btdwm.h"
#include "functions.h"
#include "layouts.h"
#include "keysym.h"

/*
 * Appearance
 */

const char font_desc[]		= "Broad 5.8";

const char bg_normal[]		= "#222222";
const char bg_focus[]		= "#1C1C1C";
const char bg_center[]		= "#444444";

const char fg_normal[]		= "#ECECEC";
const char fg_light[]		= "#AAAAAA";

const char status_active[]	= "#83A598";
const char status_focus[]	= "#DFDFDF";
const char status_urgent[]	= "#A57074";


/*
 * Layouts
 */

struct layout layouts[] = {
	/* Symbol	Name		Layout	mfact */
/* 0 */	{ "[F]",	"Floating",	NULL,	0.0f },
/* 1 */	{ "[T]",	"Tile",		&tile,	0.5f },
/* 2 */	{ "[M]",	"Max",		&max,	0.0f }
};
const int layouts_len = sizeof(layouts) / sizeof(layouts[0]);


/*
 * Tags
 */

const struct tag tags[] = {
	/* Name		Layout */
	{ "λ",		&layouts[1] },
	{ "α",		&layouts[2] },
	{ "β",		&layouts[0] },
	{ "γ",		&layouts[0] },
	{ "δ",		&layouts[1] },
	{ "μ",		&layouts[2] },
	{ "π",		&layouts[1] },
};
const int tags_len = sizeof(tags) / sizeof(tags[0]);

#if 0
static const char *tags[] = {
	/* "A", "B", "C", "D", "E", "F", "G" */
	"1", "2", "3", "4", "5", "6", "7"
	/* "λ", "α", "β", "γ", "δ", "μ", "π" */
	/* "主", "次", "造", "声", "像", "全", "媕" */
	/* "완", "투", "티", "포", "핍", "싯", "센" */
};
#endif


/*
 * Rules
 */

const struct rule rules[] = {
	/* Class	Instance	Title	Mask	Float	Monitor */
	{ "NULL",	NULL,		NULL,	0,	0,	-1 }
};
const int rules_len = sizeof(rules) / sizeof(rules[0]);


/*
 * Key bindings
 */

static const char *cmd_poweroff[] = { SHCMD("systemctl poweroff") , 0 };
static const char *cmd_reboot[] = { SHCMD("systemctl reboot") , 0 };
static const char *cmd_suspend[] = { SHCMD("systemctl suspend") , 0 };
static const char *cmd_halt[] = { SHCMD("systemctl halt") , 0 };

static const char *cmd_lock[] = { SHCMD("~/Documents/cs/scripts/lock/lock.sh") , 0 };

static const char *cmd_run[] = { SHCMD("dmenu_run -fn 'Noto Sans Mono-8' -nb '#444444'"), 0 };
static const char *cmd_term[]  = { "st", 0 };

static const char *cmd_w[] = { SHCMD("WINEARCH=win32 WINEPREFIX=~/.office2010 wine ~/.office2010/drive_c/Program\\ Files/Microsoft\\ Office/Office14/WINWORD.EXE"), 0 };
static const char *cmd_e[] = { SHCMD("WINEARCH=win32 WINEPREFIX=~/.office2010 wine ~/.office2010/drive_c/Program\\ Files/Microsoft\\ Office/Office14/EXCEL.EXE"), 0 };
static const char *cmd_r[] = { SHCMD("WINEARCH=win32 WINEPREFIX=~/.office2010 wine ~/.office2010/drive_c/Program\\ Files/Microsoft\\ Office/Office14/POWERPNT.EXE"), 0 };
static const char *cmd_y[] = { "lowriter", 0 };
static const char *cmd_u[] = { "localc", 0 };
static const char *cmd_i[] = { "loimpress", 0 };
static const char *cmd_d[] = { "audacity", 0 };
static const char *cmd_f[] = { "firefox", 0 };
static const char *cmd_g[] = { "gimp", 0 };
static const char *cmd_k[] = { "keepassx", 0 };
static const char *cmd_c[] = { "chromium", 0 };
static const char *cmd_v[] = { "virtualbox", 0 };
static const char *cmd_n[] = { SHCMD("chromium -incognito"), 0 };

static const char *cmd_scrot[] = { SHCMD("scrot"), 0 };

/* TODO OSD */
static const char *cmd_play[] = { SHCMD("mpc toggle"), 0 };
static const char *cmd_stop[] = { SHCMD("mpc stop"), 0 };
static const char *cmd_prev[] = { SHCMD("mpc prev"), 0 };
static const char *cmd_next[] = { SHCMD("mpc next"), 0 };

/* TODO OSD */
static const char *cmd_vdec[] = { SHCMD("amixer set Master 5%-"), 0 };
static const char *cmd_vinc[] = { SHCMD("amixer set Master 5%+"), 0 };
static const char *cmd_mute[] = { SHCMD("amixer set Master toggle"), 0 };

/* TODO OSD */
static const char *cmd_bdec[] = { SHCMD("xbacklight -dec 5"), 0 };
static const char *cmd_binc[] = { SHCMD("xbacklight -inc 5"), 0 };

const struct key keys[] = {
	/* Modifier		Key	Function	Arguments */
	{ K_SUPER | K_CTRL,	K_Q,	quit,		{ 0 } },

	/* Power */
	{ K_SUPER | K_CTRL,	K_ESC,	spawn,		{ .v = cmd_poweroff } },
	{ K_SUPER | K_CTRL,	K_F1,	spawn,		{ .v = cmd_reboot } },
	{ K_SUPER | K_CTRL,	K_F2,	spawn,		{ .v = cmd_suspend } },
	{ K_SUPER | K_CTRL,	K_F3,	spawn,		{ .v = cmd_halt } },

	/* Locking */
	{ K_SUPER,		K_L,	spawn,		{ .v = cmd_lock } },

	/* Applications */
	{ K_SUPER,		K_R,	spawn,		{ .v = cmd_run } },
	{ K_SUPER,		K_RET,	spawn,		{ .v = cmd_term } },

	{ K_CTRL | K_SHIFT,	K_W,	spawn,		{ .v = cmd_w } },
	{ K_CTRL | K_SHIFT,	K_E,	spawn,		{ .v = cmd_e } },
	{ K_CTRL | K_SHIFT,	K_R,	spawn,		{ .v = cmd_r } },
	{ K_CTRL | K_SHIFT,	K_Y,	spawn,		{ .v = cmd_y } },
	{ K_CTRL | K_SHIFT,	K_U,	spawn,		{ .v = cmd_u } },
	{ K_CTRL | K_SHIFT,	K_I,	spawn,		{ .v = cmd_i } },
	{ K_CTRL | K_SHIFT,	K_D,	spawn,		{ .v = cmd_d } },
	{ K_CTRL | K_SHIFT,	K_F,	spawn,		{ .v = cmd_f } },
	{ K_CTRL | K_SHIFT,	K_G,	spawn,		{ .v = cmd_g } },
	{ K_CTRL | K_SHIFT,	K_K,	spawn,		{ .v = cmd_k } },
	{ K_CTRL | K_SHIFT,	K_C,	spawn,		{ .v = cmd_c } },
	{ K_CTRL | K_SHIFT,	K_V,	spawn,		{ .v = cmd_v } },
	{ K_CTRL | K_SHIFT,	K_N,	spawn,		{ .v = cmd_n } },

	/* Screenshot */
	{ K_SUPER,		K_PRINT,spawn,		{ .v = cmd_scrot } },

	/* Media keys */
	{ 0,			K_PLAY,	spawn,		{ .v = cmd_play } },
	{ 0,			K_STOP,	spawn,		{ .v = cmd_stop } },
	{ 0,			K_PREV,	spawn,		{ .v = cmd_prev } },
	{ 0,			K_NEXT,	spawn,		{ .v = cmd_next } },
	{ 0,			K_K,	spawn,		{ .v = cmd_play } },
	{ 0,			K_H,	spawn,		{ .v = cmd_stop } },
	{ 0,			K_J,	spawn,		{ .v = cmd_prev } },
	{ 0,			K_L,	spawn,		{ .v = cmd_next } },

	/* Volume control */
	{ 0,			K_VDEC,	spawn,		{ .v = cmd_vdec } },
	{ 0,			K_VINC,	spawn,		{ .v = cmd_vinc } },
	{ 0,			K_MUTE,	spawn,		{ .v = cmd_mute } },
	{ 0,			K_M,	spawn,		{ .v = cmd_stop } },
	{ 0,			K_PER,	spawn,		{ .v = cmd_prev } },
	{ 0,			K_COM,	spawn,		{ .v = cmd_next } },

	/* Backlight control */
	{ 0,			K_BDEC,	spawn,		{ .v = cmd_bdec } },
	{ 0,			K_BINC,	spawn,		{ .v = cmd_binc } },

	/* TODO Battery status */

	/* Global */
	{ K_SUPER,		K_F,	togglebar,	{ 0 } },

	{ K_SUPER | K_SHIFT,	K_SPACE,setlayout,	{ .i = -1 } },
	{ K_SUPER,		K_SPACE,setlayout,	{ .i = +1 } },

	/* { K_SUPER | K_SHIFT,	K_TAB,	viewtag,	{ .i = -1 } }, FIXME */
	/* { K_SUPER,		K_TAB,	viewtag,	{ .i = +1 } }, FIXME */

	{ K_SUPER,		K_UP,	focusstack,	{ .i = -1 } },
	{ K_SUPER,		K_DOWN,	focusstack,	{ .i = +1 } },
	{ K_SUPER,		K_LEFT,	focusstack,	{ .i = -1 } },
	{ K_SUPER,		K_RIGHT,focusstack,	{ .i = +1 } },
	{ K_ALT | K_SHIFT,	K_TAB,	focusstack,	{ .i = -1 } },
	{ K_ALT,		K_TAB,	focusstack,	{ .i = +1 } },

	/* Tags */
	{ TAGKEYS(K_1, 0) },
	{ TAGKEYS(K_2, 1) },
	{ TAGKEYS(K_3, 2) },
	{ TAGKEYS(K_4, 3) },
	{ TAGKEYS(K_5, 4) },
	{ TAGKEYS(K_6, 5) },
	{ TAGKEYS(K_7, 6) },

	/* { K_SUPER | K_SHIFT,	K_TAB,	focusstack,	{ .i = -1 } },
	{ K_SUPER,		K_TAB,	focusstack,	{ .i = +1 } }, */

	/* Client */
	{ K_SUPER,		K_Q,	killclient,	{ 0 } },
	{ K_SUPER | K_CTRL,	K_SPACE,togglefloating,	{ 0 } },

	{ K_SUPER | K_SHIFT,	K_LEFT,	setmfact,	{ .f = -0.05 } },
	{ K_SUPER | K_SHIFT,	K_RIGHT,setmfact,	{ .f = +0.05 } },

	/* Multihead */
	{ K_SUPER,		K_BL,	focusmon,	{ .i = -1 } },
	{ K_SUPER,		K_BR,	focusmon,	{ .i = +1 } },
	{ K_SUPER,		K_O,	tagmon,		{ .i = -1 } }

#if 0
	/* Notifications */
	{ K_SUPER,		K_C,	not_close,	{ 0 } },
	{ K_SUPER,		K_X,	not_closeall,	{ 0 } },
	{ K_SUPER,		K_V,	not_history,	{ 0 } },
	{ K_SUPER,		K_B,	not_context,	{ 0 } },
#endif

	/* TODO */
};
const int keys_len = sizeof(keys) / sizeof(keys[0]);


/*
 * Mouse bindings
 */

const struct button buttons[] = {
	/* Event	Mask	Button	Function	Arguments */
	{ ClkTagBar,	0,	B_LEFT,	viewtag,	{ 0 } },
	{ ClkTagBar,	0,	B_RIGHT,toggletag,	{ 0 } },
	{ ClkTagBar,	K_SUPER,B_LEFT,	moveclient,	{ 0 } },

	{ ClkClientWin,	K_SUPER,B_LEFT,	movemouse,	{ 0 } },
	{ ClkClientWin,	K_SUPER,B_RIGHT,resizemouse,	{ 0 } }

};
const int buttons_len = sizeof(buttons) / sizeof(buttons[0]);


/*
 * Misc.
 */

/* Update timeout */
const unsigned int tdelay = 2;
