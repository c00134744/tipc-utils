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
#include <time.h>
#include <errno.h>

#include <linux/tipc_netlink.h>
#include <linux/tipc.h>
#include <linux/genetlink.h>
#include <libmnl/libmnl.h>

int parse_attrs(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	tb[type] = attr;

	return MNL_CB_OK;
}

static int family_id_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[CTRL_ATTR_MAX+1];
	struct genlmsghdr *genl = mnl_nlmsg_get_payload(nlh);
	int *id = data;

	mnl_attr_parse(nlh, sizeof(*genl), parse_attrs, tb);
	if (!tb[CTRL_ATTR_FAMILY_ID])
		return MNL_CB_ERROR;

	*id = mnl_attr_get_u16(tb[CTRL_ATTR_FAMILY_ID]);

	return MNL_CB_OK;
}

static struct mnl_socket *msg_send(struct nlmsghdr *nlh)
{
	int ret;
	struct mnl_socket *nl;

	nl = mnl_socket_open(NETLINK_GENERIC);
	if (nl == NULL) {
		perror("mnl_socket_open");
		return NULL;
	}

	ret = mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID);
	if (ret < 0) {
		perror("mnl_socket_bind");
		return NULL;
	}

	ret = mnl_socket_sendto(nl, nlh, nlh->nlmsg_len);
	if (ret < 0) {
		perror("mnl_socket_send");
		return NULL;
	}

	return nl;
}

static int msg_recv(struct mnl_socket *nl, mnl_cb_t callback, void *data, int seq)
{
	int ret;
	unsigned int portid;
	char buf[MNL_SOCKET_BUFFER_SIZE];

	portid = mnl_socket_get_portid(nl);

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, callback, data);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1)
		perror("error");

	mnl_socket_close(nl);

	return ret;
}

static int msg_query(struct nlmsghdr *nlh, mnl_cb_t callback, void *data)
{
	unsigned int seq;
	struct mnl_socket *nl;

	seq = time(NULL);
	nlh->nlmsg_seq = seq;

	nl = msg_send(nlh);
	if (!nl)
		return -ENOTSUP;

	return msg_recv(nl, callback, data, seq);
}

static int get_family(void)
{
	int err;
	int nl_family;
	struct nlmsghdr *nlh;
	struct genlmsghdr *genl;
	char buf[MNL_SOCKET_BUFFER_SIZE];

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type	= GENL_ID_CTRL;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;

	genl = mnl_nlmsg_put_extra_header(nlh, sizeof(struct genlmsghdr));
	genl->cmd = CTRL_CMD_GETFAMILY;
	genl->version = 1;

	mnl_attr_put_u32(nlh, CTRL_ATTR_FAMILY_ID, GENL_ID_CTRL);
	mnl_attr_put_strz(nlh, CTRL_ATTR_FAMILY_NAME, TIPC_GENL_V2_NAME);

	if ((err = msg_query(nlh, family_id_cb, &nl_family)))
		return err;

	return nl_family;
}

int msg_doit(struct nlmsghdr *nlh, mnl_cb_t callback, void *data)
{
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	return msg_query(nlh, callback, data);
}

int msg_dumpit(struct nlmsghdr *nlh, mnl_cb_t callback, void *data)
{
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	return msg_query(nlh, callback, data);
}

struct nlmsghdr *msg_init(char *buf, int cmd, int flags)
{
	int family;
	struct nlmsghdr *nlh;
	struct genlmsghdr *genl;

	family = get_family();
	if (family <= 0) {
		fprintf(stderr,
			"Unable to get TIPC nl family id (module loaded?)\n");
		return NULL;
	}

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type	= family;

	genl = mnl_nlmsg_put_extra_header(nlh, sizeof(struct genlmsghdr));
	genl->cmd = cmd;
	genl->version = 1;

	return nlh;
}
