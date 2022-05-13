
#ifndef _UTILS_H_
#define _UTILS_H_
#include "Debug.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <errno.h>
#include <libmemcached/memcached.h>
#include <memory.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;
void bindCore(uint16_t core);
char* getMac(string s);
memcached_st* connectMemcached();
bool disconnectMemcached(memcached_st* memc);

#define LOG_ERR(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif