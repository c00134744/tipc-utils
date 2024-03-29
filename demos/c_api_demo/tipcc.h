/* ------------------------------------------------------------------------
 *
 * tipcc.h
 *
 * Short description: TIPC C binding API. The complete TIPC socket API with all
 *                    standard features is available in <linux/tipc.h>
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2015, Ericsson Canada
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * Neither the name of Ericsson Canada nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
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
 *
 * Version 0.9.2: Jon Maloy, 2015
 *
 * ------------------------------------------------------------------------
 */

#ifndef __TIPCC_H_
#define __TIPCC_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/tipc.h>

/* Addressing:
 * - struct tipc_addr may contain <type != 0, instance, domain> or
 *   <type == 0, instance == port, domain == node>
 * - tipc_domain_t has internal structure <zone.cluster.node>. Valid
 *   contents <z.c.n>, <z.c.0>, <z.0.0> or <0.0.0>, where 0 is wildcard
 */
typedef uint32_t tipc_domain_t;

struct tipc_addr {
	uint32_t      type;
	uint32_t      instance;
	tipc_domain_t domain;
};

tipc_domain_t tipc_own_node(void);
tipc_domain_t tipc_own_cluster(void);
tipc_domain_t tipc_own_zone(void);
static unsigned int tipc_zone(tipc_domain_t domain);
static unsigned int tipc_cluster(tipc_domain_t domain);
static unsigned int tipc_node(tipc_domain_t domain);
tipc_domain_t tipc_domain(unsigned int zone, unsigned int cluster,
			  unsigned int node);
char* tipc_ntoa(const struct tipc_addr *addr, char *buf, size_t len);
char* tipc_dtoa(tipc_domain_t domain, char *buf, size_t len);
char* tipc_rtoa(uint32_t type, uint32_t lower, uint32_t upper,
		tipc_domain_t domain, char *buf, size_t len);

/* Socket:
 * - 'Rejectable': sent messages will return if rejected at destination
 */
int tipc_socket(int sk_type);
int tipc_sock_non_block(int sd);
int tipc_sock_rejectable(int sd);
int tipc_close(int sd);
int tipc_sockid(int sd, struct tipc_addr *id);

int tipc_bind(int sd, uint32_t type, uint32_t upper, uint32_t lower,
	      tipc_domain_t scope);
int tipc_unbind(int sd, uint32_t type, uint32_t upper, uint32_t lower);

int tipc_connect(int sd, const struct tipc_addr *dst);
int tipc_listen(int sd, int backlog);
int tipc_accept(int sd, struct tipc_addr *src);

/* Messaging:
 * - NULL pointer parameters are always accepted
 * - tipc_sendto() to an accepting socket initiates two-way connect
 * - If no err pointer given, tipc_recvfrom() returns 0 on rejected message
 * - If (*err != 0) buf contains a potentially truncated rejected message
 */
int tipc_recvfrom(int sd, char *buf, size_t len, struct tipc_addr *src,
		  struct tipc_addr *dst, int *err);
int tipc_recv(int sd, char* buf, size_t len, bool waitall);
int tipc_sendmsg(int sd, const struct msghdr *msg);
int tipc_sendto(int sd, const char *msg, size_t len,
		const struct tipc_addr *dst);
int tipc_send(int sd, const char *msg, size_t len);
int tipc_mcast(int sd, const char *msg, size_t len,
	       const struct tipc_addr *dst);

/* Topology Server:
 * - Expiration time in [ms]
 * - If (expire < 0) subscription never expires
 */
int tipc_topsrv_conn(tipc_domain_t topsrv_node);
int tipc_srv_subscr(int sd, uint32_t type, uint32_t lower, uint32_t upper,
		    bool all, int expire);
int tipc_srv_evt(int sd, struct tipc_addr *srv, bool *up, bool *expired);
bool tipc_srv_wait(const struct tipc_addr *srv, int expire);

int tipc_neigh_subscr(tipc_domain_t topsrv_node);
int tipc_neigh_evt(int sd, tipc_domain_t *neigh_node, bool *up);

int tipc_link_subscr(tipc_domain_t topsrv_node);
int tipc_link_evt(int sd, tipc_domain_t *neigh_node, bool *up,
	          int *local_bearerid, int *remote_bearerid);
char* tipc_linkname(char *buf, size_t len, tipc_domain_t peer, int bearerid);

#endif
