/*
 * Authors    : Karl-Johan Grinnemo and Per Hurtig.
 * Date       : 06/20/2017
 * Description: Simple wget client that builds on libNEAT.
 */

#include <neat.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define HTTP_PORT 80
#define HTTP_REQ "GET / HTTP/1.0\r\nUser-agent: wget\r\nConnection: close\r\n\r\n" 
#define DNS_NAME "www.kau.se"

/* Receive buffer is 100 Mbytes. */
#define RCV_BUFFER_SIZE (100 * 1024 * 1024) 

typedef struct {
    uint32_t      a_res;
    unsigned char recv_buffer[ RCV_BUFFER_SIZE ];
} user_data_t;

user_data_t the_user_data;


neat_error_code
on_error(struct neat_flow_operations *opCB)
{
    return NEAT_OK;
}


static neat_error_code
on_readable(struct neat_flow_operations *opCB)
{
    uint32_t        the_byte_cnt = 0;
    neat_error_code the_res;
    struct neat_tlv the_options[ 1 ];

    the_options[0].tag  = NEAT_TAG_TRANSPORT_STACK;
    the_options[0].type = NEAT_TYPE_INTEGER;
    the_res = neat_read( opCB->ctx,
			 opCB->flow,
			 ( ( user_data_t * ) ( opCB->userData ) )->recv_buffer,
			 RCV_BUFFER_SIZE,
			 &the_byte_cnt,
			 the_options,
			 1);
    if ( the_res == NEAT_ERROR_WOULD_BLOCK ) {
        return NEAT_OK;
    } else if ( the_res != NEAT_OK ) {
	fprintf( stderr, "Failed to receive HTTP response\n" );
	( ( user_data_t * ) ( opCB->userData ) )->a_res = EXIT_FAILURE;
	neat_close( opCB->ctx, opCB->flow );	
    }

    if ( the_byte_cnt != 0 ) fprintf( stderr, "Bytes read: %u\n", the_byte_cnt );
    
    return NEAT_OK;
}


static neat_error_code
on_writable(struct neat_flow_operations *opCB)
{
    if ( neat_write(opCB->ctx,
		    opCB->flow,
		    ( const unsigned char * ) HTTP_REQ,
		    strlen( HTTP_REQ ),
		    NULL,
		    0 ) != NEAT_OK ) {
	fprintf( stderr, "Failed to send HTTP request\n" );
	( ( user_data_t * ) ( opCB->userData ) )->a_res = EXIT_FAILURE;
	neat_close( opCB->ctx, opCB->flow );
	return NEAT_OK;
    }

    opCB->on_writable = NULL;
    neat_set_operations(opCB->ctx, opCB->flow, opCB);

    return NEAT_OK;
}


static neat_error_code
on_connected(struct neat_flow_operations *opCB)
{
    opCB->on_writable = on_writable;
    opCB->on_readable = on_readable;
    neat_set_operations( opCB->ctx, opCB->flow, opCB);

    return NEAT_OK;
}


static neat_error_code
on_close(struct neat_flow_operations *opCB)
{
    return NEAT_OK;
}


int
main(int argc, char *argv[])
{
   struct neat_ctx             *the_ctx = NULL;
   struct neat_flow            *the_flow = NULL;
   struct neat_flow_operations the_flow_ops;
   uint32_t                    the_res = EXIT_SUCCESS;


   if ( ( the_ctx = neat_init_ctx() ) == NULL) {
        fprintf( stderr, "Failed to allocate and initialize a NEAT context.\n" );
        the_res = EXIT_FAILURE;
        goto cleanup;
   }

   neat_log_level( the_ctx, NEAT_LOG_OFF );

   if ( ( the_flow = neat_new_flow( the_ctx ) ) == NULL ) {
        fprintf( stderr, "Failed to allocate and initialize a new NEAT flow.\n" );
        the_res = EXIT_FAILURE;
        goto cleanup;
   }

   char *the_flow_properties = "\
      {\
         \"transport\": [\
            {\
               \"value\": \"SCTP\",\
               \"precedence\": 1\
            },\
            {\
               \"value\": \"TCP\",\
               \"precedence\": 1\
            }\
         ],\
         \"multihoming\": {\
            \"value\": true,\
            \"precedence\": 1\
         }\
      }";
      if ( neat_set_property( the_ctx, the_flow, the_flow_properties ) ) {
	 fprintf( stderr, "Failed to set the NEAT flow properties.\n" );
	 the_res = EXIT_FAILURE;
	 goto cleanup;
      }

      memset( &the_flow_ops, 0, sizeof( the_flow_ops ) );
      the_flow_ops.on_error     = on_error;
      the_flow_ops.on_connected = on_connected;
      the_flow_ops.on_close     = on_close;
      the_flow_ops.userData     = &the_user_data;
      neat_set_operations( the_ctx, the_flow, &the_flow_ops );
	  
      if ( neat_open( the_ctx, the_flow, DNS_NAME, HTTP_PORT, NULL, 0 ) != NEAT_OK ) {	
	 fprintf( stderr, "Failed to open a NEAT flow.\n" );
         the_res = EXIT_FAILURE;
	 goto cleanup;
      }

      neat_start_event_loop( the_ctx, NEAT_RUN_DEFAULT );
      the_res = the_user_data.a_res;
      
cleanup:
   if ( the_ctx != NULL ) { neat_free_ctx( the_ctx ); }
   if ( the_flow_ops.userData != NULL ) { free( the_flow_ops.userData ); }
   return the_res;
}
