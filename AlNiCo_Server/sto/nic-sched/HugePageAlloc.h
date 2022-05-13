/*
 * @Date: 2020-09-21 15:55:10
 * @LastEditTime: 2021-07-13 19:40:29
 * @FilePath: /TxSys/include/common/HugePageAlloc.h
 * @Author: Li Junru
 * @LastEditors: Please set LastEditors
 */
#ifndef __HUGEPAGEALLOC_H__
#define __HUGEPAGEALLOC_H__


#include <cstdint>
#include <errno.h>
#include <sys/mman.h>
#include <memory.h>
#include <string>
#define LOG_ERR(fmt, ...) printf(fmt, ##__VA_ARGS__)


inline void* hugePageAlloc(size_t size)
{

    void* res = mmap(NULL, size, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | 0x40000 /*| MAP_HUGETLB */, -1, 0);
    if (res == MAP_FAILED) {
        void *res = mmap(NULL, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS  /*| MAP_HUGETLB */, -1, 0);
        LOG_ERR("errno=%d,size = %d, ip=%s mmap failed! try another function\n", errno, size, "ens2");
        if (res == MAP_FAILED)
            printf("errno=%d,size = %d, mmap failed!\n", errno, size);
    }

    return res;
}

#endif /* __HUGEPAGEALLOC_H__ */
