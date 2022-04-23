#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define SHM_KEY 52

struct shmseg {
   unsigned int seconds;
   unsigned int nanos;

   int instances[10];
};


int main(int argc, char* argv[]) {

   int shmid;
   struct shmseg *shmp;
   
   shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644|IPC_CREAT);
   if(shmid == -1) {
      perror("user shared memory");
      return 1;
   }

   shmp = shmat(shmid, NULL, 0);
   if(shmp == (void *) -1) {
      perror("user shared memory attach");
   }

   int needs[10];
   int has[10];
   srand(time(0));
   int i;
   int good;

   for(i = 0; i < 10; i++) { //determines how many of each resource the process needs
      needs[i] = (rand() % (shmp->instances[i] -0 + 1)) + 0;
   }

   for(i = 0; i < 10; i++) { //allocates resources if there is enough
      good = 0;
      do {
         if(shmp->instances[i] >= needs[i]) {
            has[i] = has[i] + needs[i];
            shmp->instances[i] = shmp->instances[i] - needs[i];
            good = 1;
         } else {
            good = 0;
         }
      } while(!good);
   }

   for(i = 0; i < 10; i++) {
      printf("process needed %i of R%i\n", needs[i], i);
      //printf("process %i needed %i of R%i\n", argv[1], needs[i], i);
   }

   printf("Releasing all resources\n");

   for(i = 0; i < 10; i++) { //releases all resources
      shmp->instances[i] = shmp->instances[i] + has[i];
      has[i] = 0;
   }

   return 0;
}
