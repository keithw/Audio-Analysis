#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scmanal.h"

// arguments: input_filename output_filename

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 2, 1, 0, "", argc, argv );
  size_t length = scmanal->get_length( 0 );

  char output_comment[ 256 ];
  snprintf( output_comment, 256, "Norm(%s)", scmanal->get_comment( 0 ) );
  scmanal->set_comment( 1, output_comment );

  if ( scmanal->get_type( 0 ) == FFT_DOMAIN ) {
    fprintf( stderr, "%s: Cannot normalize FFT file (%s).\n", argv[ 0 ],
	     argv[ 1 ] );
    exit( 1 );
  }

  scmanal->set_type( 1, scmanal->get_type( 0 ) );
  scmanal->set_begin( 1, scmanal->get_begin( 0 ) );
  scmanal->set_end( 1, scmanal->get_end( 0 ) );
  scmanal->set_compl_begin( 1, scmanal->get_compl_begin( 0 ) );
  scmanal->set_compl_end( 1, scmanal->get_compl_end( 0 ) );

  scmanal->make_outputs( length );
  double *input = scmanal->get_file( 0 ), *output = scmanal->get_file( 1 );

  double energy = 0;

  for ( uint i = 0; i < length; i++ ) {
    energy += input[ i ] * input[ i ];
  }

  double scalefactor = sqrt(length / energy);
  fprintf( stderr, "%s: Scaling by %f.\n", argv[ 0 ], scalefactor );

  energy = 0;

  for ( uint i = 0; i < length; i++ ) {
    output[ i ] = input[ i ] * scalefactor;
    energy += output[ i ] * output[ i ];
  }

  fprintf( stderr, "%s: New power: %f.\n", argv[ 0 ], energy / length );

  delete scmanal;
}
