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

#include "btdwm.h"

PangoFontDescription *font;
double colors[COLOR_MAX][3];

static void rgb_set(int index, const char *col)
{
	char r[] = { col[1], col[2], '\0' };
	char g[] = { col[3], col[4], '\0' };
	char b[] = { col[5], col[6], '\0' };

	colors[index][0] = strtol(r, 0, 16) / 255.0f;
	colors[index][1] = strtol(g, 0, 16) / 255.0f;
	colors[index][2] = strtol(b, 0, 16) / 255.0f;
}

static void rgb_get(cairo_t *cr, int palette, int status, int bg,
		double *r, double *g, double *b)
{
	switch (palette) {
	case PLT_CENTER:
		if (!bg) {
			*r = colors[COLOR_FGNORMAL][0];
			*g = colors[COLOR_FGNORMAL][1];
			*b = colors[COLOR_FGNORMAL][2];
			break;
		}
	case PLT_CENLIGHT:
		if (bg) {
			*r = colors[COLOR_BGCENTER][0];
			*g = colors[COLOR_BGCENTER][1];
			*b = colors[COLOR_BGCENTER][2];
			break;
		}
	case PLT_ACTIVE:
		if (status) {
			*r = colors[COLOR_STACTIVE][0];
			*g = colors[COLOR_STACTIVE][1];
			*b = colors[COLOR_STACTIVE][2];
			break;
		}
	case PLT_INACTIVE:
		if (bg) {
			*r = colors[COLOR_BGNORMAL][0];
			*g = colors[COLOR_BGNORMAL][1];
			*b = colors[COLOR_BGNORMAL][2];
		} else {
			*r = colors[COLOR_FGLIGHT][0];
			*g = colors[COLOR_FGLIGHT][1];
			*b = colors[COLOR_FGLIGHT][2];
		}
		break;
	case PLT_FOCUS:
		if (status) {
			*r = colors[COLOR_STFOCUS][0];
			*g = colors[COLOR_STFOCUS][1];
			*b = colors[COLOR_STFOCUS][2];
		} else if (bg) {
			*r = colors[COLOR_BGFOCUS][0];
			*g = colors[COLOR_BGFOCUS][1];
			*b = colors[COLOR_BGFOCUS][2];
		} else {
			*r = colors[COLOR_FGNORMAL][0];
			*g = colors[COLOR_FGNORMAL][1];
			*b = colors[COLOR_FGNORMAL][2];
		}
		break;
	case PLT_URGENT:
			*r = colors[COLOR_STURGENT][0];
			*g = colors[COLOR_STURGENT][1];
			*b = colors[COLOR_STURGENT][2];
		break;
	}
}

/* FIXME This is a hack for now */
void gradient_draw(cairo_t *cr, int x, int y, int w, int h,
		int palette1, int palette2)
{
	cairo_pattern_t *grad;
	double r, g, b;

	grad = cairo_pattern_create_linear(x, y, x + w, y);

	rgb_get(cr, palette1, 0, 1, &r, &g, &b);
	cairo_pattern_add_color_stop_rgb(grad, 0.0, r, g, b);
	rgb_get(cr, palette2, 0, 1, &r, &g, &b);
	cairo_pattern_add_color_stop_rgb(grad, 1.0, r, g, b);

	cairo_set_source(cr, grad);

	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);

	cairo_pattern_destroy(grad);
}

void rectangle_draw(cairo_t *cr, int x, int y, int w, int h, int palette)
{
	double r, g, b;

	rgb_get(cr, palette, 0, 1, &r, &g, &b);
	cairo_set_source_rgb(cr, r, g, b);

	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);
}

void status_draw(cairo_t *cr, int x, int y, int w, int palette)
{
	double r, g, b;

	rgb_get(cr, palette, 1, 1, &r, &g, &b);
	cairo_set_source_rgb(cr, r, g, b);

	cairo_rectangle(cr, x, y, w, 1);
	cairo_fill(cr);
}

void text_draw(cairo_t *cr, int x, int y, int w, int h,
		const char *text, int palette)
{
	PangoLayout *layout;
	double r, g, b;

	layout = pango_cairo_create_layout(cr);

	rectangle_draw(cr, x, y, w, h, palette);

	/* FIXME TODO XXX */
	cairo_move_to(cr, x + 4, y + 5);

	rgb_get(cr, palette, 0, 0, &r, &g, &b);
	cairo_set_source_rgb(cr, r, g, b);

	pango_layout_set_font_description(layout, font);
	pango_layout_set_text(layout, text, -1);

	pango_cairo_update_layout(cr, layout);
	pango_cairo_show_layout(cr, layout);

	g_object_unref(G_OBJECT(layout));
}

/* TODO Use dummy cairo cr */
int textnw(cairo_t *cr, const char *text, unsigned int len)
{
	PangoLayout *layout;
	int w;

	layout = pango_cairo_create_layout(cr);

	pango_layout_set_font_description(layout, font);
	pango_layout_set_text(layout, text, strlen(text));

	pango_cairo_update_layout(cr, layout);

	pango_layout_get_pixel_size(layout, &w, 0);

	g_object_unref(G_OBJECT(layout));

	return w;
}

void font_init(void)
{
	font = pango_font_description_from_string(font_desc);
	if (!font)
		die("invalid font: %s\n", font);

	rgb_set(COLOR_BGCENTER, bg_center);
	rgb_set(COLOR_BGNORMAL, bg_normal);
	rgb_set(COLOR_BGFOCUS, bg_focus);
	rgb_set(COLOR_FGNORMAL, fg_normal);
	rgb_set(COLOR_FGLIGHT, fg_light);
	rgb_set(COLOR_STACTIVE, status_active);
	rgb_set(COLOR_STFOCUS, status_focus);
	rgb_set(COLOR_STURGENT, status_urgent);
}

void font_quit(void)
{
	pango_font_description_free(font);
}
