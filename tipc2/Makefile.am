sbin_PROGRAMS=tipc2
tipc2_SOURCES=tipc2.c \
	bearer.c bearer.h \
	cmdl.c cmdl.h \
	link.c link.h \
	media.c media.h \
	misc.c misc.h \
	msg.c msg.h \
	nametable.c nametable.h \
	node.c node.h \
	socket.c socket.h
tipc2_CFLAGS=-Werror $(LIBMNL_CFLAGS)
tipc2_LDADD=-lrt $(LIBMNL_LIBS) -lmnl
tipc2_LDFLAGS=-I /usr/include/libmnl/
