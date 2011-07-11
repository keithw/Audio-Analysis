#include <rfftw.h>
#include <fftw.h>
#include <math.h>

#include "scmanal.h"

// arguments: input_filename output_filename slicesize hannings

void spectro( );

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 2, 1, 2, "SLICESIZE HANNINGS", argc, argv );
  int length = scmanal->get_length( 0 );
  int slicesize = atoi( argv[ 3 ] );
  int hannings = atoi( argv[ 4 ] );

  if ( slicesize < 0 ) {
    fprintf( stderr, "%s: Cannot do negative slicesize.\n", argv[ 0 ] );
    exit( 1 );
  }

  if ( hannings < 0 ) {
    fprintf( stderr, "%s: Cannot do negative Hannings.\n", argv[ 0 ] );
    exit( 1 );
  }

  if ( scmanal->get_type( 0 ) != TIME_DOMAIN ) {
    fprintf( stderr, "%s: Input is not time domain.\n", argv[ 0 ] );
    exit( 1 );
  }

  char output_comment[ 256 ];
  snprintf( output_comment, 256, "Spectrum(%s)", scmanal->get_comment( 0 ) );
  scmanal->set_comment( 1, output_comment );

  double rate = length / ( scmanal->get_end( 0 ) - scmanal->get_begin( 0 ) );

  scmanal->set_type( 1, FFT_DOMAIN );
  scmanal->set_begin( 1, scmanal->get_compl_begin( 0 ) );
  scmanal->set_end( 1, scmanal->get_compl_end( 0 ) );
  scmanal->set_compl_begin( 1, scmanal->get_begin( 0 ) );
  scmanal->set_compl_end( 1, scmanal->get_begin( 0 ) + slicesize / rate );

  scmanal->make_outputs( slicesize );
  double *input = scmanal->get_file( 0 ), *output = scmanal->get_file( 1 );
  double *window = new double[ slicesize ];
  double *fft_out = new double[ slicesize ];

  /* Make window */

  uint slices = 0;

  for ( int i = 0; i < slicesize; i++ ) {
    window[ i ] = 1;
  }

  for ( int counter = 0; counter < hannings; counter++ ) {
    for ( int i = 0; i < slicesize; i++ ) {
      window[ i ] *= 1 - cos( (2 * M_PI * i) / slicesize );
    }
  }

  double energy = 0;

  for ( int i = 0; i < slicesize; i++ ) {
    energy += window[ i ] * window[ i ];
  }

  double scalefactor = sqrt( slicesize / energy );

  for ( int i = 0; i < slicesize; i++ ) {
    window[ i ] *= scalefactor;
  }

  // Plan FFT
  rfftw_plan forward_plan;

  if ( ( forward_plan = rfftw_create_plan( slicesize, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE ) )
       == NULL ) {
    fprintf( stderr, "%s: Bad FFT plan.\n", argv[ 0 ] );
    exit( 1 );
  }

  for ( int index = 0; index + slicesize <= length; index += slicesize ) {
    // Window
    for ( int i = 0; i < slicesize; i++ ) {
      input[ index + i ] *= window[ i ];
    }

    // Perform FFT
    rfftw_one( forward_plan, input + index, fft_out );
    for ( int i = 0; i < slicesize; i++ ) {
      fft_out[ i ] /= double( slicesize );
    }
    
    output[ 0 ] += fft_out[ 0 ] * fft_out[ 0 ];

    for ( int i = 1; i < (slicesize + 1) / 2; i++ ) {
      output[ i ] += fft_out[ i ] * fft_out[ i ]
	+ fft_out[ slicesize - i ] * fft_out[ slicesize - i ];
    }

    if ( slicesize % 2 == 0 ) {
      output[ slicesize / 2 ] += fft_out[ slicesize / 2 ]
	* fft_out[ slicesize / 2 ];
    }

    slices++;
  }

  for ( int i = 0; i < slicesize; i++ ) {
    output[ i ] /= (double) slices;
    output[ i ] = sqrt( output[ i ] );
  }

  fprintf( stderr, "Done, slices = %d.\n", slices );

  delete window;
  delete fft_out;
  delete scmanal;
}
