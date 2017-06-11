/*
 * TODO
 * Relocate in header file
 */

#include "btdwm.h"
#include "functions.h"
#include "layouts.h"
#include "keysym.h"

/*
 * Appearance
 */

const char font_desc[]		= "sans-serif 5.8";

const char bg[]			= "#1C1C1C";

const char fg[]			= "#ECECEC";
const char fg_light[]		= "#AAAAAA";

const char status_active[]	= "#83A598";
const char status_focus[]	= "#DFDFDF";
const char status_urgent[]	= "#F0C674";


/*
 * Layouts
 */

struct layout layouts[] = {
	/* Symbol	Name		Layout	mfact */
	{ "[F]",	"Floating",	NULL,	0.0 },
	{ "[T]",	"Tile",		&tile,	0.5 },
	{ "[M]",	"Max",		&max,	0.0 }
};
const int layouts_len = sizeof(layouts) / sizeof(layouts[0]);


/*
 * Tags
 */

const struct tag tags[] = {
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
const int tags_len = sizeof(tags) / sizeof(tags[0]);


/*
 * Rules
 */

const struct rule rules[] = {
	/* Class	Instance	Title		Monitor	Tags	Float.	Transparent */
	{ "st-256color",NULL,		NULL,		-1,	0,	0,	1 },
	{ "st-256color",NULL,		"vim",		-1,	0,	0,	0 },
	{ "st-256color",NULL,		"VIM",		-1,	0,	0,	0 },
	{ "Firefox",	NULL,		"Page Info",	-1,	0,	1,	0 },
	{ NULL,		NULL,		"QEMU",		-1,	0,	1,	0 },
	{ NULL,		NULL,		"pinentry",	-1,	0,	1,	0 },
};
const int rules_len = sizeof(rules) / sizeof(rules[0]);


/*
 * Key bindings
 */

static const char *cmd_eject[] = { SHCMD("eject -t"), 0 };

static const char *cmd_poweroff[] = { SHCMD("systemctl poweroff"), 0 };
static const char *cmd_reboot[] = { SHCMD("systemctl reboot"), 0 };
static const char *cmd_suspend[] = { SHCMD("systemctl suspend"), 0 };
static const char *cmd_halt[] = { SHCMD("halt"), 0 };

static const char *cmd_lock[] = { SHCMD("~/.scripts/wm/lock/lock.sh"), 0 };

static const char *cmd_run[] = { SHCMD("dmenu_run -fn 'Inconsolata-g for Powerline-8' -nb '#1C1C1C' -nf '#AAAAAA' -sb '#1C1C1C' -sf '#ECECEC' -l 16"), 0 };
static const char *cmd_term[]  = { "st", 0 };

static const char *cmd_q[] = { SHCMD("qutebrowser --backend webengine") };
static const char *cmd_w[] = { SHCMD("WINEARCH=win32 WINEPREFIX=~/.office2010 wine ~/.office2010/drive_c/Program\\ Files/Microsoft\\ Office/Office14/WINWORD.EXE"), 0 };
static const char *cmd_e[] = { SHCMD("WINEARCH=win32 WINEPREFIX=~/.office2010 wine ~/.office2010/drive_c/Program\\ Files/Microsoft\\ Office/Office14/EXCEL.EXE"), 0 };
static const char *cmd_r[] = { SHCMD("WINEARCH=win32 WINEPREFIX=~/.office2010 wine ~/.office2010/drive_c/Program\\ Files/Microsoft\\ Office/Office14/POWERPNT.EXE"), 0 };
static const char *cmd_y[] = { "lowriter", 0 };
static const char *cmd_u[] = { "localc", 0 };
static const char *cmd_i[] = { "loimpress", 0 };
static const char *cmd_p[] = { SHCMD("firefox-nightly --private-window about:privatebrowsing"), 0 };
static const char *cmd_a[] = { "audacity", 0 };
static const char *cmd_s[] = { "tor-browser-en", 0 };
static const char *cmd_f[] = { "firefox-nightly", 0 };
static const char *cmd_g[] = { "gimp", 0 };
static const char *cmd_j[] = { SHCMD("usermenu -fn 'Inconsolata-g for Powerline-8' -nb '#1C1C1C' -nf '#AAAAAA' -sb '#1C1C1C' -sf '#ECECEC' -l 16"), 0 };
static const char *cmd_k[] = { SHCMD("passmenu -fn 'Inconsolata-g for Powerline-8' -nb '#1C1C1C' -nf '#AAAAAA' -sb '#1C1C1C' -sf '#ECECEC' -l 16"), 0 };
static const char *cmd_l[] = { "kdenlive", 0 };
static const char *cmd_v[] = { "virtualbox", 0 };

static const char *cmd_scrot[] = { SHCMD("scrot"), 0 };

static const char *cmd_toggle[] = { SHCMD("~/.scripts/wm/mpd.sh toggle"), 0 };
static const char *cmd_stop[] = { SHCMD("~/.scripts/wm/mpd.sh stop"), 0 };
static const char *cmd_prev[] = { SHCMD("~/.scripts/wm/mpd.sh prev"), 0 };
static const char *cmd_next[] = { SHCMD("~/.scripts/wm/mpd.sh next"), 0 };

static const char *cmd_mute[] = { SHCMD("~/.scripts/wm/vol.sh mute"), 0 };
static const char *cmd_vdec[] = { SHCMD("~/.scripts/wm/vol.sh dec"), 0 };
static const char *cmd_vinc[] = { SHCMD("~/.scripts/wm/vol.sh inc"), 0 };

static const char *cmd_bdec[] = { SHCMD("~/.scripts/wm/bri.sh dec"), 0 };
static const char *cmd_binc[] = { SHCMD("~/.scripts/wm/bri.sh inc"), 0 };

static const char *cmd_status[] = { SHCMD("~/.scripts/wm/status.sh"), 0 };

const struct key keys[] = {
	/* Modifier		Key	Function	union arguments */
	{ K_SUPER | K_CTRL,	K_Q,	quit,		{ 0 } },

	{ 0,			K_PWR,	spawn,		{ .v = cmd_eject } },

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

	{ K_CTRL | K_SHIFT,	K_Q,	spawn,		{ .v = cmd_q } },
	{ K_CTRL | K_SHIFT,	K_W,	spawn,		{ .v = cmd_w } },
	{ K_CTRL | K_SHIFT,	K_E,	spawn,		{ .v = cmd_e } },
	{ K_CTRL | K_SHIFT,	K_R,	spawn,		{ .v = cmd_r } },
	{ K_CTRL | K_SHIFT,	K_Y,	spawn,		{ .v = cmd_y } },
	{ K_CTRL | K_SHIFT,	K_U,	spawn,		{ .v = cmd_u } },
	{ K_CTRL | K_SHIFT,	K_I,	spawn,		{ .v = cmd_i } },
	{ K_CTRL | K_SHIFT,	K_P,	spawn,		{ .v = cmd_p } },
	{ K_CTRL | K_SHIFT,	K_A,	spawn,		{ .v = cmd_a } },
	{ K_CTRL | K_SHIFT,	K_S,	spawn,		{ .v = cmd_s } },
	{ K_CTRL | K_SHIFT,	K_F,	spawn,		{ .v = cmd_f } },
	{ K_CTRL | K_SHIFT,	K_G,	spawn,		{ .v = cmd_g } },
	{ K_CTRL | K_SHIFT,	K_J,	spawn,		{ .v = cmd_j } },
	{ K_CTRL | K_SHIFT,	K_K,	spawn,		{ .v = cmd_k } },
	{ K_CTRL | K_SHIFT,	K_L,	spawn,		{ .v = cmd_l } },
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

	/* Status */
	{ K_SUPER,		K_Z,	spawn,		{ .v = cmd_status } },

	/* Global */
	{ K_SUPER,		K_F,	togglebar,	{ 0 } },
	{ K_SUPER,		K_G,	togglegaps,	{ 0 } },

	{ K_SUPER | K_SHIFT,	K_SPACE,setlayout,	{ .i = -1 } },
	{ K_SUPER,		K_SPACE,setlayout,	{ .i = +1 } },

	{ K_SUPER,		K_UP,	focusstack,	{ .i = -1 } },
	{ K_SUPER,		K_DOWN,	focusstack,	{ .i = +1 } },
	{ K_SUPER,		K_LEFT,	focusstack,	{ .i = -1 } },
	{ K_SUPER,		K_RIGHT,focusstack,	{ .i = +1 } },
	{ K_ALT | K_SHIFT,	K_TAB,	focusstack,	{ .i = -2 } },
	{ K_ALT,		K_TAB,	focusstack,	{ .i = +2 } },

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
	{ K_SUPER,		K_O,	tagmon,		{ .i = +1 } }
};
const int keys_len = sizeof(keys) / sizeof(keys[0]);


/*
 * Mouse bindings
 */

const struct button buttons[] = {
	/* Event	Mask	Button	Function	union arguments */
	{ CLICK_TAGS,	0,	B_LEFT,	viewtag,	{ 0 } },
	{ CLICK_TAGS,	0,	B_RIGHT,toggletag,	{ 0 } },
	{ CLICK_TAGS,	K_SUPER,B_LEFT,	moveclient,	{ 0 } },

	{ CLICK_CLIENT,	K_SUPER,B_LEFT,	movemouse,	{ 0 } },
	{ CLICK_CLIENT,	K_SUPER,B_RIGHT,resizemouse,	{ 0 } }

};
const int buttons_len = sizeof(buttons) / sizeof(buttons[0]);
