#include "printbin.h"
#include "varencode.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	unsigned int data[3] = {103578, 829929, 79353};
	unsigned int res[3] = {0, 0, 0};
	int size;

	size = 3;
	stream_bin((unsigned char*)data, size*sizeof(unsigned int));
	printf("\n");
	
	struct varencode_ctx ctx = {(void*)data, (void*)res, size, sizeof(unsigned int)};
	int rescode = varencode(ctx);
	printf("%d\n", rescode);

	// if rescode == -1, it doesn't mean that there was an error, but that
	// the compression method does not work here
	if(rescode <= 0)
		return -1;

	stream_bin((unsigned char*)ctx.res, rescode);
	printf("\n");

	struct vardecode_ctx ctx_de = {(void*)ctx.res,(void*)data, rescode, size, sizeof(unsigned int)};
	rescode = vardecode(ctx_de);
	printf("%d\n", rescode);
	// here, if rescode < 0, it means that an error occured
	if(rescode < 0)
		return -1;

	stream_bin((unsigned char*)ctx_de.decoded, size*sizeof(unsigned int));
	printf("\n");
	return 0;
}
