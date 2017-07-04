#include <iostream>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include "SpeedCount.h"

namespace jin 
{
  void SpeedCount::Start()
  {
    running = true;
    th = thread(&SpeedCount::CalculateSpeed, this); // FIXME : maybe use enable_shared_from_this
  }
}


int main()
{
  jin::SpeedCount sc1("Test Speed count1");
  sc1.Start();
  while(1) 
  {
    sc1.Add();
    sc1.Add();
    sc1.Add();
    sc1.Add();
    sc1.Add();
    sc1.Add();
    sc1.Add();
    sleep(1);
  }
  return 0;
}
