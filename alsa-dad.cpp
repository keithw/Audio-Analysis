#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <math.h>

#define RATE 96000
#define DEVICE "hw"
#define WAIT 3

#include "scmanal.h"

// arguments: send receive_left receive_right

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 3, 1, 0, "", argc, argv );

  char left_comment[ 256 ], right_comment[ 256 ];
  snprintf( left_comment, 256, "Ldad(%s)", scmanal->get_comment( 0 ) );
  snprintf( right_comment, 256, "Rdad(%s)", scmanal->get_comment( 0 ) );
  scmanal->set_comment( 1, left_comment );
  scmanal->set_comment( 2, right_comment );

  scmanal->set_type( 1, TIME_DOMAIN );
  scmanal->set_type( 2, TIME_DOMAIN );

  scmanal->set_begin( 1, scmanal->get_begin( 0 ) );
  scmanal->set_end( 1, scmanal->get_end( 0 ) );
  scmanal->set_compl_begin( 1, scmanal->get_compl_begin( 0 ) );
  scmanal->set_compl_end( 1, scmanal->get_compl_end( 0 ) );

  scmanal->set_begin( 2, scmanal->get_begin( 0 ) );
  scmanal->set_end( 2, scmanal->get_end( 0 ) );
  scmanal->set_compl_begin( 2, scmanal->get_compl_begin( 0 ) );
  scmanal->set_compl_end( 2, scmanal->get_compl_end( 0 ) );

  size_t length = scmanal->get_length( 0 );

  if ( fabs( RATE * (scmanal->get_end( 0 ) - scmanal->get_begin( 0 ))
	     - length )
       > 0.2 ) {
    fprintf( stderr, "%s: Input %s has sampling rate other than 96 kHz.\n",
	     argv[ 0 ], argv[ 1 ] );
    exit( 1 );
  }

  scmanal->make_outputs( length );
  double
    *send = scmanal->get_file( 0 ), 
    *get_left = scmanal->get_file( 1 ),
    *get_right = scmanal->get_file( 2 );
  
  /* alsa-siner */

  int err;
  long buf[4096*10*4];
  long in_buf[4096*12*4];
  snd_pcm_t *playback_handle;
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;

  if ((err = snd_pcm_open (&playback_handle, DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
	     DEVICE,
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_open (&capture_handle, DEVICE, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
	     DEVICE,
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S32_LE)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, RATE, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  
  if ((err = snd_pcm_hw_params_set_buffer_size (playback_handle, hw_params, 4096)) < 0) {
    fprintf (stderr, "cannot set playback buffer_size (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_periods (playback_handle, hw_params, 4, 0)) < 0) {
    fprintf (stderr, "cannot set playback periods (%s)\n",
	     snd_strerror (err));
    exit (1);
  }  

  if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (playback %s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S32_LE)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, RATE, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 12)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_buffer_size (capture_handle, hw_params, 4096)) < 0) {
    fprintf (stderr, "cannot set capture buffer_size (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_periods (capture_handle, hw_params, 4, 0)) < 0) {
    fprintf (stderr, "cannot set capture periods (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (capture %s)\n",
	     snd_strerror (err));
    exit (1);
  }

  snd_pcm_hw_params_free (hw_params);

  snd_pcm_nonblock( playback_handle, 0 );
  snd_pcm_nonblock( capture_handle, 0 );

  if (( err = snd_pcm_link( capture_handle, playback_handle ) )) {
    fprintf( stderr, "Cannot link handles (%s)\n", snd_strerror( err ) );
    exit( 1 );
  }

  if ((err = snd_pcm_prepare (playback_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_prepare (capture_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  memset( buf, 0, 16384*4 );

  int pb_period = 1024;

  if ( fork() ) {
    unsigned int count = 0;
    unsigned int sample = 0;
    while( 1 ) {
      for ( int i = 0; i < pb_period*10; i++ ) {
	if ( count < RATE * WAIT ) {
	  buf [ i ] = 0;
	} else {
	  if ( sample >= length ) {
	    buf[ i ] = 0;
	  } else {
	    buf[ i ] = (long)(send[ sample ] * 256);
	  }
	  if ( i % 10 == 0 ) {
	    sample++;
	  }
	}
      }
      
      if ((err = snd_pcm_writei (playback_handle, buf, pb_period)) != pb_period ) {
	if ( err == -5 ) {
	  fprintf( stderr, "Play exiting: %d\n", count );
	  snd_pcm_close( playback_handle );
	  delete scmanal;
	  exit( 0 );
	}
	fprintf( stderr, "w(%d)", err );
	if ((err = snd_pcm_prepare (playback_handle)) < 0) {
	  fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
		   snd_strerror (err));
	  exit (1);
	}
      }
      
      count += pb_period;
    }
    fprintf( stderr, "Play exiting: %d\n", count );
    snd_pcm_close( playback_handle );
    delete scmanal;
    exit( 0 );
  } else {
    unsigned int count = 0;
    unsigned int sample = 0;
    while( count < length + RATE * WAIT ) {
      if ((err = snd_pcm_readi (capture_handle, in_buf, pb_period)) != pb_period ) {
	fprintf( stderr, "r(%d)", err );
	if ((err = snd_pcm_prepare (capture_handle)) < 0) {
	  fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
		   snd_strerror (err));
	  exit (1);
	}
      }

      for ( int i = 0; i < pb_period*12; i++ ) {
	if ( count < RATE * WAIT ) {

	} else {
	  if ( sample < length ) {
	    if ( i % 12 == 8 ) {
	      get_left[ sample ] = (in_buf[ i ] >> 8);
	    }
	    if ( i % 12 == 9 ) {
	      get_right[ sample ] = (in_buf[ i ] >> 8);
	    }
	  }
	  if ( i % 12 == 0 ) {
	    sample++;
	  }
	}
      }
      count += pb_period;
    }
    fprintf( stderr, "Capture exiting: %d\n", count );
    snd_pcm_close( capture_handle );
    delete scmanal;
    exit( 0 );
  }
}
