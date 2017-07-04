#ifndef JIN_SPEED_COUNT_H
#define JIN_SPEED_COUNT_H

#ifndef __cplusplus

#error c++ is required

#elif __cplusplus >= 201103L

#include <thread>
#include <atomic>
using std::atomic;
using std::thread;

#else

#include <boost/atomic.hpp>
#include <boost/thread.hpp>
using boost::atomic;
using boost::thread;

#endif

#include "Util.h"

namespace jin 
{
  class SpeedCount : jin::util::Uncopyable
  {
    public:
      SpeedCount(const std::string& desc = std::string("Speed"), int itv = 5) 
        : count(0), running(false) , description(desc), interval(itv) { }

      inline void Start();

      void Stop()
      {
        running = false;
      }

      void Add()
      {
        count++;
      }

      void ShowCount() {
        std::cout << count << std::endl;
      }

      ~SpeedCount() 
      { // NOTE : make thread exit first
        if(running) {
          running = false; 
          th.join();
        }
      }

    private:

      void CalculateSpeed()
      {
        //struct timeval real_start, real_stop;
        //double real_time;
        //gettimeofday(&real_start, NULL);

        while(running)
        {
          //gettimeofday(&real_stop, NULL);
          //real_time = (double) (real_stop.tv_sec - real_start.tv_sec) + 
          //    (double) (real_stop.tv_usec - real_start.tv_usec)/ CLOCKS_PER_SEC;
          //if(real_time >= interval) {
          sleep(interval);
          std::cout << description << " " << count * 1.0 / interval << " tps"<< std::endl;
          count = 0;
          //  gettimeofday(&real_start, NULL);
          //}
        }
        std::cout << "quit..." << std::endl;
      }

    private:
      atomic<int> count;
      atomic<bool> running;
      thread th;
      std::string description;
      int interval;
  };


}

#endif
