#include <stdio.h>
#include "scmanal.h"

// arguments: input_filename output_filename

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 1, 1, 0, "", argc, argv );
  size_t length = scmanal->get_length( 0 );
  double *input = scmanal->get_file( 0 );

  format_type type = scmanal->get_type( 0 );
  double begin = scmanal->get_begin( 0 );
  double end = scmanal->get_end( 0 );
  double compl_begin = scmanal->get_compl_begin( 0 );
  double compl_end = scmanal->get_compl_end( 0 );
  const char *comment = scmanal->get_comment( 0 );

  printf( "# %d %ld %e %e %e %e %s\n", type, (long)length, begin, end,
	  compl_begin, compl_end, comment );

  for ( uint i = 0; i < length; i++ ) {
    printf( "%e\n", input[ i ] );
  }

  delete scmanal;
}

