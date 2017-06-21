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
#include <libgen.h>

#define HTTP_PORT 80
#define HTTP_REQ_TEMPLATE "GET %s HTTP/1.0\r\nUser-agent: wget\r\nConnection: close\r\n\r\n" 
#define DNS_NAME_SIZE 500
#define HTTP_REQ_SIZE 600
#define URL_SIZE 500
#define PROGRESS_START 200
#define PROGRESS_REPEAT 100

/* Receive buffer is 1500 bytes. */
#define RCV_BUFFER_SIZE 1500 

typedef struct {
    uint32_t      a_res;
    char          url[ URL_SIZE ];
    unsigned char recv_buffer[ RCV_BUFFER_SIZE ];
} user_data_t;

user_data_t the_user_data;
uv_timer_t  the_progress_timer;
int         the_progress_bar_init = 1;
uint64_t    the_total_byte_cnt    = 0;

static neat_error_code on_close( struct neat_flow_operations *opCB );

    
neat_error_code
on_error( struct neat_flow_operations *opCB )
{
    ( ( user_data_t * ) ( opCB->userData ) )->a_res = EXIT_FAILURE;
    neat_close( opCB->ctx, opCB->flow );
    return NEAT_OK;
}


static neat_error_code
on_readable( struct neat_flow_operations *opCB )
{
    uint32_t        the_byte_cnt = 0;
    neat_error_code the_res;
    struct neat_tlv the_options[ 1 ];
    static int      is_file_start = 1;

    the_res = neat_read( opCB->ctx,
			 opCB->flow,
			 ( ( user_data_t * ) ( opCB->userData ) )->recv_buffer,
			 RCV_BUFFER_SIZE,
			 &the_byte_cnt,
			 the_options,
			 0);
    if ( the_res == NEAT_ERROR_WOULD_BLOCK ) {
        return NEAT_OK;
    } else if ( the_res != NEAT_OK ) {
	fprintf( stderr, "Failed to receive HTTP response\n" );
	( ( user_data_t * ) ( opCB->userData ) )->a_res = EXIT_FAILURE;
	neat_close( opCB->ctx, opCB->flow );	
    }

    if ( the_byte_cnt > 0 ) {
	FILE *fp;
	char *file_attr;

	the_total_byte_cnt += the_byte_cnt;
	
	if ( is_file_start ) {
	    is_file_start = 0;
	    file_attr = "w";
	} else {
	    file_attr = "a";
	}
			      
	if ( ( fp = fopen( basename( ( ( user_data_t * ) ( opCB->userData ) )->url ), file_attr ) ) == NULL ) {
	    fprintf( stderr, "Failed to open file: %s\n", basename( ( ( user_data_t * ) ( opCB->userData ) )->url ) );
	    on_close( opCB );
	    return NEAT_OK;
	}
	if ( fwrite ( ( ( user_data_t * ) ( opCB->userData ) )->recv_buffer,
		      sizeof( char ),
		      the_byte_cnt,
		      fp ) != the_byte_cnt ) {
	    fprintf( stderr, "Failed to write to file: %s\n", basename( ( ( user_data_t * ) ( opCB->userData ) )->url ) );
	    on_close( opCB );
	    fclose( fp );
	    return NEAT_OK;
	}
	fclose( fp );
    } else {
	opCB->on_readable = NULL;
	neat_set_operations( opCB->ctx, opCB->flow, opCB );
	on_close( opCB );
    }

    return NEAT_OK;
}


void
on_progress_timer( uv_timer_t* the_timer, int the_status )
{
    if ( the_progress_bar_init ) {
	printf( "|=" );
	fflush( stdout );
        the_progress_bar_init = 0;
    } else {
	printf( "=" );
	fflush( stdout );
    }
}


static neat_error_code
on_writable( struct neat_flow_operations *opCB )
{
    char the_http_req[ HTTP_REQ_SIZE ];

    sprintf( the_http_req, HTTP_REQ_TEMPLATE, ( ( user_data_t * ) ( opCB->userData ) )->url );
    printf( "HTTP Req = %s\n", the_http_req );
    if ( neat_write( opCB->ctx,
		     opCB->flow,
		     ( const unsigned char * ) the_http_req,
		     strlen( the_http_req ),
		     NULL,
		     0 ) != NEAT_OK ) {
	fprintf( stderr, "Failed to send HTTP request\n" );
	( ( user_data_t * ) ( opCB->userData ) )->a_res = EXIT_FAILURE;
	neat_close( opCB->ctx, opCB->flow );
	return NEAT_OK;
    }

    opCB->on_writable = NULL;
    neat_set_operations( opCB->ctx, opCB->flow, opCB );

    uv_timer_init( neat_get_event_loop( opCB->ctx ), &the_progress_timer );
    uv_timer_start( &the_progress_timer, ( uv_timer_cb ) &on_progress_timer, PROGRESS_START, PROGRESS_REPEAT ); 

    return NEAT_OK;
}


static neat_error_code
on_connected(struct neat_flow_operations *opCB)
{
    opCB->on_writable = on_writable;
    opCB->on_readable = on_readable;
    neat_set_operations( opCB->ctx, opCB->flow, opCB );

    return NEAT_OK;
}


static neat_error_code
on_close( struct neat_flow_operations *opCB )
{
    uv_timer_stop( &the_progress_timer );
    if ( !the_progress_bar_init ) {
	printf( "=|" );
	fflush( stdout );
    }

    printf( "\n\nTransfer complete. Transferred %lu bytes\n", the_total_byte_cnt );
    fflush( stdout );

    opCB->on_error     = NULL;
    opCB->on_connected = NULL;
    opCB->on_readable  = NULL;
    opCB->on_close     = NULL;
    neat_set_operations( opCB->ctx, opCB->flow, opCB );

    neat_stop_event_loop(opCB->ctx);
    
    return NEAT_OK;
}


int
main(int argc, char *argv[])
{
   char                        the_dns_name[ DNS_NAME_SIZE ];
   struct neat_ctx             *the_ctx = NULL;
   struct neat_flow            *the_flow = NULL;
   struct neat_flow_operations the_flow_ops;
   uint32_t                    the_res = EXIT_SUCCESS;

   if ( argc != 2 ) {
       fprintf( stderr, "Usage: wget URL\n" );
       return EXIT_FAILURE;
   }

   if ( sscanf( argv[ 1 ], "http://%[^/]%s", the_dns_name, the_user_data.url ) != 2 ) {
       fprintf( stderr, "Failed to parse URL: %s\n", argv[ 1 ] );
       fprintf( stderr, "Usage: wget URL\n" );
       return EXIT_FAILURE;
   }

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
	  
      if ( neat_open( the_ctx, the_flow, the_dns_name, HTTP_PORT, NULL, 0 ) != NEAT_OK ) {	
	 fprintf( stderr, "Failed to open a NEAT flow.\n" );
         the_res = EXIT_FAILURE;
	 goto cleanup;
      }

      neat_start_event_loop( the_ctx, NEAT_RUN_DEFAULT );
      the_res = the_user_data.a_res;
      
cleanup:
   if ( the_ctx != NULL ) { neat_free_ctx( the_ctx ); }
   return the_res;
}
