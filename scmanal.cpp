#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "scmanal.h"

Analysis::Analysis( uint my_num_files, uint my_num_inputs, uint my_num_extra,
		    char *extra_usage, int argc, char **argv )
{
  struct stat my_stat;

  num_files = my_num_files;
  num_inputs = my_num_inputs;
  
  // Check arguments
  if ( (uint)argc != num_files + my_num_extra + 1 ) {
    fprintf( stderr, "usage: %s", argv[ 0 ] );
    for ( uint i = 0; i < num_files; i++ ) {
      if ( i < num_inputs ) {
	fprintf( stderr, " INPUT" );
      } else {
	fprintf( stderr, " OUTPUT" );
      }
    }
    fprintf( stderr, " %s\n", extra_usage );
    exit( 1 );
  }

  calling_program = argv[ 0 ];

  // Allocate memory
  file = new (double *)[ num_files ];
  fd = new int[ num_files ];
  filename = new (char *)[ num_files ];
  length = new size_t[ num_files ];
  metadata = new metadata_struct[ num_files ];

  for ( uint i = 0; i < num_files; i++ ) {
    filename[ i ] = argv[ i + 1 ];
  }

  for ( uint i = num_inputs; i < num_files; i++ ) {
    metadata[ i ].format_version = FORMAT_VERSION;
    metadata[ i ].type = UNKNOWN;
    metadata[ i ].begin = 0;
    metadata[ i ].end = 0;
    sprintf( metadata[ i ].comment,
	     "Calling program \"%s\" failed to write metadata structure.",
	     calling_program );
  }
  
  // Open inputs
  for ( uint i = 0; i < num_inputs; i++ ) {
    // Get input length
    if ( stat( filename[ i ], &my_stat ) < 0 ) {
      fprintf( stderr, "%s: stat(%s) gives error.\n", calling_program, filename[ i ] );
      perror( "stat" );
      exit( 1 );
    }

    if ( ((my_stat.st_size - sizeof( metadata_struct )) % SAMPLE_SIZE) != 0 ) {
      fprintf( stderr, "%s: Length of %s not acceptable (%d bytes).\n",
	       calling_program, filename[ i ], SAMPLE_SIZE );
      exit( 1 );
    }
    
    length[ i ] = (my_stat.st_size - sizeof( metadata_struct )) / SAMPLE_SIZE;
    
    // Open inputs
    if ( ( fd[ i ] = open( filename[ i ], O_RDONLY ) ) < 0 ) {
      fprintf( stderr, "%s: Opening input %s gives error.\n", calling_program, filename[ i ] );
      perror( "open" );
      exit( 1 );
    }

    // Read metadata
    if ( ( lseek( fd[ i ], length[ i ] * SAMPLE_SIZE, SEEK_SET ) )
	 != (off_t)(length[ i ] * SAMPLE_SIZE) ) {
      fprintf( stderr, "%s: lseek(%s) gives error.\n", calling_program, filename[ i ] );
      perror( "lseek" );
      exit( 1 );
    }

    if ( read( fd[ i ], &(metadata[ i ]), sizeof( metadata_struct ) )
	 != sizeof( metadata_struct ) ) {
      fprintf( stderr, "%s: read(%s) gives error.\n", calling_program, filename[ i ] );
      perror( "read" );
      exit( 1 );
    }

    if ( metadata[ i ].format_version != FORMAT_VERSION ) {
      fprintf( stderr, "%s: Format version of %s (%d) is not %d.\n",
	       calling_program, filename[ i ], metadata[ i ].format_version,
	       FORMAT_VERSION );
      exit( 1 );
    }
    
    if ( metadata[ i ].type == TIME_DOMAIN ) {
      if ( fabs( length[ i ] / (metadata[ i ].end - metadata[ i ].begin )
		 - 2 * (metadata[ i ].compl_end - metadata[ i ].compl_begin))
	   > 0.2 ) {
	fprintf( stderr, "%s: Input %s claims frequency range other than Nyquist range.\n",
		 calling_program, filename[ i ] );
	exit( 1 );
      }
    } else if ( metadata[ i ].type == FFT_DOMAIN ) {
      if ( fabs( length[ i ] / (metadata[ i ].compl_end - metadata[ i ].compl_begin )
		 - 2 * (metadata[ i ].end - metadata[ i ].begin))
	   > 0.2 ) {
	fprintf( stderr, "%s: Input %s claims frequency range other than Nyquist range.\n",
		 calling_program, filename[ i ] );
	exit( 1 );
      }      
    } else {
      fprintf( stderr, "%s: Warning: Reading unknown type file (%s).\n",
	       calling_program, filename[ i ] );
    }
    
    // mmap() inputs
    if ( ( file[ i ] = (double *) mmap( NULL, length[ i ] * SAMPLE_SIZE,
					PROT_READ | PROT_WRITE,
					MAP_PRIVATE, fd[ i ], 0 ) ) < 0 ) {
      fprintf( stderr, "%s: mmap(%s) gives error.\n", calling_program, filename[ i ] );
      perror( "mmap" );
      exit( 1 );
    }
  }
}

