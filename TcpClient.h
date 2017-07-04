#ifndef JIN_TCP_CLIENT_H
#define JIN_TCP_CLIENT_H

#include <string>
#include "Util.h"

namespace jin
{

static const int kBuffSize = 128 * 1024; // 128KB

struct Buffer
{
	Buffer() { Clear();}
	void Clear() { len = kBuffSize; }
	uint8_t buff[kBuffSize];
	uint32_t len;
};

typedef Buffer SendBuffer;
typedef Buffer RecvBuffer;

class TcpClient : jin::util::Uncopyable
{
public:

	TcpClient(const std::string& ip, short port);
	~TcpClient();

	bool Connect();
	void Close();
	bool SendAll();
	bool RecvAll();
	void ClearBuff()
	{
		sendbuff.Clear();
		recvbuff.Clear();
	}
	const std::string GetIP() const { return ip; }
	const short GetPort() const { return port; }
	const int GetFd() const { return connfd; }

protected:
	int connfd;
	std::string ip;
	short port;
	SendBuffer sendbuff;
	RecvBuffer recvbuff;
};

}
#endif 
