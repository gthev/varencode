#include <stdio.h>

void char_bin(unsigned char n) {
        unsigned char i;
        for (i = 1 << 7; i > 0; i = i / 2)
                (n & i) ? printf("1") : printf("0");
}

void char_bin_f(FILE *f, unsigned char n) {
        unsigned char i;
        for (i = 1 << 7; i > 0; i = i / 2)
                (n & i) ? fprintf(f, "1") : fprintf(f, "0");
}

void short_bin(unsigned short n)
{
        unsigned short i;
        for (i = 1 << 15; i > 0; i = i / 2)
                (n & i) ? printf("1") : printf("0");
}

void int_bin(unsigned int n)
{
        unsigned i;
        for (i = 1 << 31; i > 0; i = i / 2)
                (n & i) ? printf("1") : printf("0");
}

void stream_bin(unsigned char *p, int length) {
        for(int i=0; i<length; i++) {
                char_bin(p[i]);
		printf(" ");
	}
}


void stream_bin_f(FILE *f, unsigned char *p, int length) {
        for(int i=0; i<length; i++) {
                char_bin_f(f, p[i]);
		fprintf(f, " ");
	}
}

