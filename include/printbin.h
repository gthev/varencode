/*
 * PRINTBIN.H
 *
 * Utility functions to print data in binary format
 * author : gthev
 * */
#include <stdio.h>

/*
 * Prints a byte (unsigned char) in standard output in binary format
 * */
void char_bin(unsigned char n);

/*
 * Prints a short integer in standard output in binary format
 * */
void short_bin(unsigned short n);

/*
 * Prints an interger in standard output in binary format
 * */
void int_bin(unsigned int n);

/*
 * Prints $length bytes in standard output in binary format,
 * starting from the address $p
 * */
void stream_bin(unsigned char *p, int length);

/*
 * Same as char_bin, but printed into file f
 * */
void char_bin_f(FILE *f, unsigned char n);

/*
 * same as stream_bin, but printed into file f
 * */
void stream_bin_f(FILE *f, unsigned char *p, int length);
