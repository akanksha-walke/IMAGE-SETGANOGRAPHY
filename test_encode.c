#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include <libgen.h>

/* Global variables. These will be externed into other files */
//Store raster data index, file index, length of seret filename
uint raster_data, secret_filename_len, default_ext_name = 0,flag = 1;
//string for storing magic string : * or # with '\0' character
uchar magic_string_signature[CHAR_SIZE + CHAR_SIZE];
//string storing temporary user provided decode filename
uchar temp_decode_name[MAX_FILENAME_SIZE];

int main(int argc, char* argv[]) 
{
		EncodeInfo encInfo;//Structure variable
        if (argc < 3) 
        {
				printf("ERROR: Invalid number of command line arguments.\n\n");
				exit(e_success);
		}
		if (check_operation_type(argv + 1) == e_encode){//if encoding is selected
				printf(" Encoding is selected.\n");
				if (argc < 4 || argc > 5) {
						printf("ERROR: Invalid number of command line arguments.\n");
						exit(e_success);
				}
				//Read and validate src_image filename
				//Extract only filename from given path if any
				argv[2] = basename(argv[2]);
				if (read_and_validate_bmp_format(argv + 2) == e_failure) {
						printf("ERROR: Invalid filename provided in 2nd command line argument. It must be a '.bmp' file\n\n");
						exit(e_success);
				}
				strcpy((char*)encInfo.src_image_fname, argv[2]);
				//Validate and assign 3rd command line argument as secret filename
				//Extract only filename from given path if any
				argv[3] = basename(argv[3]);
				if (read_and_validate_extn((uchar_ptr)argv[3], &encInfo))
				{
                    printf("\n");
                }
				else
				{
						printf("ERROR: Invalid secret filename.\n\n");
						exit(e_success);
				}
				strcpy((char*)encInfo.secret_fname, argv[3]);
				switch (argc) {
						case 4:
								//Giving default output filename since no 4th command line argument is given
								strcpy((char*)encInfo.stego_image_fname, "stego_img.bmp");
								break;
						case 5:
								//Read and validate given output filename
								//Extract only filename from given path if any
								argv[4] = basename(argv[4]);
								if (read_and_validate_bmp_format(argv + 4) == e_failure) {
										printf("ERROR: Invalid filename provided in 4th command line argument. It must be user provided '.bmp' file.\n\n");
										exit(e_success);
								}
								strcpy((char*)encInfo.stego_image_fname, argv[4]);
								break;
						default:
								//Read and validate given output filename
								//Extract only filename from given path if any
								argv[4] = basename(argv[4]);
								if (read_and_validate_bmp_format(argv + 4) == e_failure) {
										printf("ERROR: Invalid filename provided in 4th command line argument. It must be user provided output '.bmp' file.\n\n");
										exit(e_success);
								}
								strcpy((char*)encInfo.stego_image_fname, argv[4]);
				}
				// Test open_files
				if (open_files(&encInfo) == e_success) {
						printf("All files successfully opened.\n");
				} else
						exit(e_success);
				//Collect raster data offset: seek to 10th index of bmp file
				fseek(encInfo.fptr_src_image, 10L, SEEK_SET);
				fread(&raster_data, sizeof(raster_data), 1, encInfo.fptr_src_image);
				if (ferror(encInfo.fptr_src_image)) {//Error handling
						printf("ERROR: Error while reading from file %s\n\n", encInfo.src_image_fname);
						exit(e_success);
				}
				rewind(encInfo.fptr_src_image);
				//Copy image header
				if (copy_bmp_header((FILE*) encInfo.fptr_src_image,(FILE*) encInfo.fptr_stego_image)) {
						printf("Image header copied to output file successfully.\n");
				} else {
						printf("ERROR: Failed to copy image header\n\n");
						exit(e_success);
				}
				//Image data size should be larger than Magic String Size
				encInfo.image_capacity = get_image_size_for_bmp(encInfo.fptr_src_image); 
				if (!encInfo.image_capacity)
				{
						printf("ERROR: Source image file is empty.\n\n");
						exit(e_success);
				}
				encInfo.size_secret_file = get_file_size(encInfo.fptr_secret);
				if (!encInfo.size_secret_file)
				{
						printf("ERROR: Secret file to be encoded is empty.\n\n");
						exit(e_success);
				}
				printf("Secret data size = %lu bytes\n", encInfo.size_secret_file - CHAR_SIZE);//Last byte i.e. EOF is not considered in actual secret data size
				if(flag){
						//Magic string = MSS + secret file extn size + dot + file extn + secret data size + secret data 
						encInfo.magic_string_size = (CHAR_SIZE + INT_SIZE + CHAR_SIZE + encInfo.secret_extn_len + INT_SIZE + encInfo.size_secret_file - CHAR_SIZE) * 8;
						strcpy((char*)magic_string_signature, MAGIC_STRING);
				}
				//Check encoding capacity
				check_capacity(&encInfo) ? printf("Image data size is sufficient enough to encode the secret data\n"): printf("ERROR: Image data size is insufficient to encode the secret data\n\n");
				if (do_encoding(&encInfo))
				{
						printf("------Completed Encoding------\n");
				}
				else
				{
						printf("ERROR:------Encoding failed!!------\n");
						exit(e_success);
				}
				fclose(encInfo.fptr_src_image);//close source image file
				fclose(encInfo.fptr_secret);//Close secret file
		}


		else if (check_operation_type(argv + 1) == e_decode){//Decoding
				printf("Decoding is selected.\n");
				if (argc > 4) {
						printf("Error: Invalid number of command line arguments.\n\n");
						exit(e_success);
				}
				//Read and validate given output filename
				//Extract only filename from given path if any
				argv[2] = basename(argv[2]);
				if (read_and_validate_bmp_format(argv + 2) == e_failure) {
						printf("ERROR: Invalid filename provided in 4th command line argument. It must be a '.bmp' file\n\n");
						exit(e_success);
				}
				strcpy((char*)encInfo.stego_image_fname, argv[2]);
				switch (argc) {
						//For argc = 3, wait for file extension decoding
						case 3: default_ext_name = 1;
								break;
						case 4: //User defined decoded filename
								//Extract only filename from given path if any
								argv[3] = basename(argv[3]);
								strcpy((char*)temp_decode_name, argv[3]);
								break;
						default: 
								//Extract only filename from given path if any
								argv[3] = basename(argv[3]);
								strcpy((char*)temp_decode_name, argv[3]);
				                break;
				}
				//Open stego image file
				if((encInfo.fptr_stego_image = fopen((const char*)encInfo.stego_image_fname, "rb")) == NULL)
				{
						printf("ERROR: Unable to open file %s. This file may not be present in the current project directory.\n\n", encInfo.stego_image_fname);
						exit(e_success);
				}
				//Collect raster data offset
				fseek(encInfo.fptr_stego_image, 10L, SEEK_SET);
				fread(&raster_data, sizeof(raster_data), 1, encInfo.fptr_stego_image);
				fseek(encInfo.fptr_stego_image, raster_data, SEEK_SET);
				if (ferror(encInfo.fptr_stego_image)) {
						printf("ERROR: Error while reading file %s\n\n", encInfo.stego_image_fname);
						exit(e_success);
				}
				//Stego file index is now pointing at the end of raster data
				//Decode Magic String 
				if (do_decoding(&encInfo))
				{
						printf("------Completed Decoding------\n\n");
				}
				else
				{
						printf("ERROR: ------Decoding failed------\n\n");
						exit(e_success);
				}
				//close decoded output file
				fclose(encInfo.fptr_decoded_file);
		} else {//e_unsupported - Neither encoding nor decoding option
				printf("ERROR: 1st command line argument must be either '-e' for encoding or '-d' for decoding\n\n");
				exit(e_success);
		}
		//Close the output file
		fclose(encInfo.fptr_stego_image);//common file both in encoding & decoding part
		return 0;
}
//check operation type
OperationType check_operation_type(char *argv[])
{
        if (!strcmp(*argv, "-e"))//Check if argument is '-e'
                return e_encode;
        else if (!strcmp(*argv, "-d"))//Check if argument is '-d'
                return e_decode;
        else
                return e_unsupported;//Some other argument
}
