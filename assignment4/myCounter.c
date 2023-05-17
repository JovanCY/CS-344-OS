#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define COUNT_TO 10
#define NUM_THREADS 2

// from Exploration: Synchronization for concurrent execution
//  https://canvas.oregonstate.edu/courses/1890464/pages/exploration-synchronization-for-concurrent-execution?module_item_id=22345196

// Declare mutexes and conditions
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t myCond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t myCond2 = PTHREAD_COND_INITIALIZER; // 1 to run producer, 2 to run consumer

// counter is a variable shared by all the threads
int myCount;

void *perform_work(void *argument)
{

   while (myCount < 10)
   {

      printf("CONSUMER: MyMutex Locked\n");
      fflush(stdout);
      pthread_mutex_lock(&myMutex); // Lock the mutex before checking if the buffer has data

      while (myCount % 2 == 0)
      { // Buffer is empty. Wait for signal that the buffer has data
         printf("CONSUMER: waiting on myCond2\n");
         fflush(stdout);
         pthread_cond_wait(&myCond2, &myMutex);
      }

      printf("MyCount: %d -> %d \n", myCount, myCount + 1);
      fflush(stdout);
      myCount += 1; // There's an item, get it! This will decrement int count

      printf("CONSUMER: MyMutex Unlocked\n");
      fflush(stdout);
      pthread_mutex_unlock(&myMutex); // Unlock the mutex

      printf("CONSUMER: signaling myCond1\n");
      fflush(stdout);
      pthread_cond_signal(&myCond1); // Signal that the buffer has space
   }

   return NULL;
}

int main(void)
{

   printf("PROGRAM START\n");
   fflush(stdout);

   myCount = 0;

   pthread_t consumer;

   // Start NUM_THREADS threads

   pthread_create(&consumer, NULL, perform_work, NULL);
   printf("CONSUMER THREAD CREATED\n");
   fflush(stdout);

   pthread_cond_signal(&myCond1);

   while (myCount < 10)
   {

      printf("PRODUCER: MyMutex Locked\n");
      fflush(stdout);
      pthread_mutex_lock(&myMutex); // Lock the mutex before checking if the buffer has data

      while (myCount > 0 && myCount % 2 == 1)
      { // Buffer is empty. Wait for signal that the buffer has data
         printf("PRODUCER: waiting on myCond1\n");
         fflush(stdout);
         pthread_cond_wait(&myCond1, &myMutex);
      }

      if (myCount < 10)
      { // increment count
         printf("MyCount: %d -> %d \n", myCount, myCount + 1);
         fflush(stdout);
         myCount += 1;
      }

      printf("PRODUCER: MyMutex Unlocked\n");
      fflush(stdout);
      pthread_mutex_unlock(&myMutex); // Unlock the mutex

      printf("PRODUCER: signaling myCond2\n");
      fflush(stdout);
      pthread_cond_signal(&myCond2); // Signal that the buffer has space
   }
   // Wait for the threads to terminate

   pthread_join(consumer, NULL);

   // Destroy the mutex
   pthread_mutex_destroy(&myMutex);

   printf("PROGRAM END\n");
   fflush(stdout);
   // printf("Expected value of counter = %d\n", COUNT_TO * NUM_THREADS);
   // printf("Actual value of counter = %d\n", myCount);
   exit(EXIT_SUCCESS);
}