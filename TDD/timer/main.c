/**************************************************

file: main.c
purpose: simple demo that demonstrates the timer-function

**************************************************/



#include <stdio.h>
#include <stdlib.h>

#include "timer.h"





void timer_handler(void);

int var=0;


int main(void)
{
  if(start_timer(1000, &timer_handler))
  {
    printf("\n timer error\n");
    return(1);
  }

  printf("\npress ctl-c to quit.\n");

  while(1)
  {
    if(var > 5)
    {
      break;
    }
  }

  stop_timer();

  return(0);
}




void timer_handler(void)
{
  printf("timer: var is %i\n", var++);
}





