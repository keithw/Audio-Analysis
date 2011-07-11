#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/poll.h>
#include <errno.h>

#define MINVAL(x, y) ((x) < (y) ? (x) : (y))

#define BLOCK_SIZE 1024
#define BUFF_SIZE  1048576

int buffer_contents( int input_i, int output_i )
{
  int contents = input_i - output_i;
  while ( contents < 0 ) {
    contents += BUFF_SIZE;
  }

  return contents;
}

int main( int argc, char *argv[] )
{
  int input_i = 0, output_i = 0;
  char input_eof = 0, buffer_full = 0, buffer_empty = 1;
  char *buffer;
  struct pollfd ufds[ 2 ];

  buffer = (char *) malloc( BUFF_SIZE );
  if ( buffer == NULL ) {
    fprintf( stderr, "Could not allocate %d bytes of memory\n", BUFF_SIZE );
    return 1;
  }

  memset( buffer, 0, BUFF_SIZE );

  fcntl( STDIN_FILENO,  F_SETFL, O_NONBLOCK );
  fcntl( STDOUT_FILENO, F_SETFL, O_NONBLOCK );

  ufds[0].fd = STDIN_FILENO;
  ufds[1].fd = STDOUT_FILENO;

  while ( 1 ) {
    if ( buffer_contents( input_i, output_i ) == 0 ) {
      buffer_empty = 1;
      //      fprintf( stderr, "Buffer empty!\n" );
    } else {
      buffer_empty = 0;
    }

    if ( buffer_contents( input_i, output_i ) > BUFF_SIZE - 2*BLOCK_SIZE ) {
      buffer_full = 1;
      //      fprintf( stderr, "Buffer full!\n" );
    } else {
      buffer_full = 0;
    }

    ufds[0].events = POLLIN;
    ufds[1].events = buffer_empty ? 0 : POLLOUT;

    if ( buffer_empty && input_eof ) {
      return 0;
    }

    if( poll( ufds, 2, -1 ) == -1 ) {
      perror( "poll" );
    }

    if ( ! buffer_full ) {
      int bytes_read = read( STDIN_FILENO, buffer + input_i, MINVAL( BLOCK_SIZE,
								     BUFF_SIZE - input_i ) );

      if ( bytes_read == 0 ) {
	input_eof = 1;
      }

      if ( bytes_read > 0 ) {
	input_i += bytes_read;
	if ( input_i > BUFF_SIZE ) {
	  fprintf( stderr, "Input overflow.\n" );
	  exit( 1 );
	}
	if ( input_i == BUFF_SIZE ) {
	  input_i = 0;
	}
      }
    }

    if ( (! buffer_empty) ) {
      int bytes_written = write( STDOUT_FILENO,
				 buffer + output_i,
				 MINVAL( MINVAL( BLOCK_SIZE, BUFF_SIZE - output_i ),
					 buffer_contents( input_i, output_i ) ) );
      
      if ( bytes_written == -1 ) {
	if ( errno != EAGAIN ) {
	  perror( "write" );
	  exit( 1 );
	}
      }

      if ( bytes_written > 0 ) {
	output_i += bytes_written;
	if ( output_i > BUFF_SIZE ) {
	  fprintf( stderr, "Output overflow.\n" );
	  exit( 1 );
	}
	if ( output_i == BUFF_SIZE ) {
	  output_i = 0;
	}
      }

    }
  }
}
