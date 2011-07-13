#ifndef SCMANAL_H
#define SCMANAL_H

#include <sys/types.h>
#include <string.h>

#define MINVAL(x, y) ((x) < (y) ? (x) : (y))

#define SAMPLE_SIZE 8
#define uint unsigned int
#define FORMAT_VERSION 1

enum format_type
{
  TIME_DOMAIN,
  FFT_DOMAIN,
  UNKNOWN
};

typedef struct {
  int format_version;
  format_type type;
  double begin, end;
  double compl_begin, compl_end;
  char comment[256];
} metadata_struct;

class Analysis {
 private:
  double **file;
  int *fd;
  char **filename;
  uint num_files;
  uint num_inputs;
  size_t *length;
  char *calling_program;
  metadata_struct *metadata;

 public:
  Analysis( uint my_num_files, uint my_num_inputs, uint my_num_extra,
	    const char *extra_usage, int argc, char **argv );
  void make_outputs( size_t output_length );
  ~Analysis();

  size_t get_length( uint index ) { return length[ index ]; }
  double *get_file( uint index ) { return file[ index ]; }

  double get_begin( uint index ) { return metadata[ index ].begin; }
  double get_end( uint index ) { return metadata[ index ].end; }
  double get_compl_begin( uint index ) { return metadata[ index ].compl_begin; }
  double get_compl_end( uint index ) { return metadata[ index ].compl_end; }
  format_type get_type( uint index ) { return metadata[ index ].type; }
  const char *get_comment( uint index ) { return metadata[ index ].comment; }

  void set_begin( uint index, double sb ) { metadata[ index ].begin = sb; }
  void set_end( uint index, double se ) { metadata[ index ].end = se; }
  void set_compl_begin( uint index, double sb ) { metadata[ index ].compl_begin = sb; }
  void set_compl_end( uint index, double se ) { metadata[ index ].compl_end = se; }
  void set_type( uint index, format_type st ) { metadata[ index ].type = st; }
  void set_comment( uint index, char *sc ) { strncpy( metadata[ index ].comment, sc, 256 ); }
};

#endif
