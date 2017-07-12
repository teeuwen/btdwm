/*
 *
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

#ifndef _X11_KEYSYM_H
#define _X11_KEYSYM_H

#include <X11/keysym.h>
#include <X11/XF86keysym.h>

/* TODO Finish */

#define K_SHIFT	XCB_MOD_MASK_SHIFT
#define K_CTRL	XCB_MOD_MASK_CONTROL
#define K_SUPER	XCB_MOD_MASK_4
#define K_ALT	XCB_MOD_MASK_1
#define B_LEFT	XCB_BUTTON_INDEX_1
#define B_MID	XCB_BUTTON_INDEX_2
#define B_RIGHT	XCB_BUTTON_INDEX_3

#define K_ESC	XK_Escape
#define K_GRAVE	XK_grave
#define K_F1	XK_F1
#define K_F2	XK_F2
#define K_F3	XK_F3
#define K_F4	XK_F4
#define K_F5	XK_F5
#define K_F6	XK_F6
#define K_F7	XK_F7
#define K_F8	XK_F8
#define K_F9	XK_F9
#define K_F10	XK_F10
#define K_F11	XK_F11
#define K_F12	XK_F12

#define K_1	XK_1
#define K_2	XK_2
#define K_3	XK_3
#define K_4	XK_4
#define K_5	XK_5
#define K_6	XK_6
#define K_7	XK_7
#define K_8	XK_8
#define K_9	XK_9
#define K_0	XK_0
#define K_MIN	XK_minus
#define K_EQ	XK_equal
#define K_BCKSP	XK_BackSpace

#define K_TAB	XK_Tab
#define K_Q	XK_q
#define K_W	XK_w
#define K_E	XK_e
#define K_R	XK_r
#define K_T	XK_t
#define K_Y	XK_y
#define K_U	XK_u
#define K_I	XK_i
#define K_O	XK_o
#define K_P	XK_p
#define K_BL	XK_bracketleft
#define K_BR	XK_bracketright

#define K_A	XK_a
#define K_S	XK_s
#define K_D	XK_d
#define K_F	XK_f
#define K_G	XK_g
#define K_H	XK_h
#define K_J	XK_j
#define K_K	XK_k
#define K_L	XK_l
#define K_SCOL	XK_semicolon
#define K_APO	XK_apostrophe
#define K_RET	XK_Return

#define K_Z	XK_z
#define K_X	XK_x
#define K_C	XK_c
#define K_V	XK_v
#define K_B	XK_b
#define K_N	XK_n
#define K_M	XK_m
#define K_PER	XK_period
#define K_COM	XK_comma

#define K_SPACE	XK_space

#define K_PRINT	XK_Print

#define K_INS	XK_Insert
#define K_HOME	XK_Home
#define K_PGUP	XK_Page_Up
#define K_DEL	XK_Delete
#define K_END	XK_End
#define K_PGDN	XK_Page_Down

#define K_UP	XK_Up
#define K_DOWN	XK_Down
#define K_LEFT	XK_Left
#define K_RIGHT	XK_Right

#define K_NUML	XK_Num_Lock

#define K_PLAY	XF86XK_AudioPlay
#define K_STOP	XF86XK_AudioStop
#define K_PREV	XF86XK_AudioPrev
#define K_NEXT	XF86XK_AudioNext

#define K_VDEC	XF86XK_AudioLowerVolume
#define K_VINC	XF86XK_AudioRaiseVolume
#define K_MUTE	XF86XK_AudioMute

#define K_BDEC	XF86XK_MonBrightnessDown
#define K_BINC	XF86XK_MonBrightnessUp

#define K_EXP	XF86XK_Explorer

#define K_LA	XF86XK_LaunchA

#define K_PWR	XF86XK_PowerOff

#endif
