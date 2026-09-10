#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
  pid_t pid = fork();
  if (pid == 0)
  {
    printf("SON: pid=%i ppid=%i\n", getpid(), getppid());
    return 0;
  }
  printf("FATHER: pid=%i ppid=%i\n", getpid(), getppid());
  printf("FATHER: son pid=%i\n", pid);
  system("ps -ax > file");
  sleep(15);
  return 0;
}