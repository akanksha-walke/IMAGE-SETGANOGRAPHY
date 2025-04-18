#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

//Extern required values from main file
extern uint raster_data, passcode_len;
extern uchar magic_string_signature[CHAR_SIZE + MAX_PASSCODE_LEN + CHAR_SIZE];

/*functions definations */
Status read_and_validate_bmp_format(char *argv[])
{
        //Pointer to hold address of '.bmp' part from given argument
        const char* bmp_holder = strstr(*argv, ".bmp");
        if(bmp_holder)//Error handling
        {//If '.bmp' part is found, then check if string exactly ends with '.bmp'
                return (!strcmp(bmp_holder, ".bmp")) ? e_success : e_failure;
        }
        return e_failure;//NULL address which means '.bmp' part is not found
}

Status read_and_validate_extn(uchar_ptr sec_file_name_holder, EncodeInfo *encInfo)
{
        //Pointer to hold the heap memory of the size of filename including '\0' character
        uchar_ptr sec = (uchar_ptr) malloc(strlen((const char*)sec_file_name_holder) + 1);
        if (sec == NULL)
        {
                printf("ERROR: Unable to allocate dynamic memory.\n\n");
                return e_failure;
        }
        strcpy((char*)sec, (const char*)sec_file_name_holder);//Store the filename inside allocated heap
        uint secret_filename_len = strlen((const char*)sec);//Get length of filename
        char* ext = strtok((char*)sec, ".");//Get part of string before dot
        //If there is no dot in the filename, length of string remains the same
        if (strlen(ext) == secret_filename_len) {
                printf("ERROR: There is no dot in the given filename.\n");
                return e_failure;
        }
        //Extract the extension of secret file (i.e. part of string after dot)
        ext = strtok(NULL, ".");
        strcpy((char*)encInfo->extn_secret_file, (const char*)ext);//Store the extracted extension
        //Get and store length of secret extension
        encInfo->secret_extn_len = strlen((const char*)encInfo->extn_secret_file);
        //Validate extension size
        if (encInfo->secret_extn_len > MAX_FILE_SUFFIX) {
                printf("ERROR: file extension should not exceed 4 characters\n");
                return e_failure;
        }
        free(sec);//Free the allocated block of memory
        return e_success;//No errors found
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
     char str[54];
    fseek(fptr_src_image , 0, SEEK_SET);
    fread(str, 54 , 1 , fptr_src_image);//read from source file
    fwrite(str , 54 , 1, fptr_dest_image);//and then write it in destination file
    return e_success;
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
		uint img_size;//Return value to be read from image file
		//Seek to 34th byte to get image data size from the '.bmp' image file
		fseek(fptr_image, 34L, SEEK_SET);
		//Read the width (an unsigned integer)
		fread(&img_size, sizeof(img_size), 1, fptr_image);
		if (ferror(fptr_image))
		{
				printf("ERROR: Error while reading from the image file.\n\n");
				exit(e_success);
		}
		//Return image capacity
		return img_size;
}

uint get_file_size(FILE *fptr)//Returns file size including EOF byte
{
		//Seek to the end of file
		fseek(fptr, 0L, SEEK_END);
		return (uint) ftell(fptr);//Return file index value
}

Status open_files(EncodeInfo *encInfo)
{
		//Open source image file with proper error handling
		if ((encInfo->fptr_src_image = fopen((const char*)encInfo->src_image_fname, "rb")) == NULL)
		{
				perror("fopen");
				fprintf(stderr, "ERROR: Unable to open file %s. This file may not be present in the current project directory.\n\n", encInfo->src_image_fname);
				return e_failure;
		}
		//Open secret file with proper error handling
		if ((encInfo->fptr_secret = fopen((const char*)encInfo->secret_fname, "rb")) == NULL)
		{
				perror("fopen");
				fprintf(stderr, "ERROR: Unable to open file %s. This file may not be present in the current project directory.\n\n", encInfo->secret_fname);
				return e_failure;
		}
		//Open output image file with proper error handling
		if ((encInfo->fptr_stego_image = fopen((const char*)encInfo->stego_image_fname, "wb")) == NULL)
		{
				perror("fopen");
				fprintf(stderr, "ERROR: Unable to open file %s. This file may not be present in the current project directory.\n\n", encInfo->stego_image_fname);
				return e_failure;
		}
		return e_success;//No error found
}

Status check_capacity(EncodeInfo *encInfo)
{
		//Print messages
		printf("Image data size = %u bytes\n", encInfo->image_capacity);
		printf("Magic string size = %u bytes\n", encInfo->magic_string_size);
		//Check if image data size is greater than magic string size
		return (encInfo->magic_string_size < encInfo->image_capacity) ? e_success : e_failure;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
 uchar scan_char;//Read and store each byte into a character
        //Outer iteration till the size of given string
        for (uint i = 0; i < strlen(magic_string); i++)
        {
                for (int j = 7; j >= 0; j--)//8 times inner iteration, Note that iterator j should not be of the type uint.
                {//Read each byte
                        fread(&scan_char, sizeof(scan_char), 1, encInfo->fptr_src_image);
                        if (ferror(encInfo->fptr_src_image))//Error handling
                        {
                                printf("ERROR: Error while reading from source image file.\n\n");
                                return e_failure;
                        }
                        scan_char &= 0xFE;//Clear the least significant bit of fetched character
                        if (magic_string[i] & (01 << j))//Check every bit of magic string
                        {
                                scan_char |= 01;//Set the least significant bit of obtained character
                        }
                        else
                        {
                                scan_char |= 00;//Clear the least significant bit of obtained character
                        }
                        //Write the obtained byte onto output file
                        fwrite(&scan_char, sizeof(scan_char), 1, encInfo->fptr_stego_image);
                        if (ferror(encInfo->fptr_stego_image))//Error handling
                           {
                              printf("ERROR: Error while writing onto output image file.\n\n");
                              return e_failure;
                           }
                }
        }
        return e_success;//No errors found
}

