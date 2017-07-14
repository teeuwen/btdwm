#include "functions.h"
#include "keysym.h"
#include "layouts.h"

/*
 * Appearance
 */

static const char font_desc[]		= "sans-serif 5.8";

static const char bg[]			= "#1C1C1C";

static const char fg[]			= "#ECECEC";
static const char fg_light[]		= "#AAAAAA";

static const char status_active[]	= "#83A598";
static const char status_focus[]	= "#DFDFDF";
static const char status_urgent[]	= "#F0C674";


/*
 * Layouts
 */

static struct layout layouts[] = {
	/* Name		Layout */
	{ "Floating",	NULL },
	{ "Tile",	&tile },
	{ "Max",	&max }
};


/*
 * Tags
 */

static const struct tag tags[] = {
	/* Name				Layout */
	{ /* "A", */ /* "α", */ /* "1", */ /* "一", */ "일",
					&layouts[1] },
	{ /* "B", */ /* "β", */ /* "2", */ /* "二", */ "이",
					&layouts[2] },
	{ /* "C", */ /* "γ", */ /* "3", */ /* "三", */ "삼",
					&layouts[0] },
	{ /* "D", */ /* "δ", */ /* "4", */ /* "四", */ "사",
					&layouts[0] },
	{ /* "E", */ /* "ε", */ /* "5", */ /* "五", */ "오",
					&layouts[0] },
	{ /* "F", */ /* "ζ", */ /* "6", */ /* "六", */ "육",
					&layouts[2] },
	{ /* "G", */ /* "η", */ /* "7", */ /* "七", */ "칠",
					&layouts[1] }
};


/*
 * Rules
 */

static const struct rule rules[] = {
	/* Class	Instance	Title		Float.	Transparent */
	{ "URxvt",	NULL,		NULL,		0,	1 },
	{ "URxvt",	NULL,		"mutt",		0,	0 },
	{ "URxvt",	NULL,		"transmission-remote-cli",0,	0 },
	{ "URxvt",	NULL,		"VIM",		0,	0 },
	{ "Firefox",	NULL,		"Page Info",	1,	0 },
	/* { "feh",	NULL,		NULL,		0,	0 }, */
	{ NULL,		NULL,		"QEMU",		1,	0 },
	{ NULL,		NULL,		"pinentry",	1,	0 }
};


/*
 * Hooks
 */

static const struct hook hooks[] = {
	/* Hook			Command */
	{ NULL,			NULL }
};


/*
 * Key bindings
 */

static const char *cmd_eject[] = { SHCMD("eject -t"), 0 };

static const char *cmd_poweroff[] = { SHCMD("openrc-shutdown -p"), 0 };
static const char *cmd_reboot[] = { SHCMD("openrc-shutdown -r"), 0 };

static const char *cmd_lock[] = { SHCMD("~/.scripts/lock/lock.sh"), 0 };

static const char *cmd_run[] = { SHCMD("dmenu_run -fn 'Inconsolata\\-g for Powerline-8' -nb '#1C1C1C' -nf '#AAAAAA' -sb '#1C1C1C' -sf '#ECECEC'"), 0 };
static const char *cmd_term[]  = { "urxvt", 0 };

static const char *cmd_w[] = { "lowriter", 0 };
static const char *cmd_e[] = { "localc", 0 };
static const char *cmd_r[] = { "loimpress", 0 };
static const char *cmd_u[] = { SHCMD("usermenu -fn 'Inconsolata\\-g for Powerline-8' -nb '#1C1C1C' -nf '#AAAAAA' -sb '#1C1C1C' -sf '#ECECEC'"), 0 };
static const char *cmd_o[] = { "tor-browser-en", 0 };
static const char *cmd_p[] = { SHCMD("passmenu -fn 'Inconsolata\\-g for Powerline-8' -nb '#1C1C1C' -nf '#AAAAAA' -sb '#1C1C1C' -sf '#ECECEC'"), 0 };
static const char *cmd_s[] = { SHCMD("firefox-nightly --private-window about:privatebrowsing"), 0 };
static const char *cmd_f[] = { "firefox-nightly", 0 };
static const char *cmd_g[] = { "gimp", 0 };
static const char *cmd_v[] = { SHCMD("~/.scripts/vm/spice.sh 'Windows 7 Professional x86_64'"), 0 };

