#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sndfile.h>

#include "scmanal.h"

// arguments: input_filename output_aiff_filename

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 1, 1, 1, "OUTPUT", argc, argv );
  char *output_aiff_filename = argv[ 2 ];

  assert( scmanal->get_type( 0 ) == TIME_DOMAIN );

  /* Open AIFF input */
  SF_INFO aiff_info;
  aiff_info.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_24;
  aiff_info.frames = scmanal->get_length( 0 );
  aiff_info.samplerate = 2 * (scmanal->get_compl_end( 0 ) - scmanal->get_compl_begin( 0 ));
  aiff_info.channels = 2;
  aiff_info.sections = 0;
  aiff_info.seekable = 0;

  SNDFILE *aiff = sf_open( output_aiff_filename, SFM_WRITE, &aiff_info );
  if ( aiff == NULL ) {
    fprintf( stderr, "sf_open: %s\n", sf_strerror( NULL ) );
    exit( 1 );
  }
  
  double *file_samples = scmanal->get_file( 0 );

  for ( uint i = 0; i < aiff_info.frames; i++ ) {
    int samples[ 2 ];

    samples[ 0 ] = samples[ 1 ] = file_samples[ i ];

    assert( sf_writef_int( aiff, samples, 1 ) == 1 );
  }

  delete scmanal;
  assert( sf_close( aiff ) == 0 );
}
