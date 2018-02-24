SHLIB=		evilso
SHLIB_MAJOR=	0
SRCS=		cmds.c evilso.c
MAN=

CFLAGS+=	-I/usr/local/include \
		-I/usr/src/libexec/rtld-elf \
		-I/usr/src/libexec/rtld-elf/${MACHINE_ARCH}
LDFLAGS+=	-L/usr/local/lib

LDADD+=	-lsqlite3 -lhijack

.include <bsd.lib.mk>
