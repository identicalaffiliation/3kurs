#include <stdio.h>
#include <unistd.h>

int main()
{
  pid_t pid, ppid;
  pid = getpid();
  ppid = getppid();
  printf("\nSON PARAM: pid=%i ppid=%i\n", pid, ppid);
  sleep(15);
  return 0;
}