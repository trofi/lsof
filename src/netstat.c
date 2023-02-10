/*
 * netstat.c - main function for lsof-netstat
 *
 * V. Abell, Purdue University
 */

/*
 * Copyright 2002 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by V. Abell.
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

#include "lsof.h"
#include <stdio.h>
#include <arpa/inet.h>

void print_sockaddr(struct sockaddr_storage sockaddr, char *output) {
    char buffer[128];
    if (sockaddr.ss_family == AF_INET) {
        struct sockaddr_in *in = ((struct sockaddr_in *)&sockaddr);
        inet_ntop(sockaddr.ss_family, &in->sin_addr, buffer, sizeof(buffer));
        sprintf(output, "%s:%d", buffer, ntohs(in->sin_port));
    } else if (sockaddr.ss_family == AF_INET6) {
        struct sockaddr_in6 *in6 = ((struct sockaddr_in6 *)&sockaddr);
        inet_ntop(sockaddr.ss_family, &in6->sin6_addr, buffer, sizeof(buffer));
        sprintf(output, "%s:%d", buffer, ntohs(in6->sin6_port));
    } else {
        sprintf(output, "");
    }
}

int main(int argc, char **argv) {
    struct lsof_result *result = NULL;
    struct lsof_context *ctx = NULL;
    struct lsof_process *p;
    struct lsof_file *f;
    int res = 0;
    int pi, fi;
    int print_pass = 0;
    char buffer[128];
    int len;

    /* columns */
    char *proto_header = "Proto";
    int proto_width = strlen(proto_header);
    char *proto;
    char *local_addr_header = "Local Address";
    int local_addr_width = strlen(local_addr_header);
    char local_addr[128];
    char *foreign_addr_header = "Foreign Address";
    int foreign_addr_width = strlen(foreign_addr_header);
    char foreign_addr[128];
    char *state_header = "State";
    int state_width = strlen(state_header);
    char state[128];
    char *pid_program_header = "PID/Program name";
    char pid_program[128];
    int match;

    ctx = lsof_new();
    lsof_select_inet(ctx, 0);
    lsof_freeze(ctx);
    lsof_gather(ctx, &result);

    for (print_pass = 0; print_pass < 2; print_pass++) {
        for (pi = 0; pi < result->num_processes; pi++) {
            p = &result->processes[pi];
            for (fi = 0; fi < p->num_files; fi++) {
                f = &p->files[fi];
                match = 0;
                if (f->protocol == LSOF_PROTOCOL_TCP &&
                    f->file_type == LSOF_FILE_IPV4) {
                    match = 1;
                    proto = "tcp";
                } else if (f->protocol == LSOF_PROTOCOL_TCP &&
                           f->file_type == LSOF_FILE_IPV6) {
                    match = 1;
                    proto = "tcp6";
                } else if (f->protocol == LSOF_PROTOCOL_UDP &&
                           f->file_type == LSOF_FILE_IPV4) {
                    match = 1;
                    proto = "udp";
                } else if (f->protocol == LSOF_PROTOCOL_UDP &&
                           f->file_type == LSOF_FILE_IPV6) {
                    match = 1;
                    proto = "udp6";
                }

                if (match) {
                    /* local address */
                    print_sockaddr(f->net_local, local_addr);
                    print_sockaddr(f->net_foreign, foreign_addr);
                    sprintf(pid_program, "%d/%s", p->pid, p->command);
                    if (print_pass == 0) {
                        len = strlen(local_addr);
                        if (len > local_addr_width)
                            local_addr_width = len;

                        len = strlen(foreign_addr);
                        if (len > foreign_addr_width)
                            foreign_addr_width = len;
                    } else {
                        printf("%-*s %-*s %-*s %s\n", proto_width, proto,
                               local_addr_width, local_addr, foreign_addr_width,
                               foreign_addr, pid_program);
                    }
                }
            }
        }

        if (print_pass == 0) {
            printf("%*s %-*s %-*s %s\n", proto_width, proto_header,
                   local_addr_width, local_addr_header, foreign_addr_width,
                   foreign_addr_header, pid_program_header);
        }
    }

    if (result) {
        lsof_free_result(result);
    }
    if (ctx) {
        lsof_destroy(ctx);
    }
    return res;
}