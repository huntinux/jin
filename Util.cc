#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <cstdlib>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include "Util.h"

#define exit_if(r, ...)  \
    if(r) { \
        printf(__VA_ARGS__);  \
        printf("%s:%d error no: %d error msg %s\n", __FILE__, __LINE__, errno, strerror(errno));  \
        exit(1); \
    }

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

    int Connect(struct in_addr addr, unsigned short port)
    {
        struct sockaddr_in servaddr;
        bzero(&servaddr, sizeof servaddr);
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        servaddr.sin_addr = addr;

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

    int ReadFile(const char* fn, char* buff, size_t& len/*in out*/)
    {
        FILE* fp = NULL;
        if((fp = fopen(fn, "r")) == NULL) return -1;

        fseek(fp, 0, SEEK_END);
        int l = ftell(fp);
        if(len < l) { // buffer too small
            fclose(fp);
            return -2;
        }

        len = l;
        fseek(fp, 0, SEEK_SET);
        fread(buff, 1, len, fp); // FIXME check return value

        fclose(fp);
        return 0;
    }

    bool InterfaceGroOff(const string ifname)
    {
        string cmd;
        cmd = "ethtool -K " + ifname + " gro off ";
        cout << cmd << endl;
        return 0 == system(cmd.c_str());
    }

    bool AllIterfaceGroOff()
    {
#ifdef AUTO_GET_NET_NAME
        if(!GetNetInterfaceName(netName1, netName2, AppConf.Get("ifnprefix", "enp"))) return false;
#else
        netName1 = AppConf.Get("netName1", "enp5s0");
        netName2 = AppConf.Get("netName2", "enp6s0");
#endif
        return InterfaceGroOff(netName1) && InterfaceGroOff(netName2);

    }

    // Get interface name with prefix
    bool GetNetInterfaceName(string& netName1, string& netName2, const string& prefix)
    {
        struct ifaddrs* ifList = NULL;
        if (getifaddrs(&ifList) == -1)
        {
            perror("getifaddrs");
            return false;
        }

        bool net1Get = false, net2Get = false;
        struct ifaddrs* ifa = NULL;
        for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next)
        {
            if(ifa->ifa_addr->sa_family == AF_INET)
            {
                if(!memcmp(prefix.c_str(), ifa->ifa_name, prefix.length()))
                {
                    if(!net1Get) {
                        netName1 = ifa->ifa_name;
                        net1Get = true;
                    } else {
                        netName2 = ifa->ifa_name;
                        net2Get = true;
                        break;
                    }
                }
            }
        }

        freeifaddrs(ifList);
        return (net1Get && net2Get);
    }

    IfSock::IfSock(const string& n)
        : ifname(n), sfd(-1)
    {
        exit_if(ifname.length() >= IF_NAMESIZE,
                "Interface name is too long, max is %d.\n", IF_NAMESIZE);
        memset(&ifstruct, 0, sizeof ifstruct);
        strcpy(ifstruct.ifr_name, ifname.c_str());

        // create raw socket
        sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        exit_if(sfd == -1, "Create raw socket failed\n");

        // get frame send to or from this interface
        memset(&sll, 0, sizeof sll);
        sll.sll_family 	 = PF_PACKET;
        sll.sll_protocol = htons(ETH_P_ALL);

        // handle a specified interface not all interface
        int r = ioctl(sfd, SIOCGIFINDEX, &ifstruct);
        exit_if(r == -1, "ioctl SIGCGIFINDEX failed\n");
        sll.sll_ifindex = ifstruct.ifr_ifindex;

        // get interface hardware address
        r = ioctl(sfd, SIOCGIFHWADDR, &ifstruct);
        exit_if(r == -1, "ioctl SIOCGIFHWADDR failed\n");
        memcpy(sll.sll_addr, ifstruct.ifr_ifru.ifru_hwaddr.sa_data, ETH_ALEN);
        sll.sll_halen = ETH_ALEN;

        // make interface promisc mode to get all frame
        ifstruct.ifr_flags |= IFF_PROMISC;
        r = ioctl(sfd, SIOCSIFFLAGS, &ifstruct);
        exit_if(r == -1, "ioctl set %s to promisc mode failed\n", ifstruct.ifr_name);

        // bind socket with a interface
        r = bind(sfd, (struct sockaddr *) &sll, sizeof(sll));
        exit_if(r == -1, "Bind failed\n");
    }

    int IfSock::Recv(char buff[], const size_t len)
    {
        int n = recvfrom(sfd, buff, len, 0, NULL, NULL);
        if(n == -1)
        {
            if(errno == EINTR) return 0;
            else { perror("recvfrom"); return -1; }
        }
        //cout << "recv " << n << " bytes." << endl;
        return n;
    }

    int IfSock::Send(const char buff[], const size_t len)
    {
        ssize_t n = sendto(sfd, buff, len, 0, (struct sockaddr *)&sll, sizeof sll);
        if(static_cast<size_t>(n) != len) {
            cerr << "send not complete" << endl;
            return -1;
        }
        return n;
    }

}




