#include <minix/mthread.h>
#include "global.h"
#include "proto.h"

int mthread_barrier_init(barrier, h)
mthread_barrier_t *barrier;
int h;
{
  struct __mthread_barrier *b;

  if (barrier == NULL)
    return(EAGAIN);

  if(h <= 0)return -1;

  else if ((b = malloc(sizeof(struct __mthread_barrier))) == NULL)return(ENOMEM);

  b->hilos_ = h;
  b->hilos = 0;
  b->aux = 0;
  b->dst = 0;
  b->e = 0;

  mthread_mutex_init(&(b->mutex), NULL);
  mthread_mutex_init(&(b->mutex2), NULL);
  mthread_cond_init(&(b->cond), NULL);
  mthread_cond_init(&(b->cond2), NULL);
  *barrier = (mthread_barrier_t) b;

  return(0);
}

int mthread_barrier_update(barrier, h)
mthread_barrier_t *barrier;
int h;
{
  if (barrier == NULL){
    return(EINVAL);
  }
  if(h <= 0)return -1;
  mthread_mutex_lock(&(*barrier)->mutex2);

   if ((*barrier)->hilos != (*barrier)->hilos_ && (*barrier)->hilos != 0){
    mthread_mutex_unlock(&(*barrier)->mutex2);
    return(EBUSY);
  }
  (*barrier)->e = 3;
  (*barrier)->hilos_ = h;
  (*barrier)->e = 0;
  mthread_mutex_unlock(&(*barrier)->mutex2);
  return 0;
}

int mthread_barrier_sync(barrier)
mthread_barrier_t *barrier; 
{
  struct __mthread_barrier *b;
  b = (struct __mthread_barrier *) *barrier;

  if (b == NULL){
    return(EINVAL);
  }

  if(b->e == 0)mthread_mutex_lock(&(b->mutex));
  if(b->e == 2)return 0;

  if(b->hilos == b->hilos_ || b->e == 3){

      b->dst++;
      mthread_cond_wait(&(b->cond), &(b->mutex));

      if(b->e == 2){
        mthread_mutex_unlock(&(b->mutex));
        b->dst--;
        return 0;
      }    
      b->dst--;
  }

  b->hilos++;
  
  if(b->hilos != b->hilos_){
       mthread_cond_wait(&(b->cond2), &(b->mutex));
  }
  mthread_cond_broadcast(&(b->cond2));

  b->aux++;

  if(b->aux == b->hilos_){

      if(b->e == 0){
        mthread_cond_broadcast(&(b->cond));  
        b->hilos = b->aux = 0;
      }
      if(b->e == 1)b->e = 2;

      mthread_mutex_unlock(&(b->mutex));       
      return 0;
  }
  mthread_mutex_unlock(&(b->mutex));
  return 0;
}

int mthread_barrier_destroy(barrier)
mthread_barrier_t *barrier;
{

mthread_mutex_lock(&(*barrier)->mutex2);
  if (barrier == NULL){
    mthread_mutex_unlock(&(*barrier)->mutex2);
    return(EINVAL);
  }
  
  if ((*barrier)->hilos != (*barrier)->hilos_ && (*barrier)->hilos != 0){
    mthread_mutex_unlock(&(*barrier)->mutex2);
    return(EBUSY);
  }
  (*barrier)->e = 1;

  mthread_cond_broadcast(&(*barrier)->cond);
  mthread_cond_broadcast(&(*barrier)->cond2);

  while((*barrier)->dst != 0){
      mthread_yield();
  }
  mthread_mutex_unlock(&(*barrier)->mutex2);

  while(mthread_mutex_destroy(&(*barrier)->mutex)){
    mthread_yield();
  }

        mthread_mutex_destroy(&(*barrier)->mutex2);
        mthread_cond_destroy(&(*barrier)->cond);
        mthread_cond_destroy(&(*barrier)->cond2);
  free(*barrier);
  *barrier = NULL;

  return(0);
}
__weak_alias(pthread_barrier_sync, mthread_barrier_sync)
__weak_alias(pthread_barrier_init, mthread_barrier_init)
__weak_alias(pthread_barrier_destroy, mthread_barrier_destroy)
__weak_alias(pthread_barrier_update, mthread_barrier_update)
