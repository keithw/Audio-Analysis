#include <rfftw.h>
#include <fftw.h>
#include <math.h>

#include "scmanal.h"

// arguments: input_filename output_filename

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 2, 1, 0, "", argc, argv );
  size_t length = scmanal->get_length( 0 );

  if ( scmanal->get_type( 0 ) != TIME_DOMAIN ) {
    fprintf( stderr, "%s: Input is not time domain.\n", argv[ 0 ] );
    exit( 1 );
  }

  char output_comment[ 256 ];
  snprintf( output_comment, 256, "FFT(%s)", scmanal->get_comment( 0 ) );
  scmanal->set_comment( 1, output_comment );

  scmanal->set_type( 1, FFT_DOMAIN );
  scmanal->set_begin( 1, scmanal->get_compl_begin( 0 ) );
  scmanal->set_end( 1, scmanal->get_compl_end( 0 ) );
  scmanal->set_compl_begin( 1, scmanal->get_begin( 0 ) );
  scmanal->set_compl_end( 1, scmanal->get_end( 0 ) );

  scmanal->make_outputs( length );
  double *input = scmanal->get_file( 0 ), *output = scmanal->get_file( 1 );

  rfftw_plan forward_plan;

  // Plan FFT
  if ( ( forward_plan = rfftw_create_plan( length, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE ) )
       == NULL ) {
    fprintf( stderr, "%s: Bad FFT plan.\n", argv[ 0 ] );
    exit( 1 );
  }

  // Perform FFT
  rfftw_one( forward_plan, input, output );
  for ( uint i = 0; i < length; i++ ) {
    output[ i ] /= double( length );
  }

  delete scmanal;
}
