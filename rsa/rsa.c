#include "rsa.h"

uint64_t* rsa(int mode, uint64_t* blocks, int num_blocks, mpz_t e_or_d, mpz_t n) 
{
	uint64_t* cipher_blocks = (uint64_t*) malloc(num_blocks * sizeof(uint64_t));

	// will apply rsa to each block
	for(int i=0; i<num_blocks; i++) 
	{
		
		// convert block to mpz_t
		uint64_t* block_uint64 = &blocks[i];
		mpz_t block;
		mpz_init(block);
		uint64_to_mpz(block_uint64, block);

		// exponentiation by repeated squaring/multiplying
		mpz_t C;
		mpz_init(C);
		mpz_set_ui(C, 1);

		// go through each bit in e
		for(int b=get_num_bits(e_or_d); b>=0; b--) 
		{
			mpz_powm_ui(C, C, 2, n); // C^2 mod n
			int bit_val = mpz_tstbit(e_or_d, b);
			if(bit_val) 
			{ // b == 1
				mpz_mul(C, C, block); // C*M
				mpz_mod(C, C, n); // C*M mod n
			}
		}
		cipher_blocks[i] = mpz_to_uint64(C); // C is the encrypted form of the block

		mpz_clear(block);
		mpz_clear(C);
	}
	return cipher_blocks;
}

// e is the multiplicative inverse of d mod (p-1)(q-1) -> Public key uses 'e'
void get_e(mpz_t e, mpz_t p, mpz_t q, mpz_t d) {
	
	// calculate (p-1) * (q-1)
	
	// (p-1)
	mpz_t p1; mpz_init(p1);	mpz_set(p1, p);
	mpz_sub_ui(p1, p1, 1);
	// (q-1)
	mpz_t q1; mpz_init(q1);	mpz_set(q1, q);
	mpz_sub_ui(q1, q1, 1);
	// (p-1) * (q-1)
	mpz_t p1_q1;
	mpz_init(p1_q1);
	mpz_mul(p1_q1, p1, q1);

	int flag = mpz_invert(e, d, p1_q1);
	if(flag == 0) printf("Multiplicative inverse does not exist for given d.\n");

	// clean up
	mpz_clear(p1);
	mpz_clear(q1);	
	mpz_clear(p1_q1);

}

// d is a random large integer which is relatively prime to (p-1)(q-1) ; gcd(d, (p-1)*(q-1)) = 1
void get_d(mpz_t d, mpz_t p, mpz_t q) {

	// calculate (p-1) * (q-1)
	
	// (p-1)
	mpz_t p1;
	mpz_init(p1);
	mpz_set(p1, p);
	mpz_sub_ui(p1, p1, 1);
	// (q-1)
	mpz_t q1;
	mpz_init(q1);
	mpz_set(q1, q);
	mpz_sub_ui(q1, q1, 1);
	// (p-1) * (q-1)
	mpz_t p1_q1;
	mpz_init(p1_q1);
	mpz_mul(p1_q1, p1, q1);
	while(1) 
	{
		mpz_set_ui(d, 0); // reset value to 0

		char intStr[NUM_DIGITS_P + 1]; // +1 for null terminator
		get_rand_intStr(intStr, NUM_DIGITS_P);
		
		int flag = mpz_set_str(d, intStr, 10);
		assert(flag == 0); // non-zero = fail	
		
		mpz_t gcd;
		mpz_init(gcd);

		// calculate the gcd
		mpz_gcd(gcd, d, p1_q1);	
		int result = mpz_cmp_ui(gcd, 1);
		
		// clean up
		mpz_clear(gcd);

		if(result != 0) continue; // the gcd does not equal 1
		else break;
	}
	// clean up
	mpz_clear(p1);
	mpz_clear(q1);	
	mpz_clear(p1_q1);
}

