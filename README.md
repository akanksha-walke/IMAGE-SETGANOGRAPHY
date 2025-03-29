# **LSB IMAGE STEGANOGRAPHY**


![FnULg](https://github.com/user-attachments/assets/97956793-c6d6-4b54-9a52-5e0899516994)

### Overview of Project

By concealing information within other information, the LSB Image Steganography project demonstrates the technique of concealing the fact that communication is occurring. With LSB steganography, data is concealed in the image's least significant pixel values.  Although a variety of carrier file formats are available, digital photographs are the most widely employed due to their frequent appearance on the internet.


### **FEATURES**
* Insert text data into an image using LSB steganography.
- Recover hidden text data from a steganographic image.

### Technologies Used
C language – Function pointers, File I/O operations, File pointers, string
operations, Bitwise operations.

### **USAGE**
**Compilation**

Compile the program using a C compiler.

For example:

  1. gcc *.c
  2. ./a.out -e beautiful.bmp secret.txt stego.bmp  --> **For Encoding**
  3. ./a.out -d stego.bmp decode.txt  --> **For Decoding**

  

      
