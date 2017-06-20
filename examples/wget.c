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


neat_error_code
on_error(struct neat_flow_operations *opCB)
{
}


static neat_error_code
on_readable(struct neat_flow_operations *opCB)
{
}


static neat_error_code
on_writable(struct neat_flow_operations *opCB)
{
}


static neat_error_code
on_connected(struct neat_flow_operations *opCB)
{
}


static neat_error_code
on_close(struct neat_flow_operations *opCB)
{
}


int
main(int argc, char *argv[])
{
   struct neat_ctx             *the_ctx = NULL;
   struct neat_flow            *the_flow = NULL;
   struct neat_flow_operations the_flow_ops;
   uint32_t                    the_res;


   if ( ( the_ctx = neat_init_ctx() ) == NULL) {
        fprintf( stderr, "Failed to allocate and initialize a NEAT context.\n" );
        res = EXIT_FAILURE;
        goto cleanup;
   }

   neat_log_level( the_ctx, NEAT_LOG_OFF );

   if ( ( the_flow = neat_new_flow( the_ctx ) ) == NULL ) {
        fprintf( stderr, "Failed to allocate and initialize a new NEAT flow.\n" );
        res = EXIT_FAILURE;
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
	 res = EXIT_FAILURE;
	 goto cleanup;
      }

      if (neat_open( the_ctx, the_flow, , 80, NULL, 0) != NEAT_OK) {
	 fprintf( stderr, "Failed to open a NEAT flow.\n" );
         res = EXIT_FAILURE;
	 goto cleanup;
      }


cleanup:
   if ( the_ctx != NULL ) { neat_free_ctx( the_ctx ); }
   return res;
}
