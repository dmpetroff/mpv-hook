CFLAGS := -Wall -O2 -D_GNU_SOURCE $(shell pkg-config --cflags libnotify)
LIBS := $(shell pkg-config --libs libnotify)

-include Makefile.local

hook.so: hook.c hook_lib.h
	$(CC) -g -o $@ -fPIC -shared $(CFLAGS) $< $(LIBS)
