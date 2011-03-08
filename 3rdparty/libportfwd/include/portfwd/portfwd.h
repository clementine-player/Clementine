#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <iostream>
//fwd:
struct UPNPUrls;
struct IGDdatas;

class Portfwd
{
    public:
        Portfwd();
        ~Portfwd();
        /// timeout: milliseconds to wait for a router to respond
        /// 2000 is typically enough.
        bool init(unsigned int timeout);
        void get_status();
        bool add(unsigned short port);
        bool remove(unsigned short port);
        
        const std::string& external_ip() const 
        { return m_externalip; }
        const std::string& lan_ip() const 
        { return m_lanip; }
        unsigned int max_upstream_bps() const { return m_upbps; }
        unsigned int max_downstream_bps() const { return m_downbps; }

    protected:
        struct UPNPUrls* urls;
        struct IGDdatas* data;
        
        std::string m_lanip, m_externalip;
        unsigned int m_upbps, m_downbps;
};

