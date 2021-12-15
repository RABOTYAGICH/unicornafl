
#include <stdio.h>
#include "coverage.h"
#include "shmalloc.h"


ht_original addEntry(ht_original entry, struct uc_struct* uc, bool ovf, int index)
{
    ht_original ret = {0};
    size_t dbl_sizeHt = sizeof(ht_original);

    ht_original *tmp = (ht_original*) shmalloc(uc->addrs->block_id, &dbl_sizeHt, uc->shm_ptr, MAX_MEM,ovf,index, uc);
    if (tmp==NULL)
    {
        return ret;
    }
    uc->addrs->block_id++;

    *tmp = entry;
   
   // (Header*)((void*)tmp-0x60)index = 3;
    ret.next = tmp;
    return ret;

}

void coverage_handler(struct uc_struct* uc, int afl_idx)
{
    if (uc->afl_cov == 1)
  {
 
  
    if (uc->address!=0)
    {
     // printf("ADDRESSZ 0x%llx AFL_IDX %d\n", uc->address, afl_idx);
         if(uc->addrs->entries[afl_idx].count==16)
        {
            ht_original ret = addEntry(uc->addrs->entries[afl_idx],uc,false,afl_idx);
            if (ret.next != NULL) 
            {
                uc->addrs->entries[afl_idx] = ret;
            }
             else  
            { 
                void *buf;
                void *shm_tmp;

                Header *first, *curr_new;
                int shm_id;
                uc->size_next += MAX_MEM;
                if ((shm_id = shmget(IPC_PRIVATE, uc->size_next, 0777 | IPC_CREAT | IPC_EXCL)) == -1) {
                    printf("errno %d\n",errno );

                    fprintf(stderr, "Failed to get a shared memory segment.\n");
                    exit(EXIT_FAILURE);
                }


                if ((shm_tmp = shmat(shm_id, NULL, 0)) == (void *) -1) {
                            printf("errno %d\n",errno );

                    fprintf(stderr, "Failed to attach to our shared memory segment.\n");
                    exit(EXIT_FAILURE);
                }
                memcpy(shm_tmp,uc->shm_ptr, uc->cur_size);
                uc->cur_size+=MAX_MEM;
                first = curr_new = (Header *) shm_tmp;

                curr_new = (Header *) offset2ptr(curr_new->next, shm_tmp);
                

                uc->addrs = (ht *)(first+1);

                ht_original temp = {0};
                ht_original *TMP, *TMPP;
                void* buff;
                
                do
                {
                    TMP = (curr_new+1);
                    if (TMP->next==NULL)
                    {
                        uc->addrs->entries[curr_new->index].next = TMP;
                    }
                    else
                    {
                        temp = uc->addrs->entries[curr_new->index];

                        TMPP = uc->addrs->entries[curr_new->index].next;
                        temp.next = TMP;
                        temp.next->next = TMPP;
                        uc->addrs->entries[curr_new->index] = temp;
                    }
                   
                    curr_new = (Header *) offset2ptr(curr_new->next, shm_tmp);
                } while (curr_new!=NULL&&curr_new->next!=-1);
                    
                
               
                
                buf = uc->shm_ptr;
                uc->shm_ptr = shm_tmp;
                ret = addEntry(uc->addrs->entries[afl_idx],uc, true,afl_idx);

                if (ret.next != NULL) 
                {
                    uc->addrs->entries[afl_idx] = ret;
                    shmdt(buf);
                }
                else
                {
                    fprintf(stderr, "Failed to get a shared memory segment.\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        uc->addrs->entries[afl_idx].addrs[uc->addrs->entries[afl_idx].count] = uc->address;  //uc->addrs->entries[65535].original.addrs[0]
        uc->addrs->entries[afl_idx].count+=1;
        uc->address=0;
    }
  }
}

void start_coverage(struct uc_struct* uc)
{
   


    char *tmout_r = getenv("AFL_COVERAGE_TMOUT");

    uc->afl_tmout = atoi(tmout_r);;

    size_t dbl_sizeD = sizeof(ht);
    uc->addrs = (ht *) shmalloc(0, &dbl_sizeD, uc->shm_ptr, MAX_MEM,false,-1, uc);


    uc->addrs->time = time(NULL);

}

void coverage_output(struct uc_struct* uc)
{
    if (uc->afl_cov==1)
    {
                    /* code */
                
                
        time_t now = time(NULL);
        if (now-uc->addrs->time>=uc->afl_tmout)
        {
            FILE* file;
            char *tmout_r = getenv("AFL_COVDIR");
            file = fopen(tmout_r, "w");    
            if (file != -1) 
            {
                for (size_t i = 0; i < 65536; i++)
                {
                    if(uc->addrs->entries[i].count>0){
                        for (size_t j = 0; j < uc->addrs->entries[i].count; j++)
                        {
                            fprintf(file,"0x%llx\n", uc->addrs->entries[i].addrs[j]);

                        }
                        
                    }
                }
            }
            uc->addrs->time = time(NULL);
        }
    }
}