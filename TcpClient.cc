#include <cassert>
#include <glog/logging.h>
#include "TcpClient.h"

namespace jin
{

  TcpClient::TcpClient(const std::string& ip, short port)
    : connfd(-1),
    ip(ip),
    port(port)
  {

  }

  TcpClient::~TcpClient()
  {
    if(connfd > 0)
    {
      LOG(INFO) << "Close connection [" << connfd << "]";
      close(connfd);
      connfd = -1;
    }
  }

  bool TcpClient::Connect()
  {
    if(connfd == -1)
      return (connfd = jin::util::Connect(ip.c_str(), port)) > 0;
    return true;
  }

  void TcpClient::Close()
  {
    if(connfd > 0)
    {
      close(connfd);
      connfd = -1;
    }
  }

  bool TcpClient::SendAll()
  {
    return jin::util::SendAll(connfd, sendbuff.buff, sendbuff.len) == 0;
  }

  bool TcpClient::RecvAll()
  {
    uint32_t dataLen;
    if(jin::util::RecvAll(connfd, reinterpret_cast<unsigned char*>(&dataLen), sizeof dataLen) != 0)
      return false; // little endian maybe miss leading

    memcpy(recvbuff.buff, &dataLen, sizeof dataLen);
    recvbuff.len = dataLen;
    assert(dataLen <= kBuffSize - sizeof dataLen);
    return jin::util::RecvAll(connfd, recvbuff.buff + sizeof recvbuff.len, recvbuff.len) == 0;
  }

}
