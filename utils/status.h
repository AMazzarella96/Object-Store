#if !defined(STATUS_H_)
    #define STATUS_H_
#include<stdio.h>
#include<pthread.h>



struct stat{
    unsigned long op_count;
    unsigned long op_success;
    unsigned long op_failed;
};

static inline void inc_op_count(){
    
    extern struct stat stats;
    stats.op_count++;
    
}

static inline void inc_fail_count(){
    
    extern struct stat stats;
    stats.op_failed++;
    
}

static inline void inc_succ_count(){
    
    extern struct stat stats;
    stats.op_success++;
   
}

static inline void printstat(){
   
    extern struct stat stats;
    printf("|-------------------------|REPORT|-------------------------|\n");
    printf("-Operation count: %ld\n",stats.op_count);
    printf("\t-Succeeded: %ld\n", stats.op_success);
    printf("\t-Failed: %ld\n",stats.op_failed);
    printf("|--------------------------|END|---------------------------|\n");
    
}


#endif