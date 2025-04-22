#if !defined(REPORT_H_)
    #define REPORT_H_

#include<stdio.h>
#include<pthread.h>

struct info{
    unsigned long users;
    unsigned long objcount;
    unsigned long totsize;
};

pthread_mutex_t lock;

static inline unsigned long getUsrCount(){
    pthread_mutex_lock(&lock);
    extern struct info report;
    long res = report.users;
    pthread_mutex_unlock(&lock);
    return res;
}

static inline void addusr(){
    extern struct info report;
    report.users++;
}

static inline void rmusr(){
    pthread_mutex_lock(&lock);
    extern struct info report;
    report.users--;
    pthread_mutex_unlock(&lock);
}

static inline void rmobj(unsigned long len){
    pthread_mutex_lock(&lock);
    extern struct info report;
    report.objcount--;
    report.totsize-=len;
    pthread_mutex_unlock(&lock);
}

static inline void addobj(unsigned long len){
    pthread_mutex_lock(&lock);
    extern struct info report;
    report.objcount++;
    report.totsize+=len;
    pthread_mutex_unlock(&lock);
}


static inline void printrep(){
    extern struct info report;
    printf("|-------------------------|REPORT|-------------------------|\n");
    printf("-User Count %ld\n",report.users);
    printf("-Objects Count %ld\n",report.objcount);
    printf("-Total Objects size %ld\n",report.totsize);
    printf("|-----------------------|END REPORT|-----------------------|\n");
}

static inline void dismiss(){
    pthread_mutex_destroy(&lock);
}

#endif