#include "bloom.h"

void set_bit(bloom_filter_t *B, index_t i) {
    index_t correct_int = B->table[i >> 6];
    index_t pos = i % 64;
    index_t val = (index_t)1 << pos;
    //printf("THE set bit %d %d %llu\n", i >> 6, pos, val);
    B->table[i >> 6] = correct_int | val;
}

index_t get_bit(bloom_filter_t *B, index_t i) {
    index_t correct_int = B->table[i >> 6];
    index_t pos = i % 64;
    index_t val = (index_t)1 << pos;
    //printf("The get bit %d %d %llu\n", i >> 6, pos, val);

    return (correct_int & val) >> pos;
}

index_t hash1(bloom_filter_t *B, key_t k) {
    return k % B->size;
}

index_t hash2(bloom_filter_t *B, key_t k) {
    /* https://gist.github.com/badboy/6267743 */
    k = (~k) + (k << 21);
    k = k ^ (k >> 24);
    k = (k + (k << 3)) + (k << 8); 
    k = k ^ (k >> 14);
    k = (k + (k << 2)) + (k << 4);
    k = k ^ (k >> 28);
    k = k + (k << 31);
    return k % B->size;
}

index_t hash_combo(bloom_filter_t *B, key_t k, int i) {
    return ((hash1(B, k) + i*hash2(B, k)) % B->size);
}

void bloom_init(bloom_filter_t *B, index_t size_in_bits) {
    int num_ints = ((size_in_bits>>6)+1);
    B->table = malloc(num_ints * sizeof(index_t));
    for (int i = 0; i < num_ints; i++) {
        B->table[i] = (index_t) 0;
    }
    B->size = size_in_bits;
    B->count = 0;
    return;
}

void bloom_destroy(bloom_filter_t *B) {
    free(B->table);
    //free(B);
    return;
}

int bloom_check(bloom_filter_t *B, key_t k) {
    int checked = 0;
    for (int i=0; i<N_HASHES; i++) {
        index_t to_get = hash_combo(B, k, i);
        //printf("%d %d \n", to_get, get_bit(B, to_get));
        checked += get_bit(B, to_get);
    }
    if (checked == N_HASHES) {
        return 1;
    }
    return 0;
}

void bloom_add(bloom_filter_t *B, key_t k) {
    for (int i=0; i<N_HASHES; i++) {
        index_t to_set = hash_combo(B, k, i);
        set_bit(B, to_set);
        //printf("Setting a bit %d %d\n", to_set, get_bit(B, to_set));
    }
    return;
}

int count_bits(bloom_filter_t *B) {
    int bits_set = 0;
    for (int i=0; i<B->size; i++) {
        bits_set = bits_set + get_bit(B, i);
    }
    return bits_set;
}

int *random_nums(int num) {
    printf("HERE\n");

    int seed = 124091890;
    srand(seed);
    
    int *rands = malloc(sizeof(int) * num);
    for (int i = 0; i<num; i++) {
        rands[i] = rand() % 1000000;
    }
    printf("Returning random nums\n");
    return rands;
}

void check_overlap(int* rand_one, int* rand_two) {
    bloom_filter_t *filter = (bloom_filter_t *) malloc(sizeof(bloom_filter_t));
    bloom_init(filter, 1000);

    for (int i; i<100; i++) {
        bloom_add(filter, rand_one[i]);
    }
    printf("Number of bits: %d \n", count_bits(filter));
    int overlap = 0;
    for (int i; i<100; i++) {
        overlap = overlap + bloom_check(filter, rand_two[i]);
    }
    printf("Overlap: %d \n", overlap);
    bloom_destroy(filter);
}

int main() {
    bloom_filter_t *filter;
    bloom_init(filter, 100);
    printf("Testing Bit Gets and Sets\n");
    
    printf("0 - Hash 1: %llu, Hash 2: %llu\n", hash1(filter, 0), hash2(filter, 0));
    printf("1 - Hash 1: %llu, Hash 2: %llu\n", hash1(filter, 1), hash2(filter, 1));
    printf("2 - Hash 1: %llu, Hash 2: %llu\n", hash1(filter, 2), hash2(filter, 2));
    printf("3 - Hash 1: %llu, Hash 2: %llu\n", hash1(filter, 3), hash2(filter, 3));
    printf("13 - Hash 1: %llu, Hash 2: %llu\n", hash1(filter, 13), hash2(filter, 13));
    printf("97 - Hash 1: %llu, Hash 2: %llu\n", hash1(filter, 97), hash2(filter, 97));
    
    printf("Get Bit 0: %llu\n", get_bit(filter, 0));
    printf("Get Bit 5: %llu\n", get_bit(filter, 5));
    set_bit(filter, 5);
    set_bit(filter, 57);
    printf("Get Bit 5: %llu\n", get_bit(filter, 5));
    printf("Get Bit 6: %llu\n", get_bit(filter, 6));
    printf("Get Bit 57: %llu\n", get_bit(filter, 57));
    
    bloom_destroy(filter);

    printf("------\n");
    printf("Testing Bloom Adds and Checks\n");
    
    bloom_init(filter, 100);
    printf("Bloom Check 5: %d\n", bloom_check(filter, 5));
    bloom_add(filter, 5);
    printf("Bloom Check 5: %d\n", bloom_check(filter, 5));
    printf("Bloom Check 3: %d\n", bloom_check(filter, 3));
    bloom_destroy(filter);
    
    printf("------\n");
    printf("Smoke Test\n");
    bloom_init(filter, 1000);
    for (int i = 0; i < 70; i++) {
        bloom_add(filter, i);
    }
    printf("Number of Bits set: %d\n", count_bits(filter));
    bloom_destroy(filter);
    
    
    printf("------\n");
    printf("Trade Off Test\n");
    int *rand1 = random_nums(100);
    int *rand2 = random_nums(100);
    check_overlap(rand1, rand2);    
    free(rand1);
    free(rand2);
        
    return 0;
}