#include <stdio.h>
#include <stdlib.h>
#include "scmanal.h"

// arguments: input_filename output_filename scalefactor

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 2, 1, 1, "SCALEFACTOR", argc, argv );
  size_t length = scmanal->get_length( 0 );
  double scalefactor = atof( argv[ 3 ] );

  char output_comment[ 256 ];
  snprintf( output_comment, 256, "(%f) * (%s)", scalefactor, scmanal->get_comment( 0 ) );
  scmanal->set_comment( 1, output_comment );

  scmanal->set_type( 1, scmanal->get_type( 0 ) );
  scmanal->set_begin( 1, scmanal->get_begin( 0 ) );
  scmanal->set_end( 1, scmanal->get_end( 0 ) );
  scmanal->set_compl_begin( 1, scmanal->get_compl_begin( 0 ) );
  scmanal->set_compl_end( 1, scmanal->get_compl_end( 0 ) );

  scmanal->make_outputs( length );
  double *input = scmanal->get_file( 0 ), *output = scmanal->get_file( 1 );

  for ( uint i = 0; i < length; i++ ) {
    output[ i ] = input[ i ] * scalefactor;
  }

  delete scmanal;
}
