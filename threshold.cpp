#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scmanal.h"

// arguments: input_filename output_filename threshold

inline double dothresh( double num, double threshold )
{
  if ( fabs( num ) < threshold ) {
    return 0;
  } else {
    return num;
  }
}

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 2, 1, 1, "THRESHOLD", argc, argv );
  size_t length = scmanal->get_length( 0 );
  double threshold = atof( argv[ 3 ] );
  double threshold_squared = threshold * threshold;

  char output_comment[ 256 ];
  snprintf( output_comment, 256, "Threshold[%f](%s)", threshold, scmanal->get_comment( 0 ) );
  scmanal->set_comment( 1, output_comment );

  scmanal->set_type( 1, scmanal->get_type( 0 ) );
  scmanal->set_begin( 1, scmanal->get_begin( 0 ) );
  scmanal->set_end( 1, scmanal->get_end( 0 ) );
  scmanal->set_compl_begin( 1, scmanal->get_compl_begin( 0 ) );
  scmanal->set_compl_end( 1, scmanal->get_compl_end( 0 ) );

  scmanal->make_outputs( length );
  double *input = scmanal->get_file( 0 ), *output = scmanal->get_file( 1 );

  if ( scmanal->get_type( 0 ) == FFT_DOMAIN ) {
    output[ 0 ] = dothresh( input[ 0 ], threshold );
    
    for ( uint i = 1; i < (length+1) / 2; i++ ) {
      double mag = input[ i ] * input[ i ] + input[ length - i ] * input[ length - i ];
      if ( mag < threshold_squared ) {
	output[ i ] = 0;
	output[ length - i ] = 0;
      } else {
	output[ i ] = input[ i ];
	output[ length - i ] = input[ length - i ];
      }
    }

    if ( length % 2 == 0 ) {
      output[ length / 2 ] = dothresh( input[ length / 2 ], threshold );
    }
  } else {
    for ( uint i = 0; i < length; i++ ) {
      output[ i ] = dothresh( input[ i ], threshold );
    }
  }

  delete scmanal;
}
