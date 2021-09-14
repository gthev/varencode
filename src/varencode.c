/*
 * VARENCODE.C
 * This file implements mechanics to store arrays of bytes (specifically of numbers)
 * in a compressed way, inspired by Google's varints.
 * author : gthev
 */
#include "printbin.h"
#include "varencode.h"
//#include <assert.h>
#include <stdio.h>
#include <errno.h>

// TODO : les delires de DEFINE avec LITTLE_ENDIAN et BIG_ENDIAN

int varencode(struct varencode_ctx ctx) {
	int total_size, entity, idx_current;
	int res_current_idx, zero_idx, zero_bit, is_last_byte;
	short current_bit, end_bit,  should_incr_idx;
	unsigned char *block_start, working_byte, next_byte;

	total_size = ctx.nmemb * ctx.size;
	block_start = (unsigned char*)ctx.data;
	res_current_idx = 0;
	if(ctx.nmemb == 0) return 0;
	if(ctx.size <= 0) return -EINVAL;

	// printf("nmemb : %d, size : %d\n", ctx.nmemb, ctx.size);

	for(entity = 0; entity < ctx.nmemb; entity++) {
		
		// printf("=======================\n");
		// printf("Entity : %d\n", entity);

		// ================================================================
		// /!\ this step depends on the architecture (little or big endian)
		// here : big endian

		// First, we want to detect where the first non-zero block lies
		// So zero_idx contains the idx of the first non-zero byte
		// zero_bit contains the idx of the first non-zero bit
		zero_idx = ctx.size - 1;
		zero_bit = 0;
		while(zero_idx >= 0) {
			if(block_start[zero_idx] == 0) zero_idx--;
			else break;
		}

		// if zero_idx < 0, it probably means that the whole number is 0,
		// so we just encode the first byte (0)
		if(zero_idx < 0) zero_idx = 0;


		// now, same idea inside the byte
		while(zero_bit <= 7) {
			// yes I could use the non operator '!'
			if((block_start[zero_idx] & (0x80 >> zero_bit)) == 0) zero_bit++;
			else break;
		}
		if(zero_bit > 7) zero_bit = 7;


#ifdef DEBUG_VARENCODE		
		printf("Zero index : %d, bit : %d\n", zero_idx, zero_bit);
#endif


		idx_current = 0;
		current_bit = 7; end_bit = 7;
		while(idx_current <= zero_idx) {

			should_incr_idx = 0;
			working_byte = block_start[idx_current];
			next_byte = 0;
			end_bit = (current_bit > 5) ? current_bit-6 : 0;
			is_last_byte = 0;

			// char_bin(working_byte << end_bit); printf("\n");
				
			working_byte = (working_byte << end_bit) >> (end_bit + 7 - current_bit);
			

#ifdef DEBUG_VARENCODE
			printf("idx : %d, cb : %d, eb : %d\n", idx_current, current_bit, end_bit);
#endif

			if(current_bit < 6 && idx_current < zero_idx) {

#ifdef DEBUG_VARENCODE
				printf("OVERLAP\n");
#endif


				should_incr_idx = 1;
				next_byte = block_start[idx_current+1];
				
				// printf("next_bit : %d\n", next_bit);

				next_byte <<= current_bit + 1;
				working_byte |= next_byte;
			}



			// char_bin(working_byte); printf("\n");



			// okay so now we (almost) have the byte that we want to write 
			// into the stream, we just have to determine if it's the last or not

			if(idx_current*8+7-current_bit > zero_idx*8-zero_bit) {	
				is_last_byte = 1;
			}

			if(is_last_byte) {
				// then it's the last ! we begin by a 0
				// well it's probably useless but hey	
				working_byte &= 0x7f; // AND with 0111 1111

#ifdef DEBUG_VARENCODE
				printf("last byte detected\n");
#endif
			} else {
				// otherwise, intermediate byte
				working_byte |= 0x80; // OR  with 1000 0000
			}

			// now's the time to write our result to the stream !

			if(res_current_idx >= total_size) {
				// Then it's overflow ! We exit with failure
				return -1;
			}

			((unsigned char*)ctx.res)[res_current_idx++] = working_byte;

			current_bit = (current_bit + 1) % 8;
			if(current_bit == 7) should_incr_idx = 1; // then we move on to the next byte
			if(should_incr_idx) idx_current++;
			if(is_last_byte) break;
		}
		

		// ================================================================

		block_start += ctx.size;
	}
	return res_current_idx;
}

