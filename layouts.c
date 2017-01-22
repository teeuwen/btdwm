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
	resize(c, m->x, m->y + ((m->showbar) ? 16 : 0),
			(n == 1 ? m->w : mw),
			m->h - ((m->showbar) ? 16 : 0), 0);
	if (!--n)
		return;

	x = (m->x + mw > c->x + c->w) ? c->x + c->w : m->x + mw;
	y = m->y + ((m->showbar) ? 16 : 0);
	w = (m->x + mw > c->x + c->w) ? m->x + m->w - x : m->w - mw;
	h = (m->h - ((m->showbar) ? 16 : 0)) / n;

	if (h < 16)
		h = m->h - ((m->showbar) ? 16 : 0);

	for (i = 0, c = nexttiled(c->next); c; c = nexttiled(c->next), i++) {
		resize(c, x, y, w, ((i + 1 == n) ? m->y + m->h - y : h), 0);

		if (h != m->h - ((m->showbar) ? 16 : 0))
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
		resize(c, m->x, m->y + ((m->showbar) ? 16 : 0),
				m->w, m->h - ((m->showbar) ? 16 : 0), 0);
}
