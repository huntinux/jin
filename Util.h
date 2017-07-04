
#ifndef JIN_UTIL_H
#define JIN_UTIL_H

#include <string>

namespace jin 
{
  namespace util 
  {

    class Uncopyable 
    {
      protected:
        Uncopyable(){}
        ~Uncopyable(){}
      private:
        Uncopyable(const Uncopyable&);
        Uncopyable& operator=(const Uncopyable&);
    };

    const std::string ToHexString(const void* data, size_t len, const char* message);

    int Connect(const char* ip, unsigned short port);
    int SendAll(int connfd, const unsigned char *data, unsigned int data_num);
    int RecvAll(int connfd, unsigned char *data, unsigned int data_num);
  }
}

#endif
