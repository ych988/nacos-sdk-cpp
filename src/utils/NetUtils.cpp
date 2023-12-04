#include "NetUtils.h"
#include <sys/types.h>
#ifdef __MINGW32__
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#endif
#include <errno.h>
#include <src/log/Logger.h>
#include <string.h>
#include <unistd.h>

#define HOST_AND_LEN 250

namespace nacos{

#if defined(__MINGW32__)
template<typename FUN>
void for_each_netAdapter_win32_nacos(FUN && fun) { //type: PIP_ADAPTER_INFO
    unsigned long nSize = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO adapterList = (PIP_ADAPTER_INFO)new char[nSize];
    int nRet = GetAdaptersInfo(adapterList, &nSize);
    if (ERROR_BUFFER_OVERFLOW == nRet) {
        delete[] adapterList;
        adapterList = (PIP_ADAPTER_INFO)new char[nSize];
        nRet = GetAdaptersInfo(adapterList, &nSize);
    }
    auto adapterPtr = adapterList;
    while (adapterPtr && ERROR_SUCCESS == nRet) {
        if (fun(adapterPtr)) {
            break;
        }
        adapterPtr = adapterPtr->Next;
    }
    //释放内存空间
    delete[] adapterList;
}

bool check_ip_nacos(NacosString &address, const NacosString &ip) {
    if (ip != "127.0.0.1" && ip != "0.0.0.0") {
        /*获取一个有效IP*/
        address = ip;
        uint32_t addressInNetworkOrder = htonl(inet_addr(ip.data()));
        if (/*(addressInNetworkOrder >= 0x0A000000 && addressInNetworkOrder < 0x0E000000) ||*/
            (addressInNetworkOrder >= 0xAC100000 && addressInNetworkOrder < 0xAC200000) ||
            (addressInNetworkOrder >= 0xC0A80000 && addressInNetworkOrder < 0xC0A90000)) {
            //A类私有IP地址：
            //10.0.0.0～10.255.255.255
            //B类私有IP地址：
            //172.16.0.0～172.31.255.255
            //C类私有IP地址：
            //192.168.0.0～192.168.255.255
            //如果是私有地址 说明在nat内部

            /* 优先采用局域网地址，该地址很可能是wifi地址
             * 一般来说,无线路由器分配的地址段是BC类私有ip地址
             * 而A类地址多用于蜂窝移动网络
             */
            return true;
        }
    }
    return false;
}
#endif //defined(_WIN32)

NacosString NetUtils::getHostIp() NACOS_THROW(NacosException){
#ifdef __MINGW32__
    NacosString address = "127.0.0.1";
    for_each_netAdapter_win32_nacos([&](PIP_ADAPTER_INFO adapter) {
        IP_ADDR_STRING *ipAddr = &(adapter->IpAddressList);
        while (ipAddr) {
            NacosString ip = ipAddr->IpAddress.String;
            if (strstr(adapter->AdapterName, "docker")) {
                return false;
            }
            if(check_ip_nacos(address,ip)){
                return true;
            }
            ipAddr = ipAddr->Next;
        }
        return false;
    });
    return address;
#else
    struct ifaddrs *ifaddr, *ifa;
    int s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Failed to get IF address");
    }


    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        log_debug("iterating on iface=%s\n", ifa->ifa_name);
        if (ifa->ifa_addr == NULL || !(ifa->ifa_addr->sa_family==AF_INET)) {
            continue;
        }

        if((strcmp(ifa->ifa_name,"lo")==0)) {
            continue;
        }

        s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if (s != 0) {
            freeifaddrs(ifaddr);
            throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Failed to get IF address");
        }

        log_debug("selected iface=%s ip=%s\n", ifa->ifa_name, host);
        freeifaddrs(ifaddr);
        return host;
    }
#endif
    //Usually the program will not run to here
    throw NacosException(NacosException::UNABLE_TO_GET_HOST_IP, "Failed to get IF address");
}

NacosString NetUtils::getHostName() NACOS_THROW(NacosException)
{
    char hostname[HOST_AND_LEN];
    
    int res = gethostname(hostname, HOST_AND_LEN);
    if (res == 0) {
        return NacosString(hostname);
    }

    throw NacosException(NacosException::UNABLE_TO_GET_HOST_NAME, "Failed to get hostname, errno = " + NacosStringOps::valueOf(errno));
}

}//namespace nacos
