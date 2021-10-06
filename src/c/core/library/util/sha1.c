/*
 * sha1.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SHA-1 algorithmic calculation utility.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/util/sha1.h"

// declare sha-1 public functions

static unsigned long int get_value(unsigned char bb[64], unsigned int ii);

static void put_value(unsigned long int nn, unsigned char bb[64],
		unsigned long int ii);

static unsigned long int function_s(unsigned long int xx, unsigned long int nn);

static unsigned long int function_r(unsigned long int tt,
		unsigned long int ww[16]);

static unsigned long int function_f1(unsigned long int xx, unsigned long int yy,
		unsigned long int zz);

static unsigned long int function_f2(unsigned long int xx, unsigned long int yy,
		unsigned long int zz);

static unsigned long int function_f3(unsigned long int xx, unsigned long int yy,
		unsigned long int zz);

static void function_p(int mode, unsigned long int kk, unsigned long int *aa,
		unsigned long int *bb, unsigned long int *cc, unsigned long int *dd,
		unsigned long int *ee, unsigned long int xx);

static void calculate_hash(Sha1 *hash, unsigned char data[64]);

static void init_padding(unsigned char padding[64]);


// define sha-1 public functions

static unsigned long int get_value(unsigned char bb[64], unsigned int ii)
{
	unsigned long int result = 0;

	result = (((unsigned long int)bb[ii] << 24) |
			((unsigned long int)bb[(ii + 1)] << 16) |
			((unsigned long int)bb[(ii + 2)] << 8) |
			((unsigned long int)bb[(ii + 3)]));

	return result;
}

static void put_value(unsigned long int nn, unsigned char bb[64],
		unsigned long int ii)
{
	bb[ii] = (unsigned char)(nn >> 24);
	bb[(ii + 1)] = (unsigned char)(nn >> 16);
	bb[(ii + 2)] = (unsigned char)(nn >> 8);
	bb[(ii + 3)] = (unsigned char)(nn);
}

static unsigned long int function_s(unsigned long int xx, unsigned long int nn)
{
	return ((xx << nn) | ((xx & 0xffffffff) >> (32 - nn)));
}

static unsigned long int function_r(unsigned long int tt,
		unsigned long int ww[16])
{
	unsigned long int temp;

	temp = ww[(tt - 3) & 0x0f] ^ ww[(tt - 8) & 0x0f] ^ ww[(tt - 14) & 0x0f] ^
		ww[tt & 0x0f];

	ww[tt & 0x0f] = function_s(temp, 1);

	return ww[tt & 0x0f];
}

static unsigned long int function_f1(unsigned long int xx, unsigned long int yy,
		unsigned long int zz)
{
	return (zz ^ (xx & (yy ^ zz)));
}

static unsigned long int function_f2(unsigned long int xx, unsigned long int yy,
		unsigned long int zz)
{
	return (xx ^ yy ^ zz);
}

static unsigned long int function_f3(unsigned long int xx, unsigned long int yy,
		unsigned long int zz)
{
	return ((xx & yy) | (zz & (xx | yy)));
}

static void function_p(int mode, unsigned long int kk, unsigned long int *aa,
		unsigned long int *bb, unsigned long int *cc, unsigned long int *dd,
		unsigned long int *ee, unsigned long int xx)
{
	if(mode == 1) {
		*ee += function_s(*aa, 5) + function_f1(*bb, *cc, *dd) + kk + xx;
	}
	else if(mode == 2) {
		*ee += function_s(*aa, 5) + function_f2(*bb, *cc, *dd) + kk + xx;
	}
	else if(mode == 3) {
		*ee += function_s(*aa, 5) + function_f3(*bb, *cc, *dd) + kk + xx;
	}
	else if(mode == 4) {
		*ee += function_s(*aa, 5) + function_f2(*bb, *cc, *dd) + kk + xx;
	}
	*bb = function_s(*bb, 30);
}

static void calculate_hash(Sha1 *hash, unsigned char data[64])
{
	unsigned long int aa;
	unsigned long int bb;
	unsigned long int cc;
	unsigned long int dd;
	unsigned long int ee;
	unsigned long int ww[16];

	aa = hash->state[0];
	bb = hash->state[1];
	cc = hash->state[2];
	dd = hash->state[3];
	ee = hash->state[4];

	ww[0] = get_value(data, 0);
	ww[1] = get_value(data, 4);
	ww[2] = get_value(data, 8);
	ww[3] = get_value(data, 12);
	ww[4] = get_value(data, 16);
	ww[5] = get_value(data, 20);
	ww[6] = get_value(data, 24);
	ww[7] = get_value(data, 28);
	ww[8] = get_value(data, 32);
	ww[9] = get_value(data, 36);
	ww[10] = get_value(data, 40);
	ww[11] = get_value(data, 44);
	ww[12] = get_value(data, 48);
	ww[13] = get_value(data, 52);
	ww[14] = get_value(data, 56);
	ww[15] = get_value(data, 60);

	function_p(1, 0x5A827999, &aa, &bb, &cc, &dd, &ee, ww[0]);
	function_p(1, 0x5A827999, &ee, &aa, &bb, &cc, &dd, ww[1]);
	function_p(1, 0x5A827999, &dd, &ee, &aa, &bb, &cc, ww[2]);
	function_p(1, 0x5A827999, &cc, &dd, &ee, &aa, &bb, ww[3]);
	function_p(1, 0x5A827999, &bb, &cc, &dd, &ee, &aa, ww[4]);
	function_p(1, 0x5A827999, &aa, &bb, &cc, &dd, &ee, ww[5]);
	function_p(1, 0x5A827999, &ee, &aa, &bb, &cc, &dd, ww[6]);
	function_p(1, 0x5A827999, &dd, &ee, &aa, &bb, &cc, ww[7]);
	function_p(1, 0x5A827999, &cc, &dd, &ee, &aa, &bb, ww[8]);
	function_p(1, 0x5A827999, &bb, &cc, &dd, &ee, &aa, ww[9]);
	function_p(1, 0x5A827999, &aa, &bb, &cc, &dd, &ee, ww[10]);
	function_p(1, 0x5A827999, &ee, &aa, &bb, &cc, &dd, ww[11]);
	function_p(1, 0x5A827999, &dd, &ee, &aa, &bb, &cc, ww[12]);
	function_p(1, 0x5A827999, &cc, &dd, &ee, &aa, &bb, ww[13]);
	function_p(1, 0x5A827999, &bb, &cc, &dd, &ee, &aa, ww[14]);
	function_p(1, 0x5A827999, &aa, &bb, &cc, &dd, &ee, ww[15]);
	function_p(1, 0x5A827999, &ee, &aa, &bb, &cc, &dd, function_r(16, ww));
	function_p(1, 0x5A827999, &dd, &ee, &aa, &bb, &cc, function_r(17, ww));
	function_p(1, 0x5A827999, &cc, &dd, &ee, &aa, &bb, function_r(18, ww));
	function_p(1, 0x5A827999, &bb, &cc, &dd, &ee, &aa, function_r(19, ww));

	function_p(2, 0x6ED9EBA1, &aa, &bb, &cc, &dd, &ee, function_r(20, ww));
	function_p(2, 0x6ED9EBA1, &ee, &aa, &bb, &cc, &dd, function_r(21, ww));
	function_p(2, 0x6ED9EBA1, &dd, &ee, &aa, &bb, &cc, function_r(22, ww));
	function_p(2, 0x6ED9EBA1, &cc, &dd, &ee, &aa, &bb, function_r(23, ww));
	function_p(2, 0x6ED9EBA1, &bb, &cc, &dd, &ee, &aa, function_r(24, ww));
	function_p(2, 0x6ED9EBA1, &aa, &bb, &cc, &dd, &ee, function_r(25, ww));
	function_p(2, 0x6ED9EBA1, &ee, &aa, &bb, &cc, &dd, function_r(26, ww));
	function_p(2, 0x6ED9EBA1, &dd, &ee, &aa, &bb, &cc, function_r(27, ww));
	function_p(2, 0x6ED9EBA1, &cc, &dd, &ee, &aa, &bb, function_r(28, ww));
	function_p(2, 0x6ED9EBA1, &bb, &cc, &dd, &ee, &aa, function_r(29, ww));
	function_p(2, 0x6ED9EBA1, &aa, &bb, &cc, &dd, &ee, function_r(30, ww));
	function_p(2, 0x6ED9EBA1, &ee, &aa, &bb, &cc, &dd, function_r(31, ww));
	function_p(2, 0x6ED9EBA1, &dd, &ee, &aa, &bb, &cc, function_r(32, ww));
	function_p(2, 0x6ED9EBA1, &cc, &dd, &ee, &aa, &bb, function_r(33, ww));
	function_p(2, 0x6ED9EBA1, &bb, &cc, &dd, &ee, &aa, function_r(34, ww));
	function_p(2, 0x6ED9EBA1, &aa, &bb, &cc, &dd, &ee, function_r(35, ww));
	function_p(2, 0x6ED9EBA1, &ee, &aa, &bb, &cc, &dd, function_r(36, ww));
	function_p(2, 0x6ED9EBA1, &dd, &ee, &aa, &bb, &cc, function_r(37, ww));
	function_p(2, 0x6ED9EBA1, &cc, &dd, &ee, &aa, &bb, function_r(38, ww));
	function_p(2, 0x6ED9EBA1, &bb, &cc, &dd, &ee, &aa, function_r(39, ww));

	function_p(3, 0x8F1BBCDC, &aa, &bb, &cc, &dd, &ee, function_r(40, ww));
	function_p(3, 0x8F1BBCDC, &ee, &aa, &bb, &cc, &dd, function_r(41, ww));
	function_p(3, 0x8F1BBCDC, &dd, &ee, &aa, &bb, &cc, function_r(42, ww));
	function_p(3, 0x8F1BBCDC, &cc, &dd, &ee, &aa, &bb, function_r(43, ww));
	function_p(3, 0x8F1BBCDC, &bb, &cc, &dd, &ee, &aa, function_r(44, ww));
	function_p(3, 0x8F1BBCDC, &aa, &bb, &cc, &dd, &ee, function_r(45, ww));
	function_p(3, 0x8F1BBCDC, &ee, &aa, &bb, &cc, &dd, function_r(46, ww));
	function_p(3, 0x8F1BBCDC, &dd, &ee, &aa, &bb, &cc, function_r(47, ww));
	function_p(3, 0x8F1BBCDC, &cc, &dd, &ee, &aa, &bb, function_r(48, ww));
	function_p(3, 0x8F1BBCDC, &bb, &cc, &dd, &ee, &aa, function_r(49, ww));
	function_p(3, 0x8F1BBCDC, &aa, &bb, &cc, &dd, &ee, function_r(50, ww));
	function_p(3, 0x8F1BBCDC, &ee, &aa, &bb, &cc, &dd, function_r(51, ww));
	function_p(3, 0x8F1BBCDC, &dd, &ee, &aa, &bb, &cc, function_r(52, ww));
	function_p(3, 0x8F1BBCDC, &cc, &dd, &ee, &aa, &bb, function_r(53, ww));
	function_p(3, 0x8F1BBCDC, &bb, &cc, &dd, &ee, &aa, function_r(54, ww));
	function_p(3, 0x8F1BBCDC, &aa, &bb, &cc, &dd, &ee, function_r(55, ww));
	function_p(3, 0x8F1BBCDC, &ee, &aa, &bb, &cc, &dd, function_r(56, ww));
	function_p(3, 0x8F1BBCDC, &dd, &ee, &aa, &bb, &cc, function_r(57, ww));
	function_p(3, 0x8F1BBCDC, &cc, &dd, &ee, &aa, &bb, function_r(58, ww));
	function_p(3, 0x8F1BBCDC, &bb, &cc, &dd, &ee, &aa, function_r(59, ww));

	function_p(4, 0xCA62C1D6, &aa, &bb, &cc, &dd, &ee, function_r(60, ww));
	function_p(4, 0xCA62C1D6, &ee, &aa, &bb, &cc, &dd, function_r(61, ww));
	function_p(4, 0xCA62C1D6, &dd, &ee, &aa, &bb, &cc, function_r(62, ww));
	function_p(4, 0xCA62C1D6, &cc, &dd, &ee, &aa, &bb, function_r(63, ww));
	function_p(4, 0xCA62C1D6, &bb, &cc, &dd, &ee, &aa, function_r(64, ww));
	function_p(4, 0xCA62C1D6, &aa, &bb, &cc, &dd, &ee, function_r(65, ww));
	function_p(4, 0xCA62C1D6, &ee, &aa, &bb, &cc, &dd, function_r(66, ww));
	function_p(4, 0xCA62C1D6, &dd, &ee, &aa, &bb, &cc, function_r(67, ww));
	function_p(4, 0xCA62C1D6, &cc, &dd, &ee, &aa, &bb, function_r(68, ww));
	function_p(4, 0xCA62C1D6, &bb, &cc, &dd, &ee, &aa, function_r(69, ww));
	function_p(4, 0xCA62C1D6, &aa, &bb, &cc, &dd, &ee, function_r(70, ww));
	function_p(4, 0xCA62C1D6, &ee, &aa, &bb, &cc, &dd, function_r(71, ww));
	function_p(4, 0xCA62C1D6, &dd, &ee, &aa, &bb, &cc, function_r(72, ww));
	function_p(4, 0xCA62C1D6, &cc, &dd, &ee, &aa, &bb, function_r(73, ww));
	function_p(4, 0xCA62C1D6, &bb, &cc, &dd, &ee, &aa, function_r(74, ww));
	function_p(4, 0xCA62C1D6, &aa, &bb, &cc, &dd, &ee, function_r(75, ww));
	function_p(4, 0xCA62C1D6, &ee, &aa, &bb, &cc, &dd, function_r(76, ww));
	function_p(4, 0xCA62C1D6, &dd, &ee, &aa, &bb, &cc, function_r(77, ww));
	function_p(4, 0xCA62C1D6, &cc, &dd, &ee, &aa, &bb, function_r(78, ww));
	function_p(4, 0xCA62C1D6, &bb, &cc, &dd, &ee, &aa, function_r(79, ww));

	hash->state[0] += aa;
	hash->state[1] += bb;
	hash->state[2] += cc;
	hash->state[3] += dd;
	hash->state[4] += ee;
}

static void init_padding(unsigned char padding[64])
{
	memset(padding, 0, sizeof(padding));

	padding[0] = 0x80;
}

// define sha-1 public functions

int sha1_init(Sha1 *hash)
{
	if(hash == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(hash, 0, sizeof(Sha1));

	hash->total[0] = 0;
	hash->total[1] = 0;

	hash->state[0] = 0x67452301;
	hash->state[1] = 0xEFCDAB89;
	hash->state[2] = 0x98BADCFE;
	hash->state[3] = 0x10325476;
	hash->state[4] = 0xC3D2E1F0;

	return 0;
}

int sha1_hashData(Sha1 *hash, unsigned char *input, int inputLength)
{
	unsigned long int left = 0;
	unsigned long int fill = 0;

	if((hash == NULL) || (input == NULL) || (inputLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	left = hash->total[0] & 0x3f;
	fill = 64 - left;

	hash->total[0] += (unsigned long int)inputLength;
	hash->total[0] &= 0xffffffff;

	if(hash->total[0] < (unsigned long int)inputLength) {
		hash->total[1] += 1;
	}

	if((unsigned long int)(left && (unsigned long int)inputLength) >= fill) {
		memcpy((void *)(hash->buffer + left), (void *)input, fill);
		calculate_hash(hash, hash->buffer);
		inputLength -= fill;
		input += fill;
		left = 0;
	}

	while(inputLength >= 64) {
		calculate_hash(hash, input);
		inputLength -= 64;
		input += 64;
	}

	if((unsigned long int)inputLength) {
		memcpy((void *)(hash->buffer + left), (void *)input, inputLength);
	}

	return 0;
}

int sha1_calculateHash(Sha1 *hash)
{
	unsigned long int high = 0;
	unsigned long int low = 0;
	unsigned long int last = 0;
	unsigned long int paddingLength = 0;
	unsigned char message[8];

	if(hash == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	high = ((hash->total[0] >> 29) | (hash->total[1] << 3));
	low = (hash->total[0] << 3);

	put_value(high, message, 0);
	put_value(low, message, 4);

	last = (hash->total[0] & 0x3f);
	paddingLength = (last < 56) ? (56 - last) : (120 - last);

	init_padding(hash->padding);
	sha1_hashData(hash, hash->padding, paddingLength);
	sha1_hashData(hash, message, sizeof(message));

	put_value(hash->state[0], hash->digest, 0);
	put_value(hash->state[1], hash->digest, 4);
	put_value(hash->state[2], hash->digest, 8);
	put_value(hash->state[3], hash->digest, 12);
	put_value(hash->state[4], hash->digest, 16);

	return 0;
}

int sha1_constructSignature(Sha1 *hash, char **signature,
		int *signatureLength)
{
	int ii = 0;
	int length = 0;
	char *result = NULL;

	if((hash == NULL) || (signature == NULL) || (signatureLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	length = ((20 * 2) + 1);
	result = (char *)malloc(sizeof(char) * (length + 1));

	for(ii = 0; ii < 20; ii++) {
		snprintf((result + (ii * 2)), (length - (ii * 2)), "%02x",
				hash->digest[ii]);
	}

	*signature = result;
	*signatureLength = (20 * 2);

	return 0;
}

int sha1_constructSignatureStatic(Sha1 *hash, char signature[40])
{
	int ii = 0;
	char buffer[8];

	if(hash == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(signature, 0, sizeof(signature));

	for(ii = 0; ii < 20; ii++) {
		snprintf(buffer, sizeof(buffer), "%02x", hash->digest[ii]);
		signature[((ii * 2) + 0)] = buffer[0];
		signature[((ii * 2) + 1)] = buffer[1];
	}

	return 0;
}

