/*
 * Add NetID and names of all project partners

Pavan Kumar Kokkiligadda : pkk46
Nicholas Chen : nhc29

 *
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void signal_handle(int signalno)
{

  printf("OMG, I was slain!\n");
  /* Step 4: Handle SIGFPE and change the stack*/
  /* Do your tricks here
   * Your goal must be to change the stack frame of caller (main function)
   * such that you get to the line after "z=x/y"*/

  size_t instruction_length = 9;
  // printf("%p",&signalno);
  int offset = 15;
  void **pc_address = (void **)((int *)&signalno + offset);
  *pc_address += instruction_length;

  /* next statement exit(1) has been commented. understand what  happens, then remove the comment and then
      execute */

  // exit(1);
}

int main(int argc, char *argv[])
{

  int x = 5, y = 0, z = 4;

  /* Step 1: Register signal handler first*/
  signal(SIGFPE, signal_handle);
  // This will generate floating point exception

  z = x / y;

  printf("LOL, I live again !!!%d\n", z);

  return 0;
}