int vardecode(struct vardecode_ctx ctx) {
	unsigned int total_size, idx_byte_in_entity, idx_entity;
        unsigned int encoded_current_idx, i;
        short current_construction_bit, current_encoded_bit, size_taken;
        unsigned char *current_entity; 
	unsigned char working_byte, construction_byte, is_not_last_byte;
	int rescode, already_commited;

        total_size = ctx.nmemb * ctx.size;
        if(ctx.nmemb == 0) return 0;
        if(ctx.size <= 0) return -EINVAL;
	idx_byte_in_entity = 0; idx_entity = 0;
	current_entity = (unsigned char*)ctx.decoded;

	// just set everything to 0
	for(i = 0; i<total_size; i++)
		((unsigned char*)ctx.decoded)[i] = 0;
	
	if(ctx.encoded_length == 0)
		return 0;

	current_construction_bit = 7; // different in case of LITTLE_ENDIAN
	current_encoded_bit = 7;
	size_taken = 0;
	working_byte = 0;
	construction_byte = 0;
	encoded_current_idx = 0;
	while(idx_entity < ctx.nmemb) {

		size_taken = (1+current_construction_bit > current_encoded_bit) ? 
				current_encoded_bit : 1+current_construction_bit;

		if(encoded_current_idx >= ctx.encoded_length) {
			// we shouldn't be here
			return -ECHRNG;
		}


		// printf("Bits : enc %d cons %d - SIZE : %d -  IDX : ent %d enc %d\n", current_encoded_bit, current_construction_bit,size_taken, idx_entity, encoded_current_idx);

		working_byte = ((unsigned char*)ctx.encoded)[encoded_current_idx];
		
		// printf("Current encoded : "); char_bin(working_byte);
		// printf("\n");

		working_byte >>= 7 - current_encoded_bit;
		working_byte <<= 7 - size_taken + 1;
		working_byte >>= 1 + current_construction_bit - size_taken;
		construction_byte |= working_byte;

		// printf("Constructing byte : "); char_bin(construction_byte);
		// printf("\n");

		current_encoded_bit -= size_taken;
		current_construction_bit -= size_taken;

		already_commited = 0;

		int commit_construction_byte(void) {
		
			// check for entity overflow
			// we want to place it in the idx_byte_in_entity byte in the idx_entity entity
			if(idx_byte_in_entity >= ctx.size) {
				// overflow inside entity
				return -EOVERFLOW;
			}
			if(idx_entity >= ctx.nmemb) {
				// overflow of entities
				return -EFAULT;
			}
			// if everything is alright, we commit the current byte
			current_entity[idx_byte_in_entity] = construction_byte;

			// printf("[o] Writing ");
			// char_bin(construction_byte);
			// printf(" at position %d\n", idx_byte_in_entity);

			idx_byte_in_entity++;
			construction_byte = 0;
			current_construction_bit = 7;
			// we don't do the potential inc to idx_entity to detect overflows
			// later
			return 0;
		};

		if(current_construction_bit < 0) { // normally == -1 should do it but hey
			// printf("END OF CONSTRUCTION BYTE\n");
			// yes I know I could write those two lines in one
			rescode = commit_construction_byte();
			if(rescode < 0) return rescode;
			already_commited = 1;
		}

		if(current_encoded_bit <= 0) { // idem : == 0 should suffice
			
			// printf("END OF ENCODED BYTE\n");

			is_not_last_byte = ((unsigned char*)ctx.encoded)[encoded_current_idx] & 0x80;

			if(!is_not_last_byte) {
				// then we go to the next entity in the decoded
				// PROBLEM HERE : we should also COMMIT
				if(!already_commited) {
					rescode = commit_construction_byte();
					if(rescode < 0) return rescode;
					already_commited = 1;
				}
				idx_byte_in_entity = 0;
				idx_entity++;
				current_entity += ctx.size;
			}

			current_encoded_bit = 7;
			encoded_current_idx++;
			if(encoded_current_idx < ctx.encoded_length && idx_entity >= ctx.nmemb) return -EFAULT;
			if(encoded_current_idx >= ctx.encoded_length && !is_not_last_byte) break;
			// if the last byte begins with a '1', then the stream is ill-formed:
			// we'll detect it at the beginning of the loop and return -ECHRNG;
		}

	}

	// no cleaning to do yay
	return 0;
}
