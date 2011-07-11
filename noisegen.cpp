#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "scmanal.h"

// arguments: output_filename amplitude duration rate

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 1, 0, 3, "AMPLITUDE DURATION RATE", argc, argv );
  double amplitude = atof( argv[ 2 ] );
  double duration = atof( argv[ 3 ] );
  double rate = atof( argv[ 4 ] );

  char output_comment[ 256 ];
  snprintf( output_comment, 256, "(%f) * noise", amplitude );
  scmanal->set_comment( 0, output_comment );

  scmanal->set_type( 0, TIME_DOMAIN );
  scmanal->set_begin( 0, 0 );
  scmanal->set_end( 0, duration );
  scmanal->set_compl_begin( 0, 0 );
  scmanal->set_compl_end( 0, rate / 2 );

  size_t length = int(duration * rate);

  scmanal->make_outputs( length );
  double *output = scmanal->get_file( 0 );

  for ( uint i = 0; i < length; i++ ) {
    output[ i ] = 2*amplitude*((rand()/(RAND_MAX+1.0))-.5);
  }

  delete scmanal;
}
