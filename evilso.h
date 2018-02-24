#ifndef _EVILSO_H
#define	_EVILSO_H

typedef struct _cmd {
	const char	*c_cmdstr;
	void		(*c_act)(int, char *, size_t);
} cmd_t;

extern cmd_t cmds[];

void run_cmds(int, char *, size_t);

#endif
