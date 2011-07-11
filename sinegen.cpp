#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "scmanal.h"

// arguments: output_filename amplitude frequency phase duration rate

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 1, 0, 5, "AMPLITUDE FREQUENCY PHASE DURATION RATE", argc, argv );
  double amplitude = atof( argv[ 2 ] );
  double frequency = atof( argv[ 3 ] );
  double phase = atof( argv[ 4 ] );
  double duration = atof( argv[ 5 ] );
  double rate = atof( argv[ 6 ] );

  char output_comment[ 256 ];
  snprintf( output_comment, 256, "(%f) * sin( 2 pi %f t + %f )", amplitude, frequency, phase );
  scmanal->set_comment( 0, output_comment );

  scmanal->set_type( 0, TIME_DOMAIN );
  scmanal->set_begin( 0, 0 );
  scmanal->set_end( 0, duration );
  scmanal->set_compl_begin( 0, 0 );
  scmanal->set_compl_end( 0, rate / 2 );

  if ( frequency > rate / 2 ) {
    fprintf( stderr, "%s: Can't make sinusoid outside of Nyquist range.\n", argv[ 0 ] );
    exit( 1 );
  }

  size_t length = int(duration * rate);

  scmanal->make_outputs( length );
  double *output = scmanal->get_file( 0 );

  double t = 0;

  for ( uint i = 0; i < length; i++ ) {
    output[ i ] = amplitude * sin( 2 * M_PI * frequency * t + phase );

    t += 1 / rate;
  }

  delete scmanal;
}
