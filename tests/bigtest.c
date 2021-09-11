/*
 * BIGTEST.C
 *
 * Test file for varencode. Generates random arrays, encodes them and decodes the results,
 * and compare it to the initial array. Dumps error into a separate file, 'bigtest_dump' by default.
 * author : gthev
 * */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "printbin.h"
#include "varencode.h"

/*
 * dump_test prints things into a dump file
 * we can give him base, encoded, and decoded, with appropriate sizes,
 * print_limit should be between 0 and 2 inclusive
 * If 0 : only base is printed
 * If 1 : base and encoded
 * If 2 : base, encoded and decoded
 * */
void dump_test(FILE *f, unsigned *base, unsigned *encoded, unsigned *decoded, 
		int size, int size_encoded, unsigned short print_limit) {
	int i;
	fprintf(f, "Base\n");
	for(i=0;i<size;i++) {
		fprintf(f, "%d ", base[i]);
	}
	fprintf(f, "\n");
	if(print_limit < 1) return;

	fprintf(f, "Encoded\n");
	stream_bin_f(f, (char*)encoded, size_encoded);
	fprintf(f, "\n");
	if(print_limit < 2) return;
	
	fprintf(f, "Decoded\n");
	for(i=0;i<size;i++) {
		fprintf(f, "%d ", decoded[i]);
	}
	fprintf(f, "\n");
}

/*
 _______  _______ _________ _       
(       )(  ___  )\__   __/( (    /|
| () () || (   ) |   ) (   |  \  ( |
| || || || (___) |   | |   |   \ | |
| |(_)| ||  ___  |   | |   | (\ \) |
| |   | || (   ) |   | |   | | \   |
| )   ( || )   ( |___) (___| )  \  |
|/     \||/     \|\_______/|/    )_)

credits to patorjk.com for the ascii art
 * */
int main(int argc, char* argv[]) {

	unsigned n_test, i, min_val, max_val, min_size, max_size;
	unsigned nb_failures, nb_internal_errs, nb_distincts;
	unsigned *base, *encoded, *decoded;
	int rescode, ressize, size, j;
	struct varencode_ctx ctx_enc;
	struct vardecode_ctx ctx_dec;
	FILE *fdump;
	
	/*
	 ***********************************************
	     Modifiable parameters of the test suite
	 ***********************************************
	 * */
	srand(time(NULL));
	n_test = 10000;
	min_val = 0; max_val = 10000000;
	max_size = 500; min_size = 2;
	
	/*
	 ***********************************************
	 ***********************************************
	 * */
	
	// cf at the end of main for a details on the difference between those stats
	nb_failures = 0; nb_distincts = 0; nb_internal_errs = 0;

	base = NULL; encoded = NULL; decoded = NULL;
	fdump = fopen("bigtest_dump", "w");

	if(!fdump) {
		printf("Opening dump file failed. Exiting\n");
		return -1;
	}

	for(i=0; i<n_test; i++) {
		
		// we generate a random size
		size = (rand() % (max_size - min_size)) + min_size;
		ressize = 0;
		// TODO : change it to clean realloc
		base = (unsigned*)calloc(size, sizeof(unsigned));
		encoded = (unsigned*)calloc(size, sizeof(unsigned));
		decoded = (unsigned*)calloc(size, sizeof(unsigned));
		if(!base || !encoded || !decoded) {
			printf("array allocation of test %d/%d failed\n",
					i, n_test);
			return -1;
		}

		// and generate random values
		for(j=0; j<size; j++) {
			base[j] = (rand() % (max_val - min_val)) + min_val;
		}

		// okay for generating values
		// now we test the encoding
		
		// we fill the struct varencode_ctx
		ctx_enc.data = (void*)base;
		ctx_enc.res = (void*)encoded;
		ctx_enc.nmemb = size;
		ctx_enc.size = sizeof(unsigned);
		// the actual encoding is done here, and the result is in ctx_enc.res
		rescode = varencode(ctx_enc);

		if(rescode < 0) {
			// the compression was not efficient.
			// it doesn't (necessarily) mean that there was an internal problem,
			// it can mean that the array cannot be compressed with this method
			printf("Test %d/%d encoding failed. Dumping base array\n",
					i, n_test);
			fprintf(fdump, "============================\n Test %d/%d : encoding failed\n",
					i, n_test);
			
			dump_test(fdump, base, encoded, decoded, size, 0, 0);

			fprintf(fdump, "\n============================\n");
			nb_failures++;
			continue;
		}

		// else encoding seems to have worked
		// so the length of the encoded stream is given in rescode
		ressize = rescode;
		
		// to decode, we fill the struct vardecode_ctx
		ctx_dec.encoded = (void*)encoded;
		ctx_dec.decoded = (void*)decoded;
		ctx_dec.encoded_length = ressize;
		ctx_dec.nmemb = size;
		ctx_dec.size = sizeof(unsigned);

		// the actual decoding is done here, the result can be found in ctx_dec.decoded
		rescode = vardecode(ctx_dec);

		if(rescode < 0) {
			// then decoding failed
			// we print base and encoded, and the error number associated
			printf("Test %d/%d decoding failed. Dumping base and encoded arrays\n",
					i, n_test);
			fprintf(fdump, "===========================\n Test %d/%d :"
					"decoding failed with %s\n",
					i, n_test, strerror(-rescode));

			dump_test(fdump, base, encoded, decoded, size, ressize, 1);

			fprintf(fdump, "===========================\n");
			nb_internal_errs++;
			continue;
		}

		// here : okay, decoding succeeded, we check that all numbers are the same
		for(j=0; j<size; j++) {
			if(base[j] != decoded[j]) {
				//oooooh problem
				printf("Test %d/%d different. Dumping base, encoded and decoded arrays\n",
						i, n_test);
				fprintf(fdump, "===========================\n Test %d/%d :"
						"detected difference between base and decoded\n",
						i, n_test);

				dump_test(fdump, base, encoded, decoded, size, ressize, 2);

				fprintf(fdump, "===========================\n");
				nb_distincts++;
				break;
			}
		}

		// okay we go on to the next ones
		// TODO : change it to clean realloc
		free(base); free(encoded); free(decoded);
	}


	/* okay so those stats mean:
	 > a FAILURE (nb_failures) is a case where the compression did not return an error, but
		the compression was not efficient in this case, this method does not work here.
	 > an INTERNAL_ERROR (nb_internal_errs) is a case where there was a proper and 
	 	anormal error encountered during the process
	 > a DIFFERENT RESULT case (nb_distincts) is a case where the compression/decompression
	 	went without error, but there was a problem because numbers before compression
		and after decompression are different.
	*/
	printf("*******************************\n");
	printf("Tests ended up with %d total tests, %d failures, %d errors, and %d incorrect results.\n",
			n_test, nb_failures, nb_internal_errs, nb_distincts);

	return 0;
}
