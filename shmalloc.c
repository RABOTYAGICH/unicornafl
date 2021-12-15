#include "shmalloc.h"
#include <stdio.h>

long ptr2offset(void *ptr, void *shm_ptr)
{
    if(ptr == NULL) return -1;
    return ptr - shm_ptr;
}

void *offset2ptr(long offset, void *shm_ptr)
{
    if(offset == -1) return NULL;
    return (char *)shm_ptr + offset;
}
void initialize_header(Header *h, size_t size, int id, unsigned char is_first)
{
    //Sanity check
    if(h == NULL)
        return;

    h->prev = -1;
    h->next = -1;
    h->size = size;
    h->refcount = 0;
    h->id = id;
    h->is_free = 1;
    h->bitseq = BITSEQ;

    if(is_first) {
        h->has_mutex = 1;
        pthread_mutexattr_init(&(h->attr));
        pthread_mutexattr_setpshared(&(h->attr), PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&(h->mutex), &(h->attr));
    }
    else
    {
        h->has_mutex = 0;
    }
}


void *shmalloc(int id, size_t *size, void *shmptr, size_t shm_size, bool new, int index,struct uc_struct* uc)
{

    Header *first, *curr, *best_fit;
    size_t free_size, best_block_size;
    int one, two;
    printf("%d %d %d\n", id, shm_size, new);
    printf("0x%llu\n", uc->last);

    // Verify pointers
    if (shmptr == NULL) {
        
        return NULL;
    }
    if (size == NULL) {
       
        return NULL;
    }
    if (*size == 0) {
        // Like malloc(3), passing in a size of zero returns either NULL or
        // another pointer that can be successfully passed into shmfree()
        
        return NULL;
    }
    if (*size < 0) {
       
        return NULL;
    }


    // Find the first header

    first = curr = (Header *) shmptr; best_fit = NULL;
    

        
        
    // First time calling shmalloc
    if(!first || first->bitseq != BITSEQ)
    {

        free_size = shm_size;

        initialize_header(first, free_size, id, 1);
        first->is_free = 0;
        first->refcount++;
  
        curr = (Header *)((char *)shmptr + sizeof(Header) + *size);

       
       

        initialize_header(curr, free_size - (*size), -1, 0);
        printf("86\n");

        first->next = ptr2offset(curr, shmptr);
        printf("87\n");

        curr->prev = ptr2offset(first, shmptr);
        first->index = -1;
        printf("96\n");

        return (first + 1);
    }
    else
    {
        printf("177\n");
        //Lock shared memory
        pthread_mutex_lock(&(first->mutex));

        best_block_size = -1;

        //Loop through all headers to see if id already exists
        //Also record best spot to put this new item if it does not exist
        while(curr != NULL)
        {
            if (new)
            {

                uc->last = (Header *)(shmptr+uc->last->prev);
                
                uc->last = (Header *) offset2ptr(uc->last->next, shmptr);

                uc->last->next=-1;
            }
            if (uc->last!=NULL)
            {
                printf("LAST NOT NULL\n");
                best_block_size = uc->last->size;
                best_fit = uc->last;
                break;
            }
           
            //Get size of this block
            if((curr->size < best_block_size || best_block_size == -1) && curr->size >= *size && curr->is_free == 1)
            {
                best_block_size = curr->size;
                best_fit = curr;
            }

            curr = (Header *) offset2ptr(curr->next, shmptr);
        }

        //Did not find existing entry

        if(best_fit == NULL)
        {
            //Did not find a viable chunk, failure
            
            pthread_mutex_unlock(&(first->mutex));
            return NULL;
        }

        //Found a viable chunk - use it
        if (new)
        {
            best_fit->size = shm_size;
            free_size = shm_size;      
        }
        
        else{
            free_size = best_fit->size; //Total size of chunk before next header}
        }
        best_fit->size = *size;
        best_fit->refcount = 1;
        best_fit->id = id;
        best_fit->is_free = 0;
        best_fit->index = index;
        one = free_size - (best_fit->size*2);

        printf("free_size: %d, best_fit->size: %d\n", free_size,best_fit->size);

        two = best_fit->size;
        //Check if there is enough room to make another header
        printf("one: %d, two: %d\n", one,two);
        if(one > two)
        {
            curr = (Header *) ((char *) best_fit + best_fit->size + sizeof(Header));
            initialize_header(curr, (size_t)((char *)free_size - best_fit->size - sizeof(Header)), -1, 0);
            //Adjust pointers
            curr->prev = ptr2offset(best_fit, shmptr);
            curr->next = best_fit->next;
            if(best_fit->next != -1)
            {
                ((Header *)offset2ptr(best_fit->next, shmptr))->prev = ptr2offset(curr, shmptr);
            }
            best_fit->next = ptr2offset(curr, shmptr);

        }
        else {
            printf("OUT OF MEMORY\n");
            pthread_mutex_unlock(&(first->mutex));
            uc->last = best_fit;

            return NULL;
        }

        pthread_mutex_unlock(&(first->mutex));
        uc->last = curr;
        return (best_fit + 1);
    }
}
