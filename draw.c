/*
 *
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

#include <cairo/cairo-xcb.h>
#include <pango/pangocairo.h>

#include <string.h>
#include <stdlib.h>

#include "btdwm.h"

PangoFontDescription *font;
double colors[COLOR_MAX][3];

/* TODO There's a CPP for this kind of stuff */
static void rgb_set(int index, const char *col)
{
	char r[] = { col[1], col[2], '\0' };
	char g[] = { col[3], col[4], '\0' };
	char b[] = { col[5], col[6], '\0' };

	colors[index][0] = strtol(r, 0, 16) / 255.0;
	colors[index][1] = strtol(g, 0, 16) / 255.0;
	colors[index][2] = strtol(b, 0, 16) / 255.0;
}

static void rgba_get(struct monitor *m, cairo_t *cr, int palette, int status,
		int bg, double *r, double *g, double *b, double *a)
{
	if (bg) {
		if (m->barcr == cr && (!m->layouts[m->tag]->arrange ||
				!m->client || ISFLOATING(m->client) ||
				ISTRANSPARENT(m->client))) {
			*r = *g = *b = 0.0;
			*a = 0.75;
		} else {
			*r = colors[COLOR_BG][0];
			*g = colors[COLOR_BG][1];
			*b = colors[COLOR_BG][2];
			*a = 1.0;
		}

		return;
	}

	*a = 1.0;

	switch (palette) {
	case PLT_ACTIVE:
		if (status) {
			*r = colors[COLOR_STACTIVE][0];
			*g = colors[COLOR_STACTIVE][1];
			*b = colors[COLOR_STACTIVE][2];
			break;
		}
	case PLT_CENLIGHT:
	case PLT_INACTIVE:
		*r = colors[COLOR_FGLIGHT][0];
		*g = colors[COLOR_FGLIGHT][1];
		*b = colors[COLOR_FGLIGHT][2];
		break;
	case PLT_CENTER:
	case PLT_FOCUS:
		if (status) {
			*r = colors[COLOR_STFOCUS][0];
			*g = colors[COLOR_STFOCUS][1];
			*b = colors[COLOR_STFOCUS][2];
		} else {
			*r = colors[COLOR_FG][0];
			*g = colors[COLOR_FG][1];
			*b = colors[COLOR_FG][2];
		}
		break;
	case PLT_URGENT:
		*r = colors[COLOR_STURGENT][0];
		*g = colors[COLOR_STURGENT][1];
		*b = colors[COLOR_STURGENT][2];
		break;
	}
}

int textw(cairo_t *cr, const char *text)
{
	PangoLayout *layout;
	int w;

	layout = pango_cairo_create_layout(cr);

	pango_layout_set_font_description(layout, font);
	pango_layout_set_text(layout, text, strlen(text));

	pango_cairo_update_layout(cr, layout);

	pango_layout_get_pixel_size(layout, &w, NULL);

	g_object_unref(G_OBJECT(layout));

	return w;
}

static int texth(cairo_t *cr, const char *text)
{
	PangoLayout *layout;
	int h;

	layout = pango_cairo_create_layout(cr);

	pango_layout_set_font_description(layout, font);
	pango_layout_set_text(layout, text, strlen(text));

	pango_cairo_update_layout(cr, layout);

	pango_layout_get_pixel_size(layout, NULL, &h);

	g_object_unref(G_OBJECT(layout));

	return h;
}

void rectangle_draw(struct monitor *m, cairo_t *cr, int x, int y, int w, int h,
		int palette)
{
	double r, g, b, a;

	rgba_get(m, cr, palette, 0, 1, &r, &g, &b, &a);
	cairo_set_source_rgba(cr, r, g, b, a);

	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);
}

void text_draw(struct monitor *m, cairo_t *cr, int x, int y, int w, int h,
		const char *text, int palette)
{
	PangoLayout *layout;
	double r, g, b, a;

	layout = pango_cairo_create_layout(cr);

	cairo_move_to(cr, x + 4, y + (h / 2) - (texth(cr, text) / 2));

	rgba_get(m, cr, palette, 0, 0, &r, &g, &b, &a);
	cairo_set_source_rgba(cr, r, g, b, a);

	pango_layout_set_font_description(layout, font);
	pango_layout_set_text(layout, text, -1);

	pango_cairo_update_layout(cr, layout);
	pango_cairo_show_layout(cr, layout);

	g_object_unref(G_OBJECT(layout));
}

void status_draw(struct monitor *m, int x, int y, int w, int palette)
{
	double r, g, b, a;

	rgba_get(m, m->barcr, palette, 1, 0, &r, &g, &b, &a);
	cairo_set_source_rgba(m->barcr, r, g, b, a);

	cairo_rectangle(m->barcr, x, y, w, 1);
	cairo_fill(m->barcr);
}

void font_init(void)
{
	font = pango_font_description_from_string(font_desc);
	if (!font)
		die("invalid font: %s\n", font);

	rgb_set(COLOR_BG, bg);
	rgb_set(COLOR_FG, fg);
	rgb_set(COLOR_FGLIGHT, fg_light);
	rgb_set(COLOR_STACTIVE, status_active);
	rgb_set(COLOR_STFOCUS, status_focus);
	rgb_set(COLOR_STURGENT, status_urgent);
}

void font_quit(void)
{
	pango_font_description_free(font);
}
