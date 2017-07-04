#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Util.h"

namespace jin
{
  namespace util
  {
    const std::string ToHexString(const void* data, size_t len, const char* message)
    {
      std::ostringstream oss;
      oss << std::setfill('0');
      if(message) oss << message;
      for(size_t i = 0; i < len; i++)
      {
        unsigned int t =
          *(static_cast<const unsigned char*>(data) + i);
        oss << std::hex << std::setw(2) << t ;
      }
      oss << std::dec << std::endl;
      return oss.str();
    }


    int SendAll(int connfd, const unsigned char *data, unsigned int data_num)
    {
      int count;
      unsigned int sendnum = 0;
      while (sendnum != data_num)
      {
        count = write(connfd, data + sendnum, data_num - sendnum);
        if (count <= 0) {
          if (count < 0 && errno == EINTR) {
            count = 0;
          } else {
            return -1;
          }
        }
        sendnum += count;
      }
      return 0;
    }

    int RecvAll(int connfd, unsigned char *data, unsigned int data_num)
    {
      int count;
      unsigned int recvnum = 0;

      while (recvnum != data_num)
      {
        count = recv(connfd, data + recvnum, data_num - recvnum, MSG_WAITALL);
        if (count <= 0) {
          printf("count=%d errno=%d %s\n",count, errno, strerror(errno));
          if (count < 0 && errno == EINTR) {
            count = 0;
          } else {
            return -1;
          }
        }
        recvnum += count;
      }
      return 0;
    }

    int Connect(const char* ip, unsigned short port)
    {
      struct sockaddr_in servaddr;
      bzero(&servaddr, sizeof servaddr);
      servaddr.sin_family = AF_INET;
      servaddr.sin_port = htons(port);
      int r;
      if((r = inet_pton(AF_INET, ip, &servaddr.sin_addr)) != 1)
      {
        if(r == 0) fprintf(stderr, "ip invalid:%s\n", ip);
        else perror("inet_pton");
        return -1;
      }

      int sfd = socket(AF_INET, SOCK_STREAM, 0);
      if(sfd == -1) {
        perror("socket");
        return -1;
      }

      int ret = connect(sfd, (struct sockaddr *)&servaddr, sizeof servaddr);
      if (ret == -1) {
        close(sfd);
        perror("connect");
        return -1;
      }
      return sfd;
    }

  }

}
