
#include <stdio.h>
#include "coverage.h"
#include "shmalloc.h"


ht_original addEntry(ht_original entry, struct uc_struct* uc, bool ovf, int index)
{
    ht_original ret = {0};
    size_t dbl_sizeHt = sizeof(ht_original);
    printf("addEntry\n");
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
void start_coverage(struct uc_struct* uc)
{
   


    char *tmout_r = getenv("AFL_COVERAGE_TMOUT");
    uc->cur_size = MAX_MEM;
    uc->afl_tmout = atoi(tmout_r);;

    size_t dbl_sizeD = sizeof(ht);
    uc->addrs = (ht *) shmalloc(0, &dbl_sizeD, uc->shm_ptr, MAX_MEM,false,-1, uc);


    uc->addrs->time = time(NULL);

}

void coverage_handler(struct uc_struct* uc, int afl_idx)
{
    if (uc->afl_cov == 1)
  {
 
  
    if (uc->address!=0)
    {
         if(uc->addrs->entries[3].count==16)
        {
            ht_original ret = addEntry(uc->addrs->entries[3],uc,false,3);
            if (ret.next != NULL) 
            {
                uc->addrs->entries[3] = ret;
            }
             else  
            { 
                void *buf;
                void *shm_tmp;
                printf("OVERFLOW\n");
                Header *first, *curr_new;
                int shm_id;
                uc->size_next += MAX_MEM;
                if ((shm_id = shmget(IPC_PRIVATE, uc->size_next, 0777 | IPC_CREAT | IPC_EXCL)) == -1) {
                    printf("errno %d\n",errno );

                    fprintf(stderr, "Failed to get a shared memory segment.\n");
                    exit(EXIT_FAILURE);
                }

                printf("73\n");

                if ((shm_tmp = shmat(shm_id, NULL, 0)) == (void *) -1) {
                            printf("errno %d\n",errno );

                    fprintf(stderr, "Failed to attach to our shared memory segment.\n");
                    exit(EXIT_FAILURE);
                }
                memcpy(shm_tmp,uc->shm_ptr, uc->cur_size);
                uc->cur_size+=MAX_MEM;
                first = curr_new = (Header *) shm_tmp;
                printf("first %d\n", curr_new->index);

                curr_new = (Header *) offset2ptr(curr_new->next, shm_tmp);
                

                uc->addrs = (ht *)(first+1);

                ht_original temp = {0};
                ht_original *TMP, *TMPP;
                void* buff;
                printf("93\n");
                printf("SHOWING: hash %d %d\n", 3, uc->addrs->entries[3].count);
                ht_original *ptr = &uc->addrs->entries[3];
                while (ptr!=NULL)
                {
                    printf("PTR ADDR : 0x%llx\n", ptr);
                    for (size_t j = 0; j < ptr->count; j++)
                    {
                        printf("SHOWING: hash %d %d SHOWING: addr: 0x%llx \n",3, ptr->count, ptr->addrs[j]);

                    }
                    printf("\n");
                    ptr = ptr->next;
                    
                }
                do
                {
                    printf("%d\n", curr_new->index);

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

                } while (curr_new!=NULL&&curr_new->bitseq != 0);
                printf("114\n");

                buf = uc->shm_ptr;
                uc->shm_ptr = shm_tmp;
                ret = addEntry(uc->addrs->entries[3],uc, true,3);

                if (ret.next != NULL) 
                {
                    uc->addrs->entries[3] = ret;
                    shmdt(buf);
                }
                else
                {
                    fprintf(stderr, "Failed to get a shared memory segment.\n");
                    exit(EXIT_FAILURE);
                }
               
                while(1)
                {
                    int qq = 1;
                }
            }
        }
        uc->addrs->entries[3].addrs[uc->addrs->entries[3].count] = uc->address;  //uc->addrs->entries[65535].original.addrs[0]
        uc->addrs->entries[3].count+=1;
        uc->address=0;
    }
  }
}


void coverage_output(struct uc_struct* uc)
{
    if (uc->afl_cov==1)
    {
        printf("REACHED\n");
        /*
        for (size_t i = 0; i < 65536; i++)
        {
            if (uc->addrs->entries[i].next!=0)
            {
                printf("SHOWING: hash %d %d\n", i, uc->addrs->entries[i].count);
                ht_original *ptr = &uc->addrs->entries[i];
                while (ptr!=NULL)
                {
                    printf("PTR ADDR : 0x%llx\n", ptr);
                    for (size_t j = 0; j < ptr->count; j++)
                    {
                        printf("SHOWING: hash %d %d SHOWING: addr: 0x%llx \n",i, ptr->count, ptr->addrs[j]);

                    }
                    printf("\n");
                    ptr = ptr->next;
                    
                }
                continue;
            }
            if(uc->addrs->entries[3].count>0){
                printf("SHOWING: hash %d %d\n", i, uc->addrs->entries[i].count);
                for (size_t j = 0; j < uc->addrs->entries[i].count; j++)
                {
                    printf("SHOWING: hash %d %d SHOWING: addr: 0x%llx \n",i, uc->addrs->entries[i].count, uc->addrs->entries[i].addrs[j]);
                    //printf("SHOWING: addr: %d\n", uc->addrs->entries[i].addrs[j]);

                }
                
            }    
        }  
        */
        /*        
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
        */
    }
}