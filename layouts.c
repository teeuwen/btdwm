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

#include "btdwm.h"

void tile(struct monitor *m)
{
	struct client *c;
	int x, y, h, w, mw;
	unsigned int i, n;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (!n)
		return;

	c = nexttiled(m->clients);
	mw = m->layouts[m->tag]->mfact * m->w;
	resize(c, m->x, m->y + ((m->showbar) ? BAR_HEIGHT : 0),
			(n == 1 ? m->w : mw),
			m->h - ((m->showbar) ? BAR_HEIGHT : 0), 0);
	if (!--n)
		return;

	x = (m->x + mw > c->x + c->w) ? c->x + c->w : m->x + mw;
	y = m->y + ((m->showbar) ? BAR_HEIGHT : 0);
	w = (m->x + mw > c->x + c->w) ? m->x + m->w - x : m->w - mw;
	h = (m->h - ((m->showbar) ? BAR_HEIGHT : 0)) / n;

	if (h < BAR_HEIGHT)
		h = m->h - ((m->showbar) ? BAR_HEIGHT : 0);

	for (i = 0, c = nexttiled(c->next); c; c = nexttiled(c->next), i++) {
		resize(c, x, y, w, ((i + 1 == n) ? m->y + m->h - y : h), 0);

		if (h != m->h - ((m->showbar) ? BAR_HEIGHT : 0))
			y = c->y + c->h;
	}
}

void col(struct monitor *m)
{
#if 0
	struct client *c;
	unsigned int i, n, h, w, x, y, mw;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (!n)
		return;

	if (n > m->nmaster
#endif
}

void max(struct monitor *m)
{
	struct client *c;

	for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
		resize(c, m->x, m->y + ((m->showbar) ? BAR_HEIGHT : 0),
				m->w, m->h - ((m->showbar) ? BAR_HEIGHT : 0), 0);
}