char* int_to_msg(uint64_t* blocks, int num_blocks) // Decryption
{
	int num_chars = num_blocks * CHARS_PER_BLOCK; // 8 characters per block
	char* msg = (char*) malloc(num_chars);
	memset(msg, 0, num_chars);

	for(int i=0; i<num_blocks; i++) {
		uint64_t block = blocks[i];
		for(int c=(CHARS_PER_BLOCK-1); c>=0; c--) {
			uint64_t mask = 0xFF;
			mask = mask << (c*8);
			unsigned char msg_char = ((block & mask) >> (c*8));	
			int idx = i*CHARS_PER_BLOCK + (CHARS_PER_BLOCK - (c+1));
			msg[idx] = msg_char;
		}
	}

	return msg;
}

// each block is contained in a uint64_t integer
uint64_t* msg_to_int(char* msg, int* num_blocks) //Encryption
{ // msg needs to be less than p*q = n

	int num_chars = strlen(msg);
	*num_blocks = ceil( (float)num_chars/CHARS_PER_BLOCK );

	uint64_t* blocks = (uint64_t*) malloc((*num_blocks) * sizeof(uint64_t));

	for(int i=0; i<(*num_blocks); i++) 
	{
		uint64_t val = 0;
		for(int c=(CHARS_PER_BLOCK-1); c>=0; c--) 
		{ 
			int idx = i*CHARS_PER_BLOCK + c;
			if(idx >= num_chars) continue;	
			//printf("idx=%i, char %c\n", idx, msg[idx]);
			for(int b=0; b<8; b++) 
			{ 
				// for each bit
				uint64_t bit_val = msg[idx] & (1 << b);
				if(bit_val > 0) bit_val = 1;
				uint64_t mask = (bit_val << (((CHARS_PER_BLOCK-1)-c)*8 + b));
				//print_bits64(mask);
				val |= mask;
			}	
		}
		blocks[i] = val;
	}
	return blocks;
} 

void mpz_print(char* name, mpz_t n) 
{
	printf("%s = ", name);
	mpz_out_str(stdout, 10, n);	// print in base 10
	printf(" (%i bits, %i digits)\n", get_num_bits(n), get_num_digits(n));
}

// generates a 100-digit prime number, and sets it to p
void get_rand_prime(mpz_t p) // gcd(d, (p-1)*(q-1)=1)
{

	// initialize random number generators	
	srand(time(NULL));
	gmp_randstate_t state;
	gmp_randinit_default(state);

	// the random prime to generate	; this is set to p at the end
	mpz_t b;
	mpz_init(b);

	// random integer to test against n
	mpz_t a;
	mpz_init(a);	
	
	mpz_t gcd;
	mpz_init(gcd);

	mpz_t jacobi_sym;
	mpz_init(jacobi_sym); 

	// the value to compare the calculated Jacobi symbol to
	mpz_t j_ab;
	mpz_init(j_ab);
	
    mpz_t exp;
	mpz_init(exp);

	char intStr[NUM_DIGITS_P + 1]; // string representation of int ; +1 null-terminator
	uint64_t tries = 0;
	while(1) 
	{
		tries++;
	
		mpz_set_ui(b, 0); // reset value to 0
		
		// generate random digits
		get_rand_intStr(intStr, NUM_DIGITS_P);
		
		// make integer odd
		if(intStr[NUM_DIGITS_P-1] < ASCII_9) {
			char ls_int = intStr[NUM_DIGITS_P-1]; // least significant int
			if(ls_int % 2 == 0) intStr[NUM_DIGITS_P-1]++;	
		}

		// create int
		int flag = mpz_set_str(b, intStr, 10);
		assert(flag == 0); // non-zero = fail	
	
		char is_prime = 1;
		// Check if b is prime (Solovay and Strassen algorithm)
		// generate random number a, 100 times ; if prime test passes for all 100 numbers, b is highly likely to be prime
		for(int i=0; i<NUM_DIGITS_P; i++) 
		{
			mpz_urandomm(a, state, b); // generates a random number between 0-b
	
			// calculate the gcd
			mpz_gcd(gcd, a, b);	
			int result = mpz_cmp_ui(gcd, 1);
			if(result != 0) 
			{ // the gcd does not equal 1
				is_prime = 0;
				break;
			}

			// calculate the Jacobi symbol J(a,b)
			int j = mpz_jacobi(a, b); // b must be odd, which is ensured above
			mpz_set_ui(jacobi_sym, j); // 1 or -1	
			// calculate the exponent (b-1)/2 
			mpz_set(exp, b);
			mpz_sub_ui(exp, exp, 1);
			mpz_cdiv_q_ui(exp, exp, 2);	
			mpz_powm(j_ab, a, exp, b); // a^{(b-1)/2} mod b
			
            result = mpz_cmp(jacobi_sym, j_ab);
			if(result != 0) 
			{
				is_prime = 0;
				break;
			}
		}
	
		if(is_prime) break; 
	}
	
	// b is prime! set to p
	mpz_set(p, b);
	
	// clean up to prevent memory leaks
	mpz_clear(b);
	mpz_clear(a);
	mpz_clear(gcd);
	mpz_clear(jacobi_sym);
	mpz_clear(j_ab);
	mpz_clear(exp);
	gmp_randclear(state);
}

