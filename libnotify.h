#include <libnotify/notify.h>

void msg_destroy(NotifyNotification *n);

NotifyNotification *msg_create(const char *title, const char *body,
		int t, int destroy);
NotifyNotification *msg_update(NotifyNotification *n,
		const char *title, const char *body, int timeout);

void msg_init(void);
void msg_quit(void);
