#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
#include <sys/poll.h>
#include <sys/soundcard.h>

int main( void )
{
  return 0;
}

#if 0

#include "scmanal.h"

#define OUTPUT_DEVICE "/dev/dsp8"
#define INPUT_DEVICE "/dev/dsp18"
const int FORMAT = AFMT_S32_LE;
const int CHANNELS = 2;
const int RATE = 96000;
const uint DELAY = 3750;
const int SAMPLE_SHIFT = 218;
typedef int32_t SAMPLE;

// arguments: send receive_left receive_right

int main( int argc, char *argv[] )
{
  int audio_output, audio_input;
  int format, channels, rate;
  uint frag_output, frag_input;
  char *out_buf, *in_buf;
  uint frag_samples;
  SAMPLE *out_samples, *in_samples;
  audio_errinfo errinfo;
  struct pollfd ufds[ 2 ];
  uint send_index = 0, receive_index = 0;
  char write_done = 0, read_done = 0;
  Analysis *scmanal = new Analysis( 3, 1, 0, "", argc, argv );
  char left_comment[ 256 ], right_comment[ 256 ];
  size_t length;
  double *send, *get_left, *get_right;
  uint frags_written = 0, frags_read = 0;

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
  
  length = scmanal->get_length( 0 );
  
  if ( fabs( RATE * (scmanal->get_end( 0 ) - scmanal->get_begin( 0 ))
	     - length )
       > 0.2 ) {
    fprintf( stderr, "%s: Input %s has sampling rate other than 96 kHz.\n",
	     argv[ 0 ], argv[ 1 ] );
    exit( 1 );
  }
  
  scmanal->make_outputs( length );
  send = scmanal->get_file( 0 );
  get_left = scmanal->get_file( 1 );
  get_right = scmanal->get_file( 2 );

  /* open devices */
  audio_output = open( OUTPUT_DEVICE, O_WRONLY | O_NONBLOCK );
  if ( audio_output == -1 ) {
    perror( "open output" );
    exit( 1 );
  }
  audio_input = open( INPUT_DEVICE, O_RDONLY | O_NONBLOCK );
  if ( audio_input == -1 ) {
    perror( "open input" );
    exit( 1 );
  }
  
  /* set formats */
  format = FORMAT;
  if ( ioctl( audio_output, SNDCTL_DSP_SETFMT, &format ) == -1 ) {
    perror( "ioctl SNDCTL_DSP_SETFMT" );
    exit( 1 );
  }
  if ( format != FORMAT ) {
    fprintf( stderr, "%s: Failed to set audio output format.\n", argv[ 0 ] );
    exit( 1 );
  }
  format = FORMAT;
  if ( ioctl( audio_input, SNDCTL_DSP_SETFMT, &format ) == -1 ) {
    perror( "ioctl SNDCTL_DSP_SETFMT" );
    exit( 1 );
  }
  if ( format != FORMAT ) {
    fprintf( stderr, "%s: Failed to set audio input format.\n", argv[ 0 ] );
    exit( 1 );
  }

  /* set channels */
  channels = CHANNELS;
  if ( ioctl( audio_output, SNDCTL_DSP_CHANNELS, &channels ) == -1 ) {
    perror( "ioctl SNDCTL_DSP_CHANNELS" );
    exit( 1 );
  }
  if ( channels != CHANNELS ) {
    fprintf( stderr, "%s: Failed to set audio output channels.\n", argv[ 0 ] );
    exit( 1 );
  }
  channels = CHANNELS;
  if ( ioctl( audio_input, SNDCTL_DSP_CHANNELS, &channels ) == -1 ) {
    perror( "ioctl SNDCTL_DSP_CHANNELS" );
    exit( 1 );
  }
  if ( channels != CHANNELS ) {
    fprintf( stderr, "%s: Failed to set audio input channels.\n", argv[ 0 ] );
    exit( 1 );
  }

  /* set rate */
  rate = RATE;
  if ( ioctl( audio_output, SNDCTL_DSP_SPEED, &rate ) == -1 ) {
    perror( "ioctl SNDCTL_DSP_SPEED" );
    exit( 1 );
  }
  if ( rate != RATE ) {
    fprintf( stderr, "%s: Failed to set rate to %d, got %d.\n",
	     argv[ 0 ], RATE, rate );
    exit( 1 );
  }
  rate = RATE;
  if ( ioctl( audio_input, SNDCTL_DSP_SPEED, &rate ) == -1 ) {
    perror( "ioctl SNDCTL_DSP_SPEED" );
    exit( 1 );
  }
  if ( rate != RATE ) {
    fprintf( stderr, "%s: Failed to set rate to %d, got %d.\n",
	     argv[ 0 ], RATE, rate );
    exit( 1 );
  }

  /* get fragment size */
  if ( ioctl( audio_output, SNDCTL_DSP_GETBLKSIZE, &frag_output ) == -1 ) {
    perror( "ioctl SNDCTL_DSP_GETBLKSIZE" );
    exit( 1 );
  }
  if ( ioctl( audio_input, SNDCTL_DSP_GETBLKSIZE, &frag_input ) == -1 ) {
    perror( "ioctl SNDCTL_DSP_GETBLKSIZE" );
    exit( 1 );
  }
  if ( frag_output != frag_input ) {
    fprintf( stderr, "%s: Fragment sizes differ (%d != %d).\n",
	     argv[ 0 ], frag_output, frag_input );
    exit( 1 );
  }

  /* allocate memory */
  out_buf = (char *) malloc( frag_output );
  in_buf = (char *) malloc( frag_input );
  if ( (out_buf == NULL) || (in_buf == NULL) ) {
    fprintf( stderr, "%s: Could not allocate %d bytes of memory.\n",
	     argv[ 0 ], frag_output + frag_input );
  }
  out_samples = (SAMPLE *) out_buf;
  in_samples = (SAMPLE *) in_buf;

  frag_samples = frag_output / sizeof( SAMPLE );
  if ( frag_samples * sizeof( SAMPLE ) != frag_output ) {
    fprintf( stderr, "%s: Fragment not integral number of samples.\n", argv[ 0 ] );
    exit( 1 );
  }
  if ( frag_samples % 2 ) {
    fprintf( stderr, "%s: Fragment not even number of samples.\n", argv[ 0 ] );
    exit( 1 );
  }

  /* zero error counter */
  if ( ioctl( audio_output, SNDCTL_DSP_GETERROR, &errinfo ) == -1 ) {
    perror( "ioctl SNDCTL_DSP_GETERROR (output)" );
    exit( 1 );
  }
  if ( ioctl( audio_input, SNDCTL_DSP_GETERROR, &errinfo ) == -1 ) {
    perror( "ioctl SNDCTL_DSP_GETERROR (input)" );
    exit( 1 );
  }

  /* set up poll */
  ufds[ 0 ].fd = audio_output;
  ufds[ 0 ].events = POLLOUT;
  ufds[ 1 ].fd = audio_input;
  ufds[ 1 ].events = POLLIN;

  /* start cycle */
  while ( 1 ) {
    uint bytes_written, bytes_read;

    if ( read_done && write_done ) {
      break;
    }

    if ( poll( ufds, 2, -1 ) == -1 ) {
      perror( "poll" );
      exit( 1 );
    }

    if ( ufds[ 0 ].revents & POLLOUT ) {
      /* write */
      if ( frags_written < DELAY ) {
	for ( uint i = 0; i < frag_samples; i++ ) {
	  out_samples[ i ] = i % 2;
	}
      } else {
	for ( uint i = 0; i < frag_samples; i += 2 ) {
	  uint myindex = i/2 + send_index;
	  SAMPLE the_sample;
	  if ( myindex >= length ) {
	    write_done = 1;
	    //	    ufds[ 0 ].events = 0;
	    the_sample = 0;
	  } else {
	    the_sample = int(send[ myindex ] * 256);
	  }
	  out_samples[ i ] = the_sample;
	  out_samples[ i + 1 ] = the_sample;
	}
	send_index += frag_samples / 2;
      }
      bytes_written = write( audio_output, out_buf, frag_output );
      if ( bytes_written != frag_output ) {
	fprintf( stderr, "%s: Wrote only %d bytes.\n",
		 argv[ 0 ], bytes_written );
	perror( "write" );
	exit( 1 );
      }
      frags_written++;
    }

    if ( ufds[ 1 ].revents & POLLIN ) {
      /* read */
      bytes_read = read( audio_input, in_buf, frag_input );
      if ( bytes_read != frag_input ) {
	fprintf( stderr, "%s: Read only %d bytes.\n",
		 argv[ 0 ], bytes_read );
	perror( "read" );
	exit( 1 );
      }
      frags_read++;
      if ( frags_read < DELAY ) {} else {
	for ( uint i = 0; i < frag_samples; i += 2 ) {
	  int myindex = i/2 + receive_index - SAMPLE_SHIFT;
	  if ( myindex >= (int)length ) {
	    read_done = 1;
	    //	    ufds[ 1 ].events = 0;
	    continue;
	  }
	  if ( myindex < 0 ) {} else {
	    double left_sample = double( in_samples[ i ] >> 8 );
	    double right_sample = double( in_samples[ i + 1 ] >> 8 );
	    get_left[ myindex ] = left_sample;
	    get_right[ myindex ] = right_sample;
	  }
	}
	receive_index += frag_samples / 2;
      }
    }

    /* check for output error */
    if ( ioctl( audio_output, SNDCTL_DSP_GETERROR, &errinfo ) == -1 ) {
      perror( "ioctl SNDCTL_DSP_GETERROR (output)" );
    }
    if ( errinfo.play_underruns != 0 ) {
      fprintf( stderr, "%s: Play underrun (%d).\n",
	       argv[ 0 ], errinfo.play_underruns );
      exit( 1 );
    }
    if ( errinfo.rec_overruns != 0 ) {
      fprintf( stderr, "%s: Record overrun (output!) (%d).\n",
	       argv[ 0 ], errinfo.rec_overruns );
      exit( 1 );
    }
    
    /* check for input error */
    if ( ioctl( audio_input, SNDCTL_DSP_GETERROR, &errinfo ) == -1 ) {
      perror( "ioctl SNDCTL_DSP_GETERROR (input)" );
    }
    if ( errinfo.play_underruns != 0 ) {
      fprintf( stderr, "%s: Play underrun (input!) (%d).\n",
	       argv[ 0 ], errinfo.play_underruns );
      exit( 1 );
    }
    if ( errinfo.rec_overruns != 0 ) {
      fprintf( stderr, "%s: Record overrun. (%d)\n",
	       argv[ 0 ], errinfo.rec_overruns );
      exit( 1 );
    }
  }

  if ( close( audio_output ) == -1 ) {
    perror( "close output" );
    exit( 1 );
  }

  if ( close( audio_input ) == -1 ) {
    perror( "close input" );
    exit( 1 );
  }

  delete scmanal;
  return 0;
}

#endif
