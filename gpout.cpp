#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scmanal.h"

// arguments: input_filename output_filename

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 1, 1, 2, "MIN MAX", argc, argv );
  size_t length = scmanal->get_length( 0 );
  double min = atof( argv[ 2 ] );
  double max = atof( argv[ 3 ] );
  double begin = scmanal->get_begin( 0 );
  double end = scmanal->get_end( 0 );
  double *input = scmanal->get_file( 0 );

  if ( scmanal->get_type( 0 ) == FFT_DOMAIN ) {
    uint start_index = (uint) ((min - begin) * (length / 2) / (end - begin));
    uint end_index = (uint) ((max - begin) * (length / 2) / (end - begin));

    if ( start_index < 0 ) {
      start_index = 0;
    }

    if ( end_index > length / 2 ) {
      end_index = length / 2;
    }

    for ( uint i = start_index; i < end_index; i++ ) {
      if ( i == 0 ) {
	printf( "%e %e %e\n", begin + ((end - begin) * 2 * i / length),
		input[ i ], 0.0 );
      } else {
	if ( isnan( input[ i ] ) || isnan( input[ length - i ] ) ) {} else {
	  printf( "%e %e %e\n", begin + ((end - begin) * 2 * i / length),
		  input[ i ], input[ length - i ] );
	}
      }
    }
  } else {
    uint start_index = (uint) ((min - begin) * length / (end - begin));
    uint end_index = (uint) ((max - begin) * length / (end - begin));

    if ( start_index < 0 ) {
      start_index = 0;
    }

    if ( end_index > length ) {
      end_index = length;
    }

    for ( uint i = start_index; i < end_index; i++ ) {
      if ( !isnan( input[ i ] ) ) {
	printf( "%e %e\n", begin + ((end - begin) * i / length), input[ i ] );
      }
    }
  }

  delete scmanal;
}
