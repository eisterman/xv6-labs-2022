#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("Insert number of ticks");
    return 1;
  }
  int ticks = atoi(argv[1]);
  sleep(ticks);
  return 0;
}
