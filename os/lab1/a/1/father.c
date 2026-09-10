#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
  pid_t pid, ppid; int status;
  pid = getpid();
  ppid = getppid();
  printf("\nFATHER PARAM: pid=%i ppid=%i\n", pid, ppid);
  if (fork() == 0)
    execl("./son", "son", NULL);
  system("ps -ax > file");
  wait(&status);
  return 0;
}