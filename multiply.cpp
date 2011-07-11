#include <stdio.h>
#include <stdlib.h>

#include "scmanal.h"

// arguments: factor_one_filename factor_two_filename product_filename

int main( int argc, char **argv )
{
  Analysis *scmanal = new Analysis( 3, 2, 0, "", argc, argv );

  char output_comment[ 256 ];
  snprintf( output_comment, 256, "(%s) * (%s)", scmanal->get_comment( 0 ),
	    scmanal->get_comment( 1 ) );
  scmanal->set_comment( 2, output_comment );

  if ( scmanal->get_length( 0 ) != scmanal->get_length( 1 ) ) {
    fprintf( stderr, "%s: Input file sizes differ.\n", argv[ 0 ] );
    exit( 1 );
  }

  if ( scmanal->get_type( 0 ) != scmanal->get_type( 1 ) ) {
    fprintf( stderr, "%s: Input file types differ.\n", argv[ 0 ] );
    exit( 1 );
  }

  if ( (scmanal->get_begin( 0 ) != scmanal->get_begin( 1 ))
       || (scmanal->get_end( 0 ) != scmanal->get_end( 1 ))
       || (scmanal->get_compl_begin( 0 ) != scmanal->get_compl_begin( 1 ))
       || (scmanal->get_compl_end( 0 ) != scmanal->get_compl_end( 1 ))) {
    fprintf( stderr, "%s: Input file spans differ.\n", argv[ 0 ] );
    exit( 1 );
  }

  scmanal->set_type( 2, scmanal->get_type( 0 ) );
  scmanal->set_begin( 2, scmanal->get_begin( 0 ) );
  scmanal->set_end( 2, scmanal->get_end( 0 ) );
  scmanal->set_compl_begin( 2, scmanal->get_compl_begin( 0 ) );
  scmanal->set_compl_end( 2, scmanal->get_compl_end( 0 ) );

  size_t length = scmanal->get_length( 0 );
  scmanal->make_outputs( length );
  double
    *input_1 = scmanal->get_file( 0 ), 
    *input_2 = scmanal->get_file( 1 ),
    *output = scmanal->get_file( 2 );

  if ( scmanal->get_type( 0 ) == FFT_DOMAIN ) {
    output[ 0 ] = input_1[ 0 ] * input_2[ 0 ];
    
    for ( uint i = 1; i < (length+1) / 2; i++ ) {
      double a_real = input_1[ i ];
      double a_imag = input_1[ length - i ];

      double b_real = input_2[ i ];
      double b_imag = input_2[ length - i ];
    
      output[ i ]          = a_real * b_real - a_imag * b_imag;
      output[ length - i ] = a_imag * b_real + a_real * b_imag;
    }

    if ( length % 2 == 0 ) {
      output[ length / 2 ] = input_1[ length / 2 ] * input_2[ length / 2 ];
    }
  } else {
    for ( uint i = 0; i < length; i++ ) {
      output[ i ] = input_1[ i ] * input_2[ i ];
    }
  }

  delete scmanal;
}
