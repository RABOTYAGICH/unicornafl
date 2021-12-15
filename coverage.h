#define SHM_ENV_VAR2 "__AFL_SHM_ID2"
#include <pthread.h>
#include "uc_priv.h"
#define MAX_MEM 29572904



struct ht_original
{
    unsigned long long addrs[17];
    struct ht_original *next;
    int count;

};



// Hash table structure: create with ht_create, free with ht_destroy.
struct ht {
    struct ht_original entries[65547];  // hash slots
    unsigned int length;      // number of items in hash table
    time_t time;
    int id;
    int block_id;
    struct Header* last;

};