void Analysis::make_outputs( size_t output_length )
{
  for ( uint i = num_inputs; i < num_files; i++ ) {
    length[ i ] = output_length;

    // Open outputs
    if ( ( fd[ i ] = open( filename[ i ], O_RDWR | O_CREAT | O_TRUNC,
			   S_IRUSR | S_IWUSR ) ) < 0 ) {
      fprintf( stderr, "%s: Opening output %s gives error.\n", calling_program, filename[ i ] );
      perror( "open" );
      exit( 1 );
    }

    // Set size of output
    if ( ( lseek( fd[ i ], length[ i ] * SAMPLE_SIZE, SEEK_SET ) )
	 != (off_t)(length[ i ] * SAMPLE_SIZE) ) {
      fprintf( stderr, "%s: lseek(%s) gives error.\n", calling_program, filename[ i ] );
      perror( "lseek" );
      exit( 1 );
    }

    if ( metadata[ i ].type == TIME_DOMAIN ) {
      if ( fabs( length[ i ] / (metadata[ i ].end - metadata[ i ].begin )
		 - 2 * (metadata[ i ].compl_end - metadata[ i ].compl_begin))
	   > 0.2 ) {
	fprintf( stderr, "%s: Output %s claims frequency range other than Nyquist range.\n",
		 calling_program, filename[ i ] );
	exit( 1 );
      }
    } else if ( metadata[ i ].type == FFT_DOMAIN ) {
      if ( fabs( length[ i ] / (metadata[ i ].compl_end - metadata[ i ].compl_begin )
		 - 2 * (metadata[ i ].end - metadata[ i ].begin))
	   > 0.2 ) {
	fprintf( stderr, "%s: Output %s claims frequency range other than Nyquist range.\n",
		 calling_program, filename[ i ] );
	exit( 1 );
      }      
    } else {
      fprintf( stderr, "%s: Warning: Writing unknown type file (%s).\n",
	       calling_program, filename[ i ] );
    }

    ssize_t write_return = 0;
    do {
      if ( ( write_return = write( fd[ i ], &(metadata[ i ]),
				   sizeof( metadata_struct ) ) ) < 0 ) {
	fprintf( stderr, "%s: write(%s) gives error.\n", calling_program, filename[ i ] );
	perror( "write" );
	exit( 1 );
      }
    } while ( write_return == 0 );
    
    // mmap() outputs
    if ( ( file[ i ] = (double *) mmap( NULL, length[ i ] * SAMPLE_SIZE,
					PROT_READ | PROT_WRITE,
					MAP_SHARED, fd[ i ], 0 ) ) < 0 ) {
      fprintf( stderr, "%s: mmap(%s) gives error.\n", calling_program, filename[ i ] );
      perror( "mmap" );
      exit( 1 );
    }
  }
}

Analysis::~Analysis()
{
  for ( uint i = 0; i < num_files; i++ ) {
    if ( munmap( file[ i ], length[ i ] ) < 0 ) {
      fprintf( stderr, "%s: munmap(%s) gives error.\n", calling_program, filename[ i ] );
      perror( "munmap" );
      exit( 1 );
    }

    if ( close( fd[ i ] ) < 0 ) {
      fprintf( stderr, "%s: close(%s) gives error.\n", calling_program, filename[ i ] );
      perror( "close" );
      exit( 1 );
    }    
  }
  
  delete[] file;
  delete[] fd;
  delete[] filename;
  delete[] length;
  delete[] metadata;
}

