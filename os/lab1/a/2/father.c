#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
  printf("\nFATHER: pid=%i ppid=%i\n", getpid(), getppid());
  if (fork() == 0)
  {
    execl("./son", "son", NULL);
  }
  printf("FATHER: finishing\n");
  return 0;
}