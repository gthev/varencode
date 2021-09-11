/*
 * VARENCODE.H
 * This file implements mechanics to store arrays of bytes (specifically of numbers)
 * in a compressed way, inspired by Google's varints.
 * author : gthev
 */

struct varencode_ctx {
	/*
	 * data : pointer to the data to encode
	 * res  : pointer to an allocated array, of the same size as data,
	 * 	  to be filled by the encoded result
	 * nmemb : number of entities in the given array
	 * size : size (in bytes) of an entity of the given array. Must be at least 1
	 * 
	 * So data and res should be of size nmemb*size bytes
	 * */
	void *data, *res;
	unsigned int nmemb, size; 
};

struct vardecode_ctx {
	/*
	 * encoded : encoded array, of size encoded_length bytes
	 * decoded : allocated array, of size nmemb * size, to be filled
	 * 	     by the decoded result
	 * */
	void *encoded, *decoded;
	unsigned int encoded_length;
	unsigned int nmemb, size;
};

/*
 * Encode an array of bytes as a varint stream-like array.
 * Takes a struct varencode_ctx as a parameter, and returns : 
 *  - a positive integer if the compression was succesful, indicating
 *  	how many bytes of ctx.res were effectively used
 *  - -1 if we'd have to overflow it (so the compression wouldn't be effective)
 * */
int varencode(struct varencode_ctx ctx);


/*
 * Decode a stream of varints
 * Takes a struct vardecode_ctx as a parameter, and returns :
 * 
 * 0 if everything's ok
 * -EINVAL if an argument is faulty (ctx.size is null for example)
 * -EFAULT if there are too much entities
 * -EOVERFLOW if a particular entity is overflowed
 * -ECHRNG if the encoded stream is overflowed (for example, if last byte begins by '1')
 *  
 * */
int vardecode(struct vardecode_ctx ctx);