void get_p_q(mpz_t p, mpz_t q) {
    while(1) {
		get_rand_prime(p);	
		get_rand_prime(q);
	
		mpz_t pq;
		mpz_init(pq);
		mpz_mul(pq, p, q);
		int num_bits = get_num_bits(pq);	
		mpz_clear(pq);
		if(num_bits != 64) continue; // 64-bit blocks

		int result = mpz_cmp(p, q);
		if(result == 0) continue; // must be different primes 
		else {		
            break;
		}
	}
}

uint64_t* p_rsa(int mode, uint64_t* blocks, int num_blocks, mpz_t e_or_d, mpz_t n) 
{
	uint64_t* cipher_blocks = (uint64_t*) malloc(num_blocks * sizeof(uint64_t));

    int num_threads = omp_get_num_procs();
    omp_set_num_threads(num_threads); 

    #pragma omp parallel for
    for(int i=0; i<num_blocks; i++) 
	{
		// convert block to mpz_t
		uint64_t* block_uint64 = &blocks[i];
		mpz_t block;
		mpz_init(block);
		uint64_to_mpz(block_uint64, block);
		// exponentiation by repeated squaring/multiplying
		mpz_t C;
		mpz_init(C);
		mpz_set_ui(C, 1);

		// go through each bit in e
		for(int b=get_num_bits(e_or_d); b>=0; b--) 
		{
			mpz_powm_ui(C, C, 2, n); // C^2 mod n
			int bit_val = mpz_tstbit(e_or_d, b);
			if(bit_val) { // b == 1
				mpz_mul(C, C, block); // C*M
				mpz_mod(C, C, n); // C*M mod n
			}
		
		}
		cipher_blocks[i] = mpz_to_uint64(C); // C is the encrypted form of the block

		mpz_clear(block);
		mpz_clear(C);
	}
	return cipher_blocks;
}

