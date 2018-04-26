#include <stdio.h>
#include <pthread.h>
#include <iostream> //


#include "rwlock.h"

RWLock::RWLock() {
  AR = 0;
  WR = 0;
  AW = 0;
  WW = 0;
  lock = PTHREAD_MUTEX_INITIALIZER;
  okToRead = PTHREAD_COND_INITIALIZER;
  okToWrite = PTHREAD_COND_INITIALIZER;
}
RWLock::~RWLock() { }
void RWLock::startRead() {
  pthread_mutex_lock(&lock);
  while ((AW + WW) > 0) {
    WR++;
    okToRead.wait(&lock);
    WR--;
  }
  AR++;
  pthread_mutex_unlock(&lock);
}

void RWLock::doneRead() {
    pthread_mutex_lock(&lock);
    AR--;
    if (AR == 0 && WW > 0){
        okToWrite.signal();
    }
    pthread_mutex_unlock(&lock);
}

void RWLock::startWrite() {
  pthread_mutex_lock(&lock);
  while ((AW + WR) > 0) {
    WW++;
    okToWrite.wait(&lock);
    WW--;
  }
  AW++;
  pthread_mutex_unlock(&lock);

}
void RWLock::doneWrite() { }
