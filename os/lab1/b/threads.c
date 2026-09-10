#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* thread1(void* args)
{
  int cnt = 0;
  while (1)
  {
    printf("Thread 1: count = %d\n", ++cnt);
    sleep(5);
  }
  return NULL;
}

void* thread2(void* args)
{
  int cnt = 0;
  while (1)
  {
    printf("Thread 2: count = %d\n", ++cnt);
    sleep(5);
  }
  return NULL;
}

void* thread3(void* args)
{
  int cnt = 0;
  while (1)
  {
    printf("Thread 3: count = %d\n", ++cnt);
    sleep(5);
  }
  return NULL;
}

int main()
{
  pthread_t t1, t2, t3;
  pthread_create(&t1, NULL, thread1, NULL);
  pthread_create(&t2, NULL, thread2, NULL);
  pthread_create(&t3, NULL, thread3, NULL);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  pthread_join(t3, NULL);
  return 0;
}