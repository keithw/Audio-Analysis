#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <sys/soundcard.h>

#define OUTPUT_DEVICE "/dev/dsp8"
const int FORMAT = AFMT_S32_LE;
const int CHANNELS = 2;
const int RATE = 44100;

int main( int argc, char *argv[] )
{
  int audio_output, format, channels, rate;
  uint fragbytes, fragsamples;

  /* open devices */
  audio_output = open( OUTPUT_DEVICE, O_WRONLY );
  if ( audio_output == -1 ) {
    perror( "open output" );
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

  /* get fragment size */
  if ( ioctl( audio_output, SNDCTL_DSP_GETBLKSIZE, &fragbytes ) == -1 ) {
    perror( "ioctl SNDCTL_DSP_GETBLKSIZE" );
    exit( 1 );
  }
  
  fragsamples = fragbytes / sizeof( int32_t );

  fprintf( stderr, "Fragment size: %d bytes, or %d samples\n",
	   fragbytes,
	   fragsamples );

  /* allocate buffers */

  int16_t *input_samples = new int16_t[ fragsamples ];
  int32_t *output_samples = new int32_t[ fragsamples ];

  uint samples_played = 0;
  uint samples_acked = 0;

  memset( input_samples, 0, fragsamples * sizeof( int16_t ) );
  memset( output_samples, 0, fragsamples * sizeof( int32_t ) );

  fprintf( stderr, "Synchronizing..." );

  /* play some zeros */
  for ( uint i = 0; i < 3 * (RATE * CHANNELS) / fragsamples; i++ ) {
    size_t bytes_written = write( audio_output,
				  output_samples,
				  fragbytes );
  }

  fprintf( stderr, "done.\n" );

  while ( 1 ) {
    size_t bytes_read = fread( input_samples, sizeof( int16_t ), fragsamples, stdin );

    if ( bytes_read <= 0 ) {
      perror( "read" );
      exit( 1 );
    }

    if ( fragsamples != bytes_read ) {
      perror( "short read" );
    }

    for ( uint i = 0; i < fragsamples; i++ ) {
      output_samples[ i ] = input_samples[ i ] * 256 * 256;
    }

    size_t bytes_written = write( audio_output,
				  output_samples,
				  bytes_read * 4 );

    if ( bytes_written <= 0 ) {
      perror( "write" );
      exit( 1 );
    }

    if ( fragbytes != bytes_written ) {
      perror( "short write" );
      exit( 1 );
    }

    /* indicate speed */
    samples_played += fragsamples;

    while ( samples_played - samples_acked >= (uint) RATE * CHANNELS ) {
      samples_acked += RATE * CHANNELS;
      fprintf( stderr, "Played %.2f seconds.\n", (float) samples_played / (RATE * CHANNELS) );
    }

  }

  if ( close( audio_output ) == -1 ) {
    perror( "close output" );
    exit( 1 );
  }


  return 0;
}
