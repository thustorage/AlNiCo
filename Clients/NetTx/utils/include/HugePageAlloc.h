
#ifndef __HUGEPAGEALLOC_H__
#define __HUGEPAGEALLOC_H__


#include <cstdint>
#include <errno.h>
#include <sys/mman.h>
#include <memory.h>
#include <string>

#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define LOG_ERR(fmt, ...) printf(fmt, ##__VA_ARGS__)

// char *getIP(std::string s);
inline char* getIP(std::string s)
{
    struct ifreq ifr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, s.c_str(), IFNAMSIZ - 1);

    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);

    return inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
}
inline void* hugePageAlloc(size_t size)
{

    void* res = mmap(NULL, size, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS | 0x40000 | MAP_HUGETLB , -1, 0);
    if (res == MAP_FAILED) {
        void *res = mmap(NULL, size, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS  | MAP_HUGETLB , -1, 0);
        LOG_ERR("errno=%d,size = %d, ip=%s mmap failed! try another function\n", errno, size, getIP("ens2"));
        if (res == MAP_FAILED)
            printf("errno=%d,size = %d, mmap failed!\n", errno, size);
    }

    return res;
}



#endif /* __HUGEPAGEALLOC_H__ */
