#include <ctype.h>
#include <stdio.h>
#include "scmanal.h"

// arguments: input_filename output_filename

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 1, 0, 0, "", argc, argv );

  /* Read metadata */

  size_t length;
  format_type type;
  double begin, end;
  double compl_begin, compl_end;
  char comment[ 256 ];

  memset( comment, 0, 256 );

  if ( scanf( "# %d %d %le %le %le %le ", (int *)&type, &length, &begin, &end,
	      &compl_begin, &compl_end ) != 6 ) {
    fprintf( stderr, "%s: scanf read wrong number of metadata items\n",
	     argv[ 0 ] );
    perror( "scanf" );
    exit( 1 );
  }

  fgets( comment, 256, stdin );
  if ( comment[ strlen( comment ) - 1 ] == '\n' ) {
    comment[ strlen( comment ) - 1 ] = 0;
  }

  while( 1 ) {
    char c = (char) getc( stdin );

    if ( c == '\n' ) {
      break;
    } else if ( isdigit( c ) || (c == '-') ) {
      ungetc( c, stdin );
      break;
    }
  }

  scmanal->set_type( 0, type );
  scmanal->set_begin( 0, begin );
  scmanal->set_end( 0, end );
  scmanal->set_compl_begin( 0, compl_begin );
  scmanal->set_compl_end( 0, compl_end );
  scmanal->set_comment( 0, comment );

  scmanal->make_outputs( length );
  double *output = scmanal->get_file( 0 );

  /* Read in samples */
  for( uint i = 0; i < length; i++ ) {
    double data_sample;
    int scanf_return = scanf( "%le", &data_sample );

    if ( scanf_return == 0 ) {
      perror( "scanf" );
      exit( 1 );
    }

    if ( scanf_return == EOF ) {
      fprintf( stderr, "%s: Input ended prematurely (at %d)\n", argv[ 0 ], i );
      exit( 1 );
    }

    output[ i ] = data_sample;
  }

  delete scmanal;
}
