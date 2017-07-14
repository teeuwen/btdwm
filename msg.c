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

#include "msg.h"

NotifyNotification *msg_create(const char *str, int timeout, int destroy)
{
	NotifyNotification *n;

	n = notify_notification_new(str, NULL, 0);

	if (timeout < 0)
		timeout = NOTIFY_EXPIRES_DEFAULT;
	notify_notification_set_timeout(n, timeout);

	notify_notification_show(n, 0);

	if (destroy)
		msg_destroy(n);

	return n;
}

NotifyNotification *msg_update(NotifyNotification *n, const char *str,
		int timeout)
{
	if (!n)
		return msg_create(str, timeout, 0);

	notify_notification_update(n, str, NULL, 0);

	if (timeout < 0)
		timeout = NOTIFY_EXPIRES_DEFAULT;
	notify_notification_set_timeout(n, timeout);

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
