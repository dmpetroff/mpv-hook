#include <X11/Xlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <libnotify/notify.h>
#include <glib.h>
#include "hook_lib.h"

/**
 * T(type) - functor that accepts type
 * N(id) - functor that accepts name
 * A(type, id) - functor that accepts argument type and name
 * AB - arguments begin
 * AS - arguments separator
 * AE - arguments end
 */
#define pf_XCreateWindow(T, N, A, AB, AS, AE) \
	T(Window) \
	N(XCreateWindow) \
	AB \
    A(Display*, display) AS \
    A(Window, parent) AS \
    A(int, x) AS \
    A(int, y) AS \
    A(unsigned int, width) AS \
    A(unsigned int, height) AS \
    A(unsigned int, border_width) AS \
    A(int, depth) AS \
    A(unsigned int, class) AS \
    A(Visual*, visual) AS \
    A(unsigned long, valuemask) AS \
    A(XSetWindowAttributes*, attributes) \
	AE

FUNC_TYPEDEF(pf_XCreateWindow);
FUNC_PTRDEF(pf_XCreateWindow);

static pid_t mpv_master;
static pid_t ntfy_child = 0;

FUNC_DEF(pf_XCreateWindow)
{
	Window w = FUNC_FWD(pf_XCreateWindow);
	printf("XCreateWindow => %lx\n", w);
	if (ntfy_child) {
		kill(ntfy_child, SIGUSR1);
		ntfy_child = 0;
	}
	return w;
}

static void
clear_ld_preload()
{
	extern char **environ;
	char **e = environ;
	while (*e && strcmp(*e, "LD_PRELOAD") != 0)
		e++;

	if (*e) {
		do {
			*e = *(e + 1);
			e++;
		} while (*e);
	}
}

static NotifyNotification *ntfy;

static void on_sig(int sig)
{
	int status;

	switch (sig) {
	case SIGUSR1:
		if (ntfy != NULL) {
			notify_notification_close(ntfy, NULL);
			ntfy = NULL;
		}
		exit(EXIT_SUCCESS);
		break;
	case SIGUSR2:
		if (wait(&status) == ntfy_child) {
			ntfy_child = 0;
			puts("reaped child");
		}
		break;
	}
}

static void
kill_mpv(NotifyNotification *n, char *action, void *arg)
{
	kill(mpv_master, SIGINT);
}

static const char*
guess_url(int argc, char **argv)
{
	const char *uri = getenv("MPV_URI");
	if (uri != NULL)
		return uri;

	for (int i = 1; i < argc; i++)
		if (strncmp(argv[i], "https://", 8) == 0 || strncmp(argv[i], "http://", 7) == 0)
			return argv[i];

	return "<some_video>";
}

static void
ntfy_main(pid_t mpv_master, int argc, char **argv)
{
	NotifyNotification *n;
	GMainLoop *l;
	const char *uri;

	l = g_main_loop_new(NULL, FALSE);
	notify_init("mpv");

	signal(SIGUSR1, on_sig);
	prctl(PR_SET_PDEATHSIG, SIGUSR1);
	
	uri = guess_url(argc, argv);
	n = notify_notification_new("mpv", uri, NULL);
	notify_notification_set_timeout(n, NOTIFY_EXPIRES_NEVER);
	notify_notification_add_action(n, "kill", "Cancel", kill_mpv, NULL, NULL);
	ntfy = n;

	if (!notify_notification_show(n, NULL)) {
		puts("FAILED TO SHOW!");
	}

	g_main_loop_run(l);
	exit(EXIT_SUCCESS);
}

static int init(int argc, char **argv, char **environ)
{
	clear_ld_preload();

	HOOK_NF(pf_XCreateWindow);
	if (fp_XCreateWindow == NULL)
		return 0;	/* this will happen in child processes (yt-dlp) */

	/* To reap notify child */
	signal(SIGUSR2, on_sig);

	mpv_master = getpid();
	ntfy_child = fork();
	switch (ntfy_child) {
	case 0:
		ntfy_main(mpv_master, argc, argv);
		break;
	case -1:
		exit(EXIT_FAILURE);
	default:
		break;
	}

	return 0;
}

__attribute__((section(".init_array"))) void *ctor = &init;