static const char *cmd_scrot[] = { SHCMD("~/.scripts/scrot.sh"), 0 };

static const char *cmd_toggle[] = { SHCMD("~/.scripts/osd/mpd.sh toggle"), 0 };
static const char *cmd_stop[] = { SHCMD("~/.scripts/osd/mpd.sh stop"), 0 };
static const char *cmd_prev[] = { SHCMD("~/.scripts/osd/mpd.sh prev"), 0 };
static const char *cmd_next[] = { SHCMD("~/.scripts/osd/mpd.sh next"), 0 };

static const char *cmd_mute[] = { SHCMD("~/.scripts/osd/vol.sh mute"), 0 };
static const char *cmd_vdec[] = { SHCMD("~/.scripts/osd/vol.sh dec"), 0 };
static const char *cmd_vinc[] = { SHCMD("~/.scripts/osd/vol.sh inc"), 0 };

static const char *cmd_bdec[] = { SHCMD("~/.scripts/osd/bri.sh dec"), 0 };
static const char *cmd_binc[] = { SHCMD("~/.scripts/osd/bri.sh inc"), 0 };

static const char *cmd_ttg[] = { SHCMD("~/.scripts/osd/ttg.sh"), 0 };
static const char *cmd_mtg[] = { SHCMD("~/.scripts/osd/mtg.sh"), 0 };

static const char *cmd_status[] = { SHCMD("~/.scripts/osd/status.sh"), 0 };

