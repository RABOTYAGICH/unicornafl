
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
        }
        uc->addrs->entries[afl_idx].addrs[uc->addrs->entries[afl_idx].count] = uc->address;  //uc->addrs->entries[65535].original.addrs[0]
        uc->addrs->entries[afl_idx].count+=1;
        uc->address=0;
    }
  }
}

void start_coverage(struct uc_struct* uc)
{
   


    printf("qqq");
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
            printf(">>> %s\n", tmout_r); 
        }
    }
}