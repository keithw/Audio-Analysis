#include <stdio.h>
#include <stdlib.h>

#include "scmanal.h"

// arguments: addend_one_filename addend_two_filename sum_filename

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 3, 2, 0, "", argc, argv );

  char output_comment[ 256 ];
  snprintf( output_comment, 256, "(%s) + (%s)", scmanal->get_comment( 0 ),
	    scmanal->get_comment( 1 ) );
  scmanal->set_comment( 2, output_comment );

  if ( scmanal->get_length( 0 ) != scmanal->get_length( 1 ) ) {
    fprintf( stderr, "%s: Input file sizes differ.\n", argv[ 0 ] );
    exit( 1 );
  }

  if ( scmanal->get_type( 0 ) != scmanal->get_type( 1 ) ) {
    fprintf( stderr, "%s: Input file types differ.\n", argv[ 0 ] );
    exit( 1 );
  }

  if ( (scmanal->get_begin( 0 ) != scmanal->get_begin( 1 ))
       || (scmanal->get_end( 0 ) != scmanal->get_end( 1 ))
       || (scmanal->get_compl_begin( 0 ) != scmanal->get_compl_begin( 1 ))
       || (scmanal->get_compl_end( 0 ) != scmanal->get_compl_end( 1 ))) {
    fprintf( stderr, "%s: Input file spans differ.\n", argv[ 0 ] );
    exit( 1 );
  }

  scmanal->set_type( 2, scmanal->get_type( 0 ) );
  scmanal->set_begin( 2, scmanal->get_begin( 0 ) );
  scmanal->set_end( 2, scmanal->get_end( 0 ) );
  scmanal->set_compl_begin( 2, scmanal->get_compl_begin( 0 ) );
  scmanal->set_compl_end( 2, scmanal->get_compl_end( 0 ) );

  size_t length = scmanal->get_length( 0 );
  scmanal->make_outputs( length );
  double
    *input_1 = scmanal->get_file( 0 ), 
    *input_2 = scmanal->get_file( 1 ),
    *output = scmanal->get_file( 2 );

  for ( uint i = 0; i < length; i++ ) {
    output[ i ] = input_1[ i ] + input_2[ i ];
  }

  delete scmanal;
}