static const struct key keys[] = {
	/* Modifier		Key	Function	union arguments */
	{ K_SUPER | K_CTRL,	K_Q,	quit,		{ 0 } },

	{ 0,			K_PWR,	spawn,		{ .v = cmd_eject } },

	/* Power */
	{ K_SUPER | K_CTRL,	K_ESC,	spawn,		{ .v = cmd_poweroff } },
	{ K_SUPER | K_CTRL,	K_F1,	spawn,		{ .v = cmd_reboot } },

	/* Locking */
	{ K_SUPER,		K_L,	spawn,		{ .v = cmd_lock } },

	/* Applications */
	{ K_SUPER,		K_R,	spawn,		{ .v = cmd_run } },
	{ K_SUPER,		K_RET,	spawn,		{ .v = cmd_term } },

	{ K_CTRL | K_SHIFT,	K_W,	spawn,		{ .v = cmd_w } },
	{ K_CTRL | K_SHIFT,	K_E,	spawn,		{ .v = cmd_e } },
	{ K_CTRL | K_SHIFT,	K_R,	spawn,		{ .v = cmd_r } },
	{ K_CTRL | K_SHIFT,	K_U,	spawn,		{ .v = cmd_u } },
	{ K_CTRL | K_SHIFT,	K_O,	spawn,		{ .v = cmd_o } },
	{ K_CTRL | K_SHIFT,	K_P,	spawn,		{ .v = cmd_p } },
	{ K_CTRL | K_SHIFT,	K_S,	spawn,		{ .v = cmd_s } },
	{ K_CTRL | K_SHIFT,	K_F,	spawn,		{ .v = cmd_f } },
	{ K_CTRL | K_SHIFT,	K_G,	spawn,		{ .v = cmd_g } },
	{ K_CTRL | K_SHIFT,	K_V,	spawn,		{ .v = cmd_v } },

	/* Screenshot */
	{ K_SUPER,		K_PRINT,spawn,		{ .v = cmd_scrot } },

	/* Media keys */
	{ 0,			K_PLAY,	spawn,		{ .v = cmd_toggle } },
	{ 0,			K_STOP,	spawn,		{ .v = cmd_stop } },
	{ 0,			K_PREV,	spawn,		{ .v = cmd_prev } },
	{ 0,			K_NEXT,	spawn,		{ .v = cmd_next } },
	{ K_SUPER,		K_S,	spawn,		{ .v = cmd_toggle } },
	{ K_SUPER,		K_W,	spawn,		{ .v = cmd_stop } },
	{ K_SUPER,		K_A,	spawn,		{ .v = cmd_prev } },
	{ K_SUPER,		K_D,	spawn,		{ .v = cmd_next } },

	/* Volume control */
	{ 0,			K_MUTE,	spawn,		{ .v = cmd_mute } },
	{ 0,			K_VDEC,	spawn,		{ .v = cmd_vdec } },
	{ 0,			K_VINC,	spawn,		{ .v = cmd_vinc } },
	{ K_SUPER,		K_M,	spawn,		{ .v = cmd_mute } },
	{ K_SUPER,		K_COM,	spawn,		{ .v = cmd_vdec } },
	{ K_SUPER,		K_PER,	spawn,		{ .v = cmd_vinc } },

	/* Backlight control */
	{ 0,			K_BDEC,	spawn,		{ .v = cmd_bdec } },
	{ 0,			K_BINC,	spawn,		{ .v = cmd_binc } },
	{ K_SUPER,		K_J,	spawn,		{ .v = cmd_bdec } },
	{ K_SUPER,		K_K,	spawn,		{ .v = cmd_binc } },

	/* Touch screen */
	{ 0,			K_EXP,	spawn,		{ .v = cmd_ttg } },
	{ K_SUPER,		K_SCOL,	spawn,		{ .v = cmd_ttg } },

	/* Touchpad */
	{ 0,			K_LA,	spawn,		{ .v = cmd_mtg } },
	{ K_SUPER,		K_APO,	spawn,		{ .v = cmd_mtg } },

	/* Status */
	{ K_SUPER,		K_Z,	spawn,		{ .v = cmd_status } },

	/* Global */
	{ K_SUPER,		K_F,	togglebar,	{ 0 } },

	{ K_SUPER | K_SHIFT,	K_SPACE,setlayout,	{ .i = -1 } },
	{ K_SUPER,		K_SPACE,setlayout,	{ .i = +1 } },

	{ K_SUPER,		K_UP,	focusclient,	{ .i = -1 } },
	{ K_SUPER,		K_DOWN,	focusclient,	{ .i = +1 } },
	{ K_SUPER,		K_LEFT,	focusclient,	{ .i = -1 } },
	{ K_SUPER,		K_RIGHT,focusclient,	{ .i = +1 } },
	{ K_ALT | K_SHIFT,	K_TAB,	focusclient,	{ .i = -2 } },
	{ K_ALT,		K_TAB,	focusclient,	{ .i = +2 } },

	/* Tags */
	{ TAGKEYS(K_1, 0) },
	{ TAGKEYS(K_2, 1) },
	{ TAGKEYS(K_3, 2) },
	{ TAGKEYS(K_4, 3) },
	{ TAGKEYS(K_5, 4) },
	{ TAGKEYS(K_6, 5) },
	{ TAGKEYS(K_7, 6) },

	{ K_SUPER | K_SHIFT,	K_TAB,	viewtag,	{ .i = -2 } },
	{ K_SUPER,		K_TAB,	viewtag,	{ .i = -1 } },

	/* Client */
	{ K_SUPER,		K_Q,	killclient,	{ 0 } },

	{ K_SUPER | K_CTRL,	K_SPACE,togglefloating,	{ 0 } },
	{ K_SUPER,		K_T,	toggleontop,	{ 0 } },
	{ K_SUPER,		K_Y,	togglesticky,	{ 0 } },

	{ K_SUPER | K_SHIFT,	K_LEFT,	setmfact,	{ .f = -0.05 } },
	{ K_SUPER | K_SHIFT,	K_RIGHT,setmfact,	{ .f = +0.05 } },

	/* Multihead */
	{ K_SUPER,		K_BL,	focusmon,	{ .i = -1 } },
	{ K_SUPER,		K_BR,	focusmon,	{ .i = +1 } },
	{ K_SUPER,		K_O,	sendmon,	{ .i = +1 } }
};


/*
 * Mouse bindings
 */

static const struct button buttons[] = {
	/* Event	Mask	Button	Function	union arguments */
	{ H_TAG,	0,	B_LEFT,	viewtag,	{ 0 } },
	{ H_TAG,	0,	B_RIGHT,toggletag,	{ 0 } },
	{ H_TAG,	K_SUPER,B_LEFT,	sendtag,	{ 0 } },

	{ H_CLIENT,	K_SUPER,B_LEFT,	moveclientm,	{ 0 } },
	{ H_CLIENT,	K_SUPER,B_RIGHT,resizeclientm,	{ 0 } }

};
