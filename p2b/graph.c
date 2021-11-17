#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

void spin() {
  int i = 0;
  int j = 0;
  int k = 0;
  for (i = 0; i < 500; ++i) {
    for (j = 0; j < 200000; ++j) {
      k = j % 10;
      k = k + 1;
    }
  }
}

void print(struct pstat *st) {
   int i;
   for(i = 0; i < NPROC; i++) {
     if (st->inuse[i]) {
       printf(1, "pid: %d tickets: %d ticks: %d\n", st->pid[i], st->tickets[i], st->ticks[i]);
     }
   }
}

void compare(int pid_low, int pid_high, struct pstat *before, struct pstat *after) {
  int i, ticks_low_before=-1, ticks_low_after=-1, ticks_high_before=-1, ticks_high_after=-1;
  for(i = 0; i < NPROC; i++) {
    if (before->pid[i] == pid_low) 
        ticks_low_before = before->ticks[i];
    if (before->pid[i] == pid_high)
        ticks_high_before = before->ticks[i];
    if (after->pid[i] == pid_low)
        ticks_low_after = after->ticks[i];
    if (after->pid[i] == pid_high)
        ticks_high_after = after->ticks[i];
  }
  printf(1, "high before: %d high after: %d, low before: %d low after: %d\n", 
                     ticks_high_before, ticks_high_after, ticks_low_before, ticks_low_after);
  
  if (ticks_high_after-ticks_high_before > (ticks_low_after - ticks_low_before)*100) {
    printf(1, "XV6_SCHEDULER\t SUCCESS\n"); 
  } else {
    printf(1, "XV6_SCHEDULER\t FAILED121314\n"); 
    exit();
  }
}

int
main(int argc, char *argv[])
{
  //int pid_low = getpid();
  int lowtickets = 30, hightickets = 20;

  settickets(lowtickets);

  if (fork() == 0) {  	
    settickets(hightickets);
    
    //int pid_high = getpid();
    //struct pstat st_before, st_after;
        
    //getpinfo(&st_before);
        
    //printf(1, "\n ****PInfo before**** \n");
    //print(&st_before);
    //printf(1,"Spinning...\n");

    spin();
        
    //getpinfo(&st_after);
        
    //printf(1, "\n ****PInfo after**** \n");
    //print(&st_after);
	
    //compare(pid_low, pid_high, &st_before, &st_after);
         
    exit();
  }
  if (fork() == 0) {  	
    settickets(10);
    
    //int pid_high = getpid();
    //struct pstat st_before, st_after;
        
    //getpinfo(&st_before);
        
    //printf(1, "\n ****PInfo before**** \n");
    //print(&st_before);
    //printf(1,"Spinning...\n");

    spin();
        
    //getpinfo(&st_after);
        
    //printf(1, "\n ****PInfo after**** \n");
    //print(&st_after);
	
    //compare(pid_low, pid_high, &st_before, &st_after);
         
    exit();
  }
  spin();
  while (wait() > -1);
  exit();
}
