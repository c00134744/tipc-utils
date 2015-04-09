/*
 * Copyright (c) 2014, Ericsson AB
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
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include "bearer.h"
#include "link.h"
#include "nametable.h"
#include "socket.h"
#include "media.h"
#include "node.h"
#include "cmdl.h"

int help_flag;

static void about(struct cmdl *cmdl)
{
	fprintf(stderr,
		"Transparent Inter-Process Communication Protocol\n"
		"Usage: %s [OPTIONS] COMMAND [ARGS] ...\n"
		"\n"
		"Options:\n"
		" -h, --help \t\tPrint help for last given command\n"
		"\n"
		"Commands:\n"
		" bearer                - Show or modify bearers\n"
		" link                  - Show or modify links\n"
		" media                 - Show or modify media\n"
		" nametable             - Show nametable\n"
		" node                  - Show or modify node related parameters\n"
		" socket                - Show sockets\n",
		cmdl->argv[0]);
}

int main(int argc, char *argv[])
{
	int i;
	int res;
	struct cmdl cmdl;
	const struct cmd cmd = {"tipc", NULL, about};
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};
	const struct cmd cmds[] = {
		{ "bearer",	cmd_bearer,	cmd_bearer_help},
		{ "link",	cmd_link,	cmd_link_help},
		{ "media",	cmd_media,	cmd_media_help},
		{ "nametable",	cmd_nametable,	cmd_nametable_help},
		{ "node",	cmd_node,	cmd_node_help},
		{ "socket",	cmd_socket,	cmd_socket_help},
		{ NULL }
	};

	do {
		int option_index = 0;

		i = getopt_long(argc, argv, "h", long_options, &option_index);

		switch (i) {
		case 'h':
			/*
			 * We want the help for the last command, so we flag
			 * here in order to print later.
			 */
			help_flag = 1;
			break;
		case -1:
			/* End of options */
			break;
		default:
			/* Invalid option, error msg is printed by getopts */
			return 1;
		}
	} while (i != -1);

	cmdl.optind = optind;
	cmdl.argc = argc;
	cmdl.argv = argv;

	if ((res = run_cmd(NULL, &cmd, cmds, &cmdl, NULL)) != 0)
		return 1;

	return 0;
}
