/*
 * FDES.c
 *
 *  Created on: Jun 21, 2012
 *      Author: MLISKOV
 */

#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DES_BLOCK_SIZE 8  // 8 byte = 64 bit blocks

/*
 * Round function for the Feistel cipher.
 *
 * Given an input "inbuf" and an output space "outbuf" and a pair of DES keys "key",
 * write into outbuf:
 *    DES_ENC(inbuf,key[0]) DES_ENC(inbuf+DES_BLOCK_SIZE,key[1]).
 */
void DES_encrypt_pair (void * outbuf, void * inbuf, unsigned char * key0, unsigned char * key1) {
	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);
	int outlen = 0;
	EVP_EncryptInit_ex(&ctx, EVP_des_ecb(), NULL, (void *)key0, NULL);
	EVP_EncryptUpdate(&ctx, outbuf, &outlen, inbuf, DES_BLOCK_SIZE);

	EVP_EncryptInit_ex(&ctx, EVP_des_ecb(), NULL, (void *)key1, NULL);
	EVP_EncryptUpdate(&ctx, outbuf+DES_BLOCK_SIZE, &outlen, inbuf+DES_BLOCK_SIZE, DES_BLOCK_SIZE);
}

int main (int argc, char ** argv) {

	if (argc != 3) {
		printf ("Usage: %s key input.\n", argv[0]);
		return 0;
	}

	FILE *keyfile = fopen (argv[1], "rb");
	FILE *infile = fopen (argv[2], "rb");

	// Use a Feistel structure to encrypt.
	// Round function will be two parallel DES ciphers.
	unsigned char key[2][2][8];
	// key[0] is the pair of keys used for the first round,
	// key[1] is the pair of keys used for the second round.
	// 2 rounds, 2 keys per round, 8 bytes per key.

	// read in the key
	int rv = 0;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			rv += fread (key[i][j], sizeof(char), 8, keyfile);
		}
	}
	fclose(keyfile);
	if (rv < 32) {
		printf ("Error: key %s not long enough.\n", argv[1]);
		return 1;
	}

	unsigned char input[32];
	rv = fread(input, sizeof(char), 32, infile);
	// Pad with 0 bytes if file was not large enough.
	for ( ; rv < 32; rv++) {
		input[rv] = 0;
	}
	fclose(infile);

	unsigned char round_input[32];
	memcpy(round_input, input, 32);
	// Split the input into left and right halves
	unsigned char *L = round_input;
	unsigned char *R = round_input+16;
	unsigned char tmp[32];

	// Two rounds
	for (int round = 0; round < 2; round++) {
		/* L_{n+1} = R_n */
		memcpy(tmp, R, 16);

		/* R_{n+1} = L_n XOR f_n(R_n) */
		/* get f_n(R_n) */
		DES_encrypt_pair(tmp+16, R, key[round][0], key[round][1]);
		/* XOR with L_n */
		for (int i = 0; i < 16; i++) {
			tmp[16+i] ^= L[i];
		}

		/* Re-set for next round */
		memcpy(round_input, tmp, 32);
	}

	// Write result to stdout
	fwrite (round_input, sizeof(char), 32, stdout);
}
