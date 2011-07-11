#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "scmanal.h"

// arguments: input_filename output_filename R_ohms C_farads tau_seconds

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 2, 1, 3, "R C TAU", argc, argv );
  double r_ohms = atof( argv[ 3 ] );
  double c_farads = atof( argv[ 4 ] );
  double tau_seconds = atof( argv[ 5 ] );
  size_t length = scmanal->get_length( 0 );

  if ( scmanal->get_type( 0 ) != FFT_DOMAIN ) {
    fprintf( stderr, "%s: Input is not FFT domain.\n", argv[ 0 ] );
    exit( 1 );
  }
  
  char output_comment[ 256 ];
  snprintf( output_comment, 256, "RCDelay(%s, R=%f, C=%f, tau=%f)",
	    scmanal->get_comment( 0 ),
	    r_ohms, c_farads, tau_seconds );
  scmanal->set_comment( 1, output_comment );

  double begin_freq = scmanal->get_begin( 0 );
  double end_freq = scmanal->get_end( 0 );
  
  scmanal->set_type( 1, FFT_DOMAIN );
  scmanal->set_begin( 1, begin_freq );
  scmanal->set_end( 1, end_freq );
  scmanal->set_compl_begin( 1, scmanal->get_compl_begin( 0 ) );
  scmanal->set_compl_end( 1, scmanal->get_compl_end( 0 ) );
  
  scmanal->make_outputs( length );
  double *input = scmanal->get_file( 0 ), *output = scmanal->get_file( 1 );

  /* DC is not delayed */
  output[ 0 ] = input[ 0 ];

  for ( uint i = 1; i < (length+1) / 2; i++ ) {
    double a_real = input[ i ];
    double a_imag = input[ length - i ];

    double frequency = begin_freq + ((end_freq - begin_freq) * 2 * i / length);
    double delay = tau_seconds + ((M_PI / 2) - atan(r_ohms * c_farads * 2 * M_PI * frequency)) / (2 * M_PI * frequency);
    double argument = - 2 * M_PI * delay * frequency;

    double b_real = cos( argument );
    double b_imag = sin( argument );

    output[ i ]          = a_real * b_real - a_imag * b_imag;
    output[ length - i ] = a_imag * b_real + a_real * b_imag;
  }

  if ( length % 2 == 0 ) {
    output[ length / 2 ] = 0;
  }

  delete scmanal;
}
