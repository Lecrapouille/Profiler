#include "Profiler.hpp"
#include <unistd.h> // sleep

static void foo()
{
   PROFILE_FUNCTION();
   printf("%s", "foo\n");
   usleep(100000);
}

static void bar()
{
   PROFILE_FUNCTION();
   printf("%s", "bar\n");
   usleep(1000);
}

// g++ -W -Wall --std=c++11 -I.. example.cpp -o prog
int main()
{
  START_PROFILING("HelloWord", "profile.json");

  for (int i = 0; i < 10; ++i)
  {
    PROFILE_SCOPE("main_loop");
    if (i < 5) { foo(); } else { bar(); }
  }

  STOP_PROFILING();
  return 0;
}
