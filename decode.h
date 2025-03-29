#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types
#include "common.h"
#include "encode.h" 
 
/* Function prototypes for decoding*/

/* Decode magic string */
uchar_ptr decode_magic_string(uint size, EncodeInfo *encInfo);

/* Decode int size expression */
uint decode_int_size_expression(EncodeInfo *encInfo);

/* Read, validate and extract secret file extension */
Status read_and_validate_extn(uchar_ptr sec_file_name_holder, EncodeInfo *encInfo);

/* Decode secret file data */
Status decode_file_data(uint f_size, EncodeInfo *encInfo);

#endif
