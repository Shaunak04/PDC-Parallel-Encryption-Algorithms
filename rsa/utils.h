#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <gmp.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include <omp.h>


#define ASCII_0 48
#define ASCII_9 57

#undef  ADD_STAT
#define ADD_STAT( _etype, _desc ) _etype
enum
{
    #include "stats.h"
    NUM_STATS
};
#undef ADD_STAT

#undef ADD_STAT
#define ADD_STAT( _etype, _desc ) _desc
static const char* all_stats[NUM_STATS] =
{
    #include "stats.h"
};
#undef ADD_STAT

char* get_message(char* text_file);
void print_stats(double* s_stats, double* p_stats);
double get_seconds(double clock_ticks);
char is_correct(char* orig_msg, char* decrypted, int num_chars);

void print_elapsed(char* msg, clock_t start, clock_t stop);
void print_bits8(char n);
void print_bits64(uint64_t n);
void print_blocks(uint64_t* blocks, int num_blocks);

int get_num_digits(mpz_t n);
int get_num_bits(mpz_t n);
void uint64_to_mpz(uint64_t* n, mpz_t a);
uint64_t mpz_to_uint64(mpz_t a);
void get_rand_intStr(char* intStr, int num_digits);

void p_get_rand_intStr(char* intStr, int num_digits);

#endif // UTILS_H
