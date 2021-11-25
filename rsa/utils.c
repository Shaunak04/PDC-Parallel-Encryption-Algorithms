#include "utils.h"

char* get_message(char* text_file) {
    
    FILE* t = fopen(text_file, "r");
    if(!t) {
        printf("%s was not found.\n", text_file);
        return NULL;
    }
 	    
    // get file size
    fseek(t, 0, SEEK_END);
    long int num_bytes = ftell(t);
    
    // copy file contents
    rewind(t);
    char* message = (char*) malloc(num_bytes + 1);
    memset(message, 0, num_bytes + 1);
    fread(message, num_bytes, 1, t);   
    fclose(t);

    return message;
}

void print_stats(double* s_stats, double* p_stats) {

    double s_total = 0.0;
    double p_total = 0.0;

    printf("\n%s %16s %16s", "STATISTIC", "sequential", "parallel");   
    for(int i=0; i<NUM_STATS; i++) {
        double s = s_stats[i];
        double p = p_stats[i];

        char faster = (s < p) ? 's' : 'p';
    
        s_total += s;
        p_total += p;
    }
    printf("\n%s %16.8f %16.8f \n", "TOTAL TIME", s_total, p_total);

}

double get_seconds(double clock_ticks) {
    return clock_ticks / CLOCKS_PER_SEC;
}

char is_correct(char* orig_msg, char* decrypted, int num_chars) {
    char correct = 1;
    for(int c=0; c<num_chars; c++) {
        if(orig_msg[c] != decrypted[c]) {
            correct = 0;
            break;
        }
    }

    return correct;
}

int get_num_digits(mpz_t n) {
	return mpz_sizeinbase(n, 10);
}

int get_num_bits(mpz_t n) {
	return mpz_sizeinbase(n, 2);
}

// https://stackoverflow.com/questions/51601666/gmp-store-64-bit-interger-in-mpz-t-mpz-class-and-get-64-bit-integers-back?noredirect=1&lq=1	
uint64_t mpz_to_uint64(mpz_t a) {
	const size_t word_size = sizeof(uint64_t);
	size_t word_count = 0;
	void* out_raw = mpz_export(NULL, &word_count, 1, word_size, 0, 0, a);
	if(word_count != 1) {
		printf("mpz_to_uint64() failed.\n");
	}
	return *(uint64_t*) out_raw;
}

void uint64_to_mpz(uint64_t* n, mpz_t a) {
	mpz_import(a, 1, 1, sizeof(uint64_t), 0, 0, n);
}

// this assumes that intStr has room for num_digit digits plus the null terminator... 
void get_rand_intStr(char* intStr, int num_digits) {
	for(int i=0; i<num_digits; i++) {
		// (rand() % (upper - lower + 1)) + lower
		intStr[i] = (char)(rand() % (ASCII_9 - ASCII_0 + 1)) + ASCII_0;	
	}
	intStr[num_digits] = '\0';
}

void print_elapsed(char* msg, clock_t start, clock_t stop) {
	printf("%s: %.5f sec\n", msg, ( (double)(stop-start) ) / CLOCKS_PER_SEC); 
}

void print_bits8(char n) {
	printf("%c (%-3i) = ", n, n);
	for(int i=7; i>=0; i--) {
		if(n & (1 << i)) printf("1");
		else printf("0");
		if(i % 4 == 0) printf(" ");
	}
	printf("\n");
}

void print_bits64(uint64_t n) {
	printf("%-32lu: ", n);
	printf("| ");
	for(int i=63; i>=0; i--) {
		if(n & (1ULL << i)) printf("1");
		else printf("0");
		if(i % 4 == 0) printf(" ");
		if(i % 8 == 0) printf("| ");
	}
	printf("\n");
}

void print_blocks(uint64_t* blocks, int num_blocks) {
	for(int i=0; i<num_blocks; i++) {
		printf("Block %i ", i);
		print_bits64(blocks[i]);
	}
}

/* Parallel */

// this assumes that intStr has room for num_digit digits plus the null terminator... 
void p_get_rand_intStr(char* intStr, int num_digits) {
    
    omp_set_num_threads(2);
    #pragma omp parallel for
    for(int i=0; i<num_digits; i++) {
		// (rand() % (upper - lower + 1)) + lower
		intStr[i] = (char)(rand() % (ASCII_9 - ASCII_0 + 1)) + ASCII_0;	
	}
	intStr[num_digits] = '\0';

}