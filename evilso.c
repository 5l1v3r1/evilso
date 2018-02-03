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

static void log_packet(int, const void *, size_t);

ssize_t (*orig_send)(int, const void *, size_t, int);

static void
log_packet(int s, const void *msg, size_t len)
{
	printf("Socket %d sending msg of size %zu\n", s, len);
	printf("%s\n", msg);
}

ssize_t
send(int s, const void *msg, size_t len, int flags)
{
	ssize_t res;
	size_t i;
	int found;
	char *search_str[] = {
		"GET",
		"HTTP",
		NULL
	};

	res = orig_send(s, msg, len, flags);
	found = 1;
	for (i = 0; search_str[i] != NULL; i++) {
		if (!memmem(msg, len, search_str[i],
		    strlen(search_str[i]))) {
			found = 0;
		}
	}

	if (found) {
		log_packet(s, msg, len);
	}

	return (res);
}

__attribute__((constructor)) void
init(void)
{
	void *handle;
       
	handle = dlopen("/lib/libc.so.7",
	    RTLD_GLOBAL | RTLD_LAZY);
	if (handle == NULL) {
		exit(1);
	}

	orig_send = dlsym(handle, "send");
	if (orig_send == NULL) {
		exit(1);
	}
}
