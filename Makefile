SHLIB=		evilso
SHLIB_MAJOR=	0
SRCS=		evilso.c
MAN=

CFLAGS+=	-I/usr/local/include
LDFLAGS+=	-L/usr/local/lib

LDADD+=	-lsqlite3

.include <bsd.lib.mk>
