#ifndef __RWLOCK_H__
#define __RWLOCK_H__
#include<semaphore.h>

class RWLock{
private:
<<<<<<< Updated upstream
  int AR, WR, AW, WW;
  pthread_cond_t okToRead, okToWrite;
  pthread_mutex_t lock;
=======
    int AR, WR, AW, WW;
    pthread_cond_t okToRead, okToWrite;
    pthread_mutex_t lock;
>>>>>>> Stashed changes
public:
    RWLock();
    ~RWLock();
    //Reader
    void startRead();
    void doneRead();
    // Writer
    void startWrite();
    void  doneWrite();
};

#endif