void p_get_rand_prime(mpz_t p) {

	// initialize random number generators	
	srand(time(NULL));
	gmp_randstate_t state;
	gmp_randinit_default(state);

	// the random prime to generate	; this is set to p at the end
	mpz_t b;
	mpz_init(b);

	char intStr[NUM_DIGITS_P + 1]; // string representation of int ; +1 null-terminator
	uint64_t tries = 0;
	while(1) {
		tries++;	
		mpz_set_ui(b, 0); // reset value to 0
		
		// generate random digits
		get_rand_intStr(intStr, NUM_DIGITS_P);
		//p_get_rand_intStr(intStr, NUM_DIGITS_P); // might be overkill
	
		// make integer odd
		if(intStr[NUM_DIGITS_P-1] < ASCII_9) {
			char ls_int = intStr[NUM_DIGITS_P-1]; // least significant int
			if(ls_int % 2 == 0) intStr[NUM_DIGITS_P-1]++;	
		}

		// create int
		int flag = mpz_set_str(b, intStr, 10);
		assert(flag == 0); // non-zero = fail	
		// Check if b is prime (Solovay and Strassen algorithm)
		// generate random number a, NUM_DIGITS_P times ; if prime test passes for all iterations, b is highly likely to be prime
		uint64_t prime_test = 0; // bit map ; if not 0 after this loop, failed prime test
      
        #pragma omp parallel
        {

     		// random integer to test against n
    		mpz_t a;
    		mpz_init(a);	
    		
    		mpz_t gcd;
    		mpz_init(gcd);
    
    		mpz_t jacobi_sym;
    		mpz_init(jacobi_sym); 
    
    		// the value to compare the calculated Jacobi symbol to
    		mpz_t j_ab;
    		mpz_init(j_ab);
    		
            mpz_t exp; 
    		mpz_init(exp);
           
            #pragma omp for
            for(int i=0; i<NUM_DIGITS_P; i++) {
		    	if(prime_test != 0) continue; // that's a waste...

                mpz_urandomm(a, state, b); // generates a random number between 0-b
		    	// calculate the gcd
		    	mpz_gcd(gcd, a, b);	
		    	int result = mpz_cmp_ui(gcd, 1);
		    	if(result != 0) { // the gcd does not equal 1
		    		prime_test |= (1 << i);
		    		continue;
		    	}
		    	// calculate the Jacobi symbol J(a,b)
		    	int j = mpz_jacobi(a, b); // b must be odd, which is ensured above
		    	mpz_set_ui(jacobi_sym, j); // 1 or -1
	
		    	// calculate the exponent (b-1)/2 
		    	mpz_set(exp, b);
		    	mpz_sub_ui(exp, exp, 1);
		    	mpz_cdiv_q_ui(exp, exp, 2);

	
		    	mpz_powm(j_ab, a, exp, b); // a^{(b-1)/2} mod b
		    	result = mpz_cmp(jacobi_sym, j_ab);
		    	if(result != 0) {
                    prime_test |= (1 << i);
		    		continue;
		    	}
		    }
            mpz_clear(a);
		    mpz_clear(gcd);
		    mpz_clear(jacobi_sym);
		    mpz_clear(j_ab);
		    mpz_clear(exp);

        } // #pragma omp parallel
		if(prime_test == 0) break; // is a prime	
		
	} // while(1) ; end
	
	// b is prime! set to p
	mpz_set(p, b);
	
	// clean up to prevent memory leaks
	mpz_clear(b);
	gmp_randclear(state);
}

void p_get_p_q(mpz_t p, mpz_t q) 
{
    while(1) {
		p_get_rand_prime(p);	
		p_get_rand_prime(q);
	
		mpz_t pq;
		mpz_init(pq);
		mpz_mul(pq, p, q);
		int num_bits = get_num_bits(pq);	
		mpz_clear(pq);
		if(num_bits != 64) continue; // 64-bit blocks

		int result = mpz_cmp(p, q);
		if(result == 0) continue; // must be different primes 
		else break;
	}
}

// each block is contained in a uint64_t integer
uint64_t* p_msg_to_int(char* msg, int* num_blocks) 
{ 
	// msg needs to be less than p*q = n
	int num_chars = strlen(msg);
	*num_blocks = ceil( (float)num_chars/CHARS_PER_BLOCK );

	uint64_t* blocks = (uint64_t*) malloc((*num_blocks) * sizeof(uint64_t));

    int num_threads = omp_get_num_procs();
    omp_set_num_threads(num_threads); 
    #pragma omp parallel for
	for(int i=0; i<(*num_blocks); i++) {
		uint64_t val = 0;
		for(int c=(CHARS_PER_BLOCK-1); c>=0; c--) { 
			int idx = i*CHARS_PER_BLOCK + c;
			if(idx >= num_chars) continue;	
			for(int b=0; b<8; b++) { // for each bit
				uint64_t bit_val = msg[idx] & (1 << b);
				if(bit_val > 0) bit_val = 1;
				uint64_t mask = (bit_val << (((CHARS_PER_BLOCK-1)-c)*8 + b));
				val |= mask;
			}	
		}
		
		blocks[i] = val;
	}

	return blocks;
} 