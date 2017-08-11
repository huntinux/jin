
#ifndef JIN_UTIL_H
#define JIN_UTIL_H

#include <string>

namespace jin 
{
    namespace util
    {
        using std::string;
        using std::ifstream;
        using std::cout;
        using std::endl;
        using std::cerr;

        ///
        /// 读取文件fn的内容到buff中
        /// @return 0 成功； -1 fopen失败; -2 buff太小
        int ReadFile(const char* fn, char* buff, size_t& len/*in out*/);

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

        int Connect(struct in_addr addr, unsigned short port);
        int Connect(const char* ip, unsigned short port);
        int SendAll(int connfd, const unsigned char *data, unsigned int data_num);
        int RecvAll(int connfd, unsigned char *data, unsigned int data_num);

        bool AllIterfaceGroOff();
        bool GetNetInterfaceName(string& netName1, string& netName2, const string& prefix);

        // trim from start (in place)
        static inline void ltrim(string &s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                        std::not1(std::ptr_fun<int, int>(std::isspace))));
        }

        // trim from end (in place)
        static inline void rtrim(string &s) {
            s.erase(std::find_if(s.rbegin(), s.rend(),
                        std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        }

        // trim from both ends (in place)
        static inline void trim(string &s) {
            ltrim(s);
            rtrim(s);
        }

        // trim from start (copying)
        static inline string ltrimmed(string s) {
            ltrim(s);
            return s;
        }

        // trim from end (copying)
        static inline string rtrimmed(string s) {
            rtrim(s);
            return s;
        }

        // trim from both ends (copying)
        static inline string trimmed(string s) {
            trim(s);
            return s;
        }

        static string GetAppPath()
        {
            char p[PATH_MAX];
            ssize_t l = readlink("/proc/self/exe", p, sizeof(p) - 1);
            if(l != -1) {
                p[l] = '\0';
                return p;
            }
            perror("readlink");
            return string("");
        }

        // 配置文件解析器，配置文件的每一行是如下格式的键值对
        // key = value
        class Conf
        {
            typedef std::map<string, string> ConfMap;
            public:
            Conf(): f(GetAppPath()), m()
            {
                if(!f.empty()) {
                    f.append(".conf");
                    cout << "Expected configure file: " << f << endl;
                    if(!Load())
                        cout << "Cann't load configure file " << f << endl;
                } else {
                    cout << "GetAppPath failed" << endl;
                }
            }

            bool Load()
            {
                string l, k, v;
                ifstream c(f.c_str());
                if(c) {
                    while(getline(c, l)) {
                        trim(l);
                        if(l.empty() || l[0] == '#')  continue;

                        std::size_t pos = l.find('=');
                        if(pos != string::npos) {
                            k = l.substr(0, pos);
                            v = l.substr(pos + 1);
                            rtrim(k); ltrim(v);
                            m[k] = v;
                        }
                    }
                    return true;
                }
                return false;
            }

            bool Save(); // TODO

            void Show()
            {
                ConfMap::iterator it;
                for(it = m.begin(); it != m.end(); it++) {
                    cout << it->first << "=" << it->second << endl;
                }
            }

            string Get(const string& k, const string& defaultValue)
            {
                if(m.count(k) == 0) m[k] = defaultValue;
                return m[k];
            }

            private:
            // No copy allow
            Conf(const Conf&);
            Conf& operator= (const Conf&);

            private:
            string f;
            ConfMap m;
        };

#define AppConf util::ConfInstance()
        static Conf& ConfInstance()
        {
            static Conf c;
            return c;
        }

        // 网络接口封装类，抓取某个interface上的以太网帧
        class IfSock
        {
            public:
                IfSock(const string& n);
                ~IfSock() { if(sfd != -1) close(sfd); }

                int Recv(char buff[], const size_t len);
                int Send(const char buff[], const size_t len);
            private:
                string ifname;
                int sfd;
                struct sockaddr_ll sll;
                struct ifreq ifstruct;
        };
    } // namespace util

}

#endif