Status encode_int_size_expression(uint len, EncodeInfo *encInfo)
{
        uchar scan_char;//Read and store each byte into a character
        for (int j = INT_SIZE * 8 - 1; j >= 0; j--)//Fetch every byte till integer size
        {
                //Read each byte
                fread(&scan_char, sizeof(scan_char), 1, encInfo->fptr_src_image);
                if (ferror(encInfo->fptr_src_image))//Error handling
                {
                        printf("ERROR: Error while reading from source image file.\n\n");
                        return e_failure;
                }
                scan_char &= 0xFE;//Clear the least significant bit of obtained character
                if (len & (1 << j))//Check every bit of obtained length
                {
                        scan_char |= 01;//Set the least significant bit of obtained character
                }
                else
                {
                        scan_char |= 00;//Clear the least significant bit of obtained character
                }
                //Write obtained byte onto output file
                fwrite(&scan_char, sizeof(scan_char), 1, encInfo->fptr_stego_image);
                if (ferror(encInfo->fptr_stego_image))//Error handling
                {
                    printf("ERROR: Error while writing onto output image file.\n\n");
                    return e_failure;
                }
        }
        return e_success;//No errors found
}
Status copy_remaining_image_data(FILE *fptr_src_image, FILE *fptr_dest_image, uint f_size)
{
    char ch;
    while(fread(&ch, 1, 1, fptr_src_image) > 0)
    {
        fwrite(&ch, 1, 1, fptr_dest_image);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
		//Encode magic string signature
		fseek(encInfo->fptr_src_image, raster_data, SEEK_SET);
		if (encode_magic_string((const char*)magic_string_signature, encInfo))
		{
				printf(" Magic string successfully encoded.\n");
		}
		else
		{
				printf("ERROR: Error while encoding magic string signature.\n\n");
				return e_failure;
		}
		//Encode secret file extension length
		if (encode_int_size_expression(encInfo->secret_extn_len, encInfo))
		{
				printf("Secret file extension length successfully encoded.\n");
		}
		else
		{
				printf("ERROR: Error while encoding secret file extension length.\n\n");
				return e_failure;
		}
		//Encode the dot in secret file name
		if (encode_magic_string(".", encInfo))
        {
            printf("\n");
		}
		else
		{
				printf("ERROR: Error while encoding the dot.\n\n");
				return e_failure;
		}
		//Encode the secret file extension
		if (encode_magic_string((const char*)(encInfo->extn_secret_file), encInfo))
		{
				printf("Successfully encoded the secret file extension.\n");
		}
		else
		{
				printf("ERROR: Error while encoding the secret file extension.\n\n");
				return e_failure;
		}
		//Encode the secret file size
		if (encode_int_size_expression(encInfo->size_secret_file - CHAR_SIZE, encInfo))
		{
				printf("Secret file size successfully encoded.\n");
		}
		else
		{
				printf("ERROR: Error while encoding secret file size.\n\n");
				return e_failure;
		}
		//Encode the secret data
		// create a string to store the secret_data
		uchar_ptr secret_data = (uchar_ptr) malloc(encInfo->size_secret_file * sizeof(uchar));
		if (secret_data == NULL)
		{   
				printf("ERROR: Unable to allocate dynamic memory.\n\n");
				return e_failure;
		}   
		rewind(encInfo->fptr_secret);
		fread(secret_data, encInfo->size_secret_file * sizeof(uchar) - CHAR_SIZE, 1, encInfo->fptr_secret);
		secret_data[encInfo->size_secret_file - CHAR_SIZE] = '\0';//Set last character as NUL character
		//Now  encode secret data
		if (encode_magic_string((const char*)secret_data, encInfo))
		{
				printf("Successfully encoded the secret data.\n");
		}
		else
		{
				printf("ERROR: Error while encoding the secret data.\n\n");
				return e_failure;
		}
		free(secret_data);
		//Copy remaining image bytes
		if (copy_remaining_image_data((FILE*) encInfo->fptr_src_image,(FILE*) encInfo->fptr_stego_image, encInfo->image_capacity - encInfo->magic_string_size + CHAR_SIZE))//To append EOF i.e. the last byte, we should add byte size
	   	{
				printf(" Remaining image data copied to output file successfully\n");
		} else {
				printf("ERROR: Failed to copy remaining image data\n\n");
				return e_failure;
		}
		return e_success;//No error found
}
