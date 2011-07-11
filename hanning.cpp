#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "scmanal.h"

// arguments: input_filename output_filename iterations

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 2, 1, 1, "ITERATIONS", argc, argv );
  size_t length = scmanal->get_length( 0 );
  int iterations = atoi( argv[ 3 ] );

  if ( iterations < 0 ) {
    fprintf( stderr, "%s: Cannot do negative Hannings.\n", argv[ 0 ] );
    exit( 1 );
  }

  char output_comment[ 256 ];
  snprintf( output_comment, 256, "Hanning^%d(%s)", iterations, scmanal->get_comment( 0 ) );
  scmanal->set_comment( 1, output_comment );

  if ( scmanal->get_type( 0 ) == FFT_DOMAIN ) {
    fprintf( stderr, "%s: Can't window FFT file (%s).\n", argv[ 0 ],
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

  for ( uint i = 0; i < length; i++ ) {
    output[ i ] = 1;
  }

  for ( int counter = 0; counter < iterations; counter++ ) {
    for ( uint i = 0; i < length; i++ ) {
      output[ i ] *= 1 - cos( (2 * M_PI * i) / length );
    }
  }

  double energy = 0;

  for ( uint i = 0; i < length; i++ ) {
    energy += output[ i ] * output[ i ];
  }

  double scalefactor = sqrt( length / energy );

  for ( uint i = 0; i < length; i++ ) {
    output[ i ] *= input[ i ] * scalefactor;
  }

  delete scmanal;
}
