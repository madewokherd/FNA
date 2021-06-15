C_SRCS = \
    main.c \

H_SRCS = \
    $(SRCDIR)/../../wine/debug.h \
    $(SRCDIR)/../../wine/heap.h \

all: FNAMF.dll
.PHONY: all

%.o: $(SRCDIR)/%.c $(H_SRCS)
	$(CC) -o $@ -D_WIN32_WINNT=0x0602 --std=c99 -c -Wall -Wno-format -Werror -Wno-pragma-pack -I$(SRCDIR)/../.. -I$(SRCDIR)/../../wine $<

debug.o: $(SRCDIR)/../../wine/debug.c $(H_SRCS)
	$(CC) -o $@ -D_WIN32_WINNT=0x0602 -D__WINESRC__ --std=c99 -c -Wall -Wno-format -Werror -I$(SRCDIR)/../.. -I$(SRCDIR)/../../wine $<

FNAMF.dll: $(C_SRCS:%.c=%.o) debug.o
	$(CC) -o $@ -shared -Wl,--kill-at $(C_SRCS:%.c=%.o) debug.o -luuid -lmfplat -lmfreadwrite -lmfuuid
