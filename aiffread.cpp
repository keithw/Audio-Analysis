#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sndfile.h>

#include "scmanal.h"

// arguments: output_left_filename output_right_filename input_aiff_filename

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 2, 0, 1, "AIFF_FILENAME", argc, argv );
  char *input_aiff_filename = argv[ 3 ];

  char output_comment[ 256 ];

  snprintf( output_comment, 256, "Left channel from %s", input_aiff_filename );
  scmanal->set_comment( 0, output_comment );

  snprintf( output_comment, 256, "Right channel from %s", input_aiff_filename );
  scmanal->set_comment( 1, output_comment );

  /* Open AIFF input */
  SF_INFO aiff_info;
  aiff_info.format = 0;

  SNDFILE *aiff = sf_open( input_aiff_filename, SFM_READ, &aiff_info );
  if ( aiff == NULL ) {
    fprintf( stderr, "sf_open: %s\n", sf_strerror( NULL ) );
    exit( 1 );
  }
  
  assert( (aiff_info.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_AIFF );
  assert( (aiff_info.format & SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_24 );
  assert( aiff_info.samplerate == 96000 );
  assert( aiff_info.channels == 2 );  

  double duration = (double)aiff_info.frames / (double)aiff_info.samplerate;
  double rate = aiff_info.samplerate;
  size_t length = aiff_info.frames;

  for ( int i = 0; i < 2; i++ ) {
    scmanal->set_type( i, TIME_DOMAIN );
    scmanal->set_begin( i, 0 );
    scmanal->set_end( i, duration );
    scmanal->set_compl_begin( i, 0 );
    scmanal->set_compl_end( i, rate / 2 );
  }

  scmanal->make_outputs( length );

  double *left_output = scmanal->get_file( 0 );
  double *right_output = scmanal->get_file( 1 );

  for ( uint i = 0; i < length; i++ ) {
    int samples[ 2 ];
    assert( sf_readf_int( aiff, samples, 1 ) == 1 );

    assert( (samples[ 0 ] & 0xff) == 0 );
    assert( (samples[ 1 ] & 0xff) == 0 );

    samples[ 0 ] >>= 8;
    samples[ 1 ] >>= 8;

    left_output[ i ] = (double)samples[ 0 ];
    right_output[ i ] = (double)samples[ 1 ];
  }

  delete scmanal;
  assert( sf_close( aiff ) == 0 );
}
