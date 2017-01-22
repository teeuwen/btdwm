#include "libnotify.h"

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
