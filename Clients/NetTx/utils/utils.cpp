

#include "utils.h"
void bindCore(uint16_t core)
{
    printf("%s(%d)\n", __func__, core);
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        Debug::notifyError("can't bind core!");
    }
}

char* getMac(string s)
{
    static struct ifreq ifr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, s.c_str(), IFNAMSIZ - 1);

    ioctl(fd, SIOCGIFHWADDR, &ifr);
    close(fd);
    return (char*)ifr.ifr_hwaddr.sa_data;
}
memcached_st* connectMemcached()
{
    memcached_server_st* servers = NULL;
    memcached_return rc;
    std::string memcached_ip;
    in_port_t port;
    freopen("../../memcached.conf", "r", stdin);
    std::cin >> memcached_ip >> port;
    std::cout << memcached_ip << std::endl;
    freopen("CON", "r", stdin);
    memcached_st* memc = memcached_create(NULL);
    servers = memcached_server_list_append(servers, memcached_ip.c_str(), port, &rc);
    rc = memcached_server_push(memc, servers);

    if (rc != MEMCACHED_SUCCESS) {
        fprintf(stderr, "Counld't add server:%s\n", memcached_strerror(memc, rc));
        exit(0);
        return NULL;
    }
    return memc;
}

bool disconnectMemcached(memcached_st* memc)
{
    if (memc) {
        memcached_quit(memc);
        memcached_free(memc);
        memc = NULL;
    }
    return true;
}