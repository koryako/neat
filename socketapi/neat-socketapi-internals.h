/*
 * Socket API implementation for NEAT
 * Copyright (C) 2016-2017 by Thomas Dreibholz <dreibh@simula.no>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of NEAT nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NEAT_SOCKETAPI_INTERNALS_H
#define NEAT_SOCKETAPI_INTERNALS_H


#include <neat-socketapi.h>
#include <neat.h>

#include <pthread.h>

#include "redblacktree.h"
#include "identifierbitmap.h"


struct neat_socketapi_internals
{
   struct identifier_bitmap* identifier_bitmap;
   struct redblacktree       socket_set;
   pthread_mutex_t           socket_set_mutex;
};



#ifdef __cplusplus
extern "C" {
#endif


struct neat_socketapi_internals* nsa_initialize();
struct neat_socketapi_internals* nsa_get();
void nsa_cleanup();


struct neat_socket
{
   struct redblacktree_node node;
   int                      descriptor;

   pthread_mutex_t          mutex;
   int                      socket_domain;
   int                      socket_type;
   int                      socket_protocol;
   int                      socket_sd;
};


void nsa_socket_print_function(const void* node, FILE* fd);
int nsa_socket_comparison_function(const void* node1, const void* node2);

struct neat_socket* nsa_get_socket_for_descriptor(int sd);


#ifdef __cplusplus
}
#endif

#endif
