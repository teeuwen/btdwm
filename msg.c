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

#include "msg.h"

NotifyNotification *msg_create(const char *title, const char *body,
		int t, int destroy)
{
	NotifyNotification *n;

	n = notify_notification_new(title, body, 0);

	if (t < 0)
		t = NOTIFY_EXPIRES_DEFAULT;
	notify_notification_set_timeout(n, t);

	notify_notification_show(n, 0);

	if (destroy)
		msg_destroy(n);

	return n;
}

NotifyNotification *msg_update(NotifyNotification *n,
		const char *title, const char *body, int t)
{
	if (!n)
		return msg_create(title, body, t, 0);

	notify_notification_update(n, title, body, 0);

	if (t < 0)
		t = NOTIFY_EXPIRES_DEFAULT;
	notify_notification_set_timeout(n, t);

	notify_notification_show(n, 0);

	return n;
}

void msg_destroy(NotifyNotification *n)
{
	g_object_unref(G_OBJECT(n));
}

void msg_init(void)
{
	notify_init("btdwm");
}

void msg_quit(void)
{
	notify_uninit();
}
