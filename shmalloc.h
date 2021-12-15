#include <pthread.h>
#include "uc_priv.h"



#define SHM_ID 7
#define MAX_MEM 29572904
#define ARR_SIZE 16
#define BITSEQ 536870911

typedef struct Header {
    int bitseq;
    int id;
    int refcount;
    size_t size;
    long prev, next; // offsets
    unsigned char has_mutex;
    unsigned char is_free;
    pthread_mutex_t mutex;
    pthread_mutexattr_t attr;
    int index;

} Header;

long ptr2offset(void *ptr, void *shm_ptr);

void *offset2ptr(long offset, void *shm_ptr);
void initialize_header(Header *h, size_t size, int id, unsigned char is_first);
void *shmalloc(int id, size_t *size, void *shmptr, size_t shm_size, bool new,int index,struct uc_struct* uc);
