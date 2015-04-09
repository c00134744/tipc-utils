/*
 * Copyright (c) 2015, Ericsson AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <libmnl/libmnl.h>

#include "cmdl.h"

const struct cmd *find_cmd(const struct cmd *cmds, char *str)
{
	const struct cmd *c;
	const struct cmd *match = NULL;

	for (c = cmds; c->cmd; c++) {
		if (strstr(c->cmd, str) != c->cmd)
			continue;
		if (match)
			return NULL;
		match = c;
	}

	return match;
}

static struct opt *find_opt(struct opt *opts, char *str)
{
	struct opt *o;
	struct opt *match = NULL;

	for (o = opts; o->key; o++) {
		if (strstr(o->key, str) != o->key)
			continue;
		if (match)
			return NULL;

		match = o;
	}

	return match;
}

struct opt *get_opt(struct opt *opts, char *key)
{
	struct opt *o;

	for (o = opts; o->key; o++) {
		if (strcmp(o->key, key) == 0 && o->val)
			return o;
	}

	return NULL;
}

char *shift_cmdl(struct cmdl *cmdl)
{
	int next;

	if (cmdl->optind < cmdl->argc)
		next = (cmdl->optind)++;
	else
		next = cmdl->argc;

	return cmdl->argv[next];
}

/* Returns the number of options parsed or a negative error code upon failure */
int parse_opts(struct opt *opts, struct cmdl *cmdl)
{
	int i;
	int cnt = 0;

	for (i = cmdl->optind; i < cmdl->argc; i += 2) {
		struct opt *o;

		o = find_opt(opts, cmdl->argv[i]);
		if (!o) {
			fprintf(stderr, "error, invalid option \"%s\"\n",
					cmdl->argv[i]);
			return -EINVAL;
		}
		cnt++;
		o->val = cmdl->argv[i + 1];
		cmdl->optind += 2;
	}

	return cnt;
}

int run_cmd(struct nlmsghdr *nlh, const struct cmd *caller,
	    const struct cmd *cmds, struct cmdl *cmdl, void *data)
{
	char *name;
	const struct cmd *cmd;

	if ((cmdl->optind) >= cmdl->argc) {
		if (caller->help)
			(caller->help)(cmdl);
		return -EINVAL;
	}
	name = cmdl->argv[cmdl->optind];
	(cmdl->optind)++;


	cmd = find_cmd(cmds, name);
	if (!cmd) {
		/* Show help about last command if we don't find this one */
		if (help_flag && caller->help) {
			(caller->help)(cmdl);
		} else {
			fprintf(stderr, "error, invalid command \"%s\"\n", name);
			fprintf(stderr, "use --help for command help\n");
		}
		return -EINVAL;
	}

	return (cmd->func)(nlh, cmd, cmdl, data);
}
