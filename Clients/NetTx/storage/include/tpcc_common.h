#ifndef __TPCC_COMMON_H__
#define __TPCC_COMMON_H__

#include "macros.h"
#include "util.h"
#include "Debug.h"
#include <string>
#include <vector>

using namespace nocc::util;

#define MASK_UPPER ((0xffffffffULL) << 32)

extern int g_uniform_item_dist;
extern int scale_factor;
extern int g_new_order_remote_item_pct;

extern std::string NameTokens[22];

/* TPCC constants */
constexpr inline ALWAYS_INLINE size_t NumItems() { return 100000; }
constexpr inline ALWAYS_INLINE size_t NumCustomersPerDistrict() { return 3000; }
constexpr inline ALWAYS_INLINE size_t NumDistrictsPerWarehouse() { return 10; }

inline ALWAYS_INLINE size_t NumDistrict()
{
    return tpccmapsize / (NumDistrictsPerWarehouse() * (scale_factor + 1) + 1);
}
inline int GetEndWarehouse(int partition)
{
    return (partition + 1) * scale_factor;
}

inline int GetStartWarehouse(int partition)
{
    return partition * scale_factor + 1;
}
inline int WarehouseToPartition(int wid)
{
    return (wid - 1) / scale_factor;
}

inline int GetStartWarehouse_thread(int partition, int thread_)
{
    return std::max(partition * scale_factor + (thread_ * scale_factor / MAX_THREAD) + 1, GetStartWarehouse(partition));
}
inline int GetEndWarehouse_thread(int partition, int thread_)
{

    return std::min((partition)*scale_factor + ((thread_ + 1) * scale_factor / MAX_THREAD), GetEndWarehouse(partition));
}

inline ALWAYS_INLINE uint32_t districtKeyToWare(uint64_t d_key)
{
    int did = d_key % NumDistrictsPerWarehouse();
    if (did == 0) {
        return (d_key / NumDistrictsPerWarehouse()) - 1;
    }
    return d_key / NumDistrictsPerWarehouse();
}

inline ALWAYS_INLINE uint32_t customerKeyToWare(uint64_t c_key)
{
    uint32_t upper = (uint32_t)((MASK_UPPER & c_key) >> 32);
    int did = (upper % NumDistrictsPerWarehouse());
    if (did == 0)
        return (upper / NumDistrictsPerWarehouse()) - 1;
    return upper / NumDistrictsPerWarehouse();
}

inline ALWAYS_INLINE uint32_t stockKeyToWare(uint64_t s_key)
{
    int sid = s_key % 100000;
    if (sid == 0)
        return s_key / 100000 - 1;
    return s_key / 100000;
}

inline ALWAYS_INLINE uint32_t orderKeyToWare(uint64_t o_key)
{
    return customerKeyToWare(o_key);
}

inline ALWAYS_INLINE uint32_t orderLineKeyToWare(uint64_t ol_key)
{
    uint64_t oid = ol_key / 15;
    uint32_t upper = oid / 10000000;
    int did = upper % NumDistrictsPerWarehouse();
    if (did == 0)
        return (upper / NumDistrictsPerWarehouse()) - 1;
    return upper / NumDistrictsPerWarehouse();

    return customerKeyToWare(ol_key);
    //  uint64_t upper = ol_key / 10000000
}

inline ALWAYS_INLINE uint32_t newOrderKeyToWare(uint64_t no_key)
{
    return customerKeyToWare(no_key);
}

inline ALWAYS_INLINE uint32_t newOrderUpper(uint64_t no_key)
{
    uint32_t upper = (uint32_t)((MASK_UPPER & no_key) >> 32);
    return upper;
}

inline ALWAYS_INLINE uint64_t makeDistrictKey(uint32_t w_id, uint32_t d_id)
{
    uint32_t did = d_id + (w_id * NumDistrictsPerWarehouse());
    uint64_t id = static_cast<uint64_t>(did);
    return id;
}
inline ALWAYS_INLINE uint64_t makeCustomerKey(uint32_t w_id, uint32_t d_id, uint32_t c_id)
{
    uint32_t upper_id = w_id * NumDistrictsPerWarehouse() + d_id;
    uint64_t id = static_cast<uint64_t>(upper_id) << 32 | static_cast<uint64_t>(c_id);
    return id;
}
inline ALWAYS_INLINE uint64_t makeHistoryKey(uint32_t h_c_id, uint32_t h_c_d_id,
    uint32_t h_c_w_id, uint32_t h_d_id, uint32_t h_w_id)
{
    uint32_t cid = (h_c_w_id * NumDistrictsPerWarehouse() + h_c_d_id) * 3000 + h_c_id;
    uint32_t did = h_d_id + (h_w_id * NumDistrictsPerWarehouse());
    uint64_t id = static_cast<uint64_t>(cid) << 20 | static_cast<uint64_t>(did);
    return id;
}
inline ALWAYS_INLINE uint64_t makeOrderKey(uint32_t w_id, uint32_t d_id, uint32_t o_id)
{
    uint32_t upper_id = w_id * NumDistrictsPerWarehouse() + d_id;
    uint64_t id = static_cast<uint64_t>(upper_id) << 32 | static_cast<uint64_t>(o_id);
    // assert(orderKeyToWare(id) == w_id);
    return id;
}

inline ALWAYS_INLINE uint64_t makeOrderIndex(uint32_t w_id, uint32_t d_id, uint32_t c_id, uint32_t o_id)
{
    uint32_t upper_id = (w_id * NumDistrictsPerWarehouse() + d_id) * 3000 + c_id;
    uint64_t id = static_cast<uint64_t>(upper_id) << 32 | static_cast<uint64_t>(o_id);
    return id;
}

inline ALWAYS_INLINE uint64_t makeOrderLineKey(uint32_t w_id, uint32_t d_id, uint32_t o_id, uint32_t number)
{
    uint32_t upper_id = w_id * NumDistrictsPerWarehouse() + d_id;
    uint64_t oid = static_cast<uint64_t>(upper_id) * 10000000ull + static_cast<uint64_t>(o_id);
    uint64_t olid = oid * 15 + number;
    uint64_t id = static_cast<uint64_t>(olid);
    // assert(orderLineKeyToWare(id) == w_id);
    return id;
}

inline ALWAYS_INLINE uint64_t makeStockKey(uint32_t w_id, uint32_t s_id)
{
    uint32_t sid = s_id + (w_id * 100000ull);
    uint64_t id = static_cast<uint64_t>(sid);
    // assert(stockKeyToWare(id) == w_id);
    return id;
}

inline ALWAYS_INLINE uint64_t makeNewOrderKey(uint32_t w_id, uint32_t d_id, uint32_t o_id)
{
    uint32_t upper_id = w_id * NumDistrictsPerWarehouse() + d_id;
    uint64_t id = static_cast<uint64_t>(upper_id) << 32 | static_cast<uint64_t>(o_id);
    return id;
}

/* functions related to customer index
 * These functions are defined in tpcc_loader.cc
 */
void convertString(char* newstring, const char* oldstring, int size);
bool compareCustomerIndex(uint64_t key, uint64_t bound);
uint64_t makeCustomerIndex(uint32_t w_id, uint32_t d_id, std::string s_last, std::string s_first);

/* The shared data structure used in all TPCC related classes */

static const size_t CustomerLastNameMaxSize = 5 * 3;

/* Followng pieces of codes mainly comes from Silo */
inline ALWAYS_INLINE uint32_t GetCurrentTimeMillis()
{
    // XXX(stephentu): implement a scalable GetCurrentTimeMillis()
    // for now, we just give each core an increasing number
    static __thread uint32_t tl_hack = 0;
    return ++tl_hack;
}

// utils for generating random #s and strings
inline ALWAYS_INLINE int CheckBetweenInclusive(int v, int lower, int upper)
{
    INVARIANT(v >= lower);
    INVARIANT(v <= upper);
    return v;
}

inline ALWAYS_INLINE int RandomNumber(fast_random& r, int min, int max)
{
    return CheckBetweenInclusive((int)(r.next_uniform() * (max - min + 1) + min), min, max);
}

inline ALWAYS_INLINE int NonUniformRandom(fast_random& r, int A, int C, int min, int max)
{
    return (((RandomNumber(r, 0, A) | RandomNumber(r, min, max)) + C) % (max - min + 1)) + min;
}

inline ALWAYS_INLINE int GetItemId(fast_random& r)
{
    return CheckBetweenInclusive(
        g_uniform_item_dist ? RandomNumber(r, 1, NumItems()) : NonUniformRandom(r, 8191, 7911, 1, NumItems()),
        1, NumItems());
}

inline ALWAYS_INLINE int GetCustomerId(fast_random& r)
{
    return CheckBetweenInclusive(NonUniformRandom(r, 1023, 259, 1,
                                     NumCustomersPerDistrict()),
        1, NumCustomersPerDistrict());
}

// pick a number between [start, end]
inline ALWAYS_INLINE unsigned PickWarehouseId(fast_random& r, unsigned start, unsigned end)
{
    INVARIANT(start < end);
    const unsigned diff = end - start + 1;
    if (diff == 1)
        return start;
    return r.rand_number(start, end);
}

inline size_t GetCustomerLastName(uint8_t* buf, fast_random& r, int num)
{
    const std::string& s0 = NameTokens[num / 100];
    const std::string& s1 = NameTokens[(num / NumDistrictsPerWarehouse()) % NumDistrictsPerWarehouse()];
    const std::string& s2 = NameTokens[num % NumDistrictsPerWarehouse()];
    uint8_t* const begin = buf;
    const size_t s0_sz = s0.size();
    const size_t s1_sz = s1.size();
    const size_t s2_sz = s2.size();
    NDB_MEMCPY(buf, s0.data(), s0_sz);
    buf += s0_sz;
    NDB_MEMCPY(buf, s1.data(), s1_sz);
    buf += s1_sz;
    NDB_MEMCPY(buf, s2.data(), s2_sz);
    buf += s2_sz;
    return buf - begin;
}

inline ALWAYS_INLINE size_t GetCustomerLastName(char* buf, fast_random& r, int num)
{
    return GetCustomerLastName((uint8_t*)buf, r, num);
}

inline std::string GetCustomerLastName(fast_random& r, int num)
{
    std::string ret;
    ret.resize(CustomerLastNameMaxSize);
    ret.resize(GetCustomerLastName((uint8_t*)&ret[0], r, num));
    return ret;
}

inline ALWAYS_INLINE std::string GetNonUniformCustomerLastNameLoad(fast_random& r)
{
    return GetCustomerLastName(r, NonUniformRandom(r, 255, 157, 0, 999));
}

inline ALWAYS_INLINE size_t GetNonUniformCustomerLastNameRun(uint8_t* buf, fast_random& r)
{
    return GetCustomerLastName(buf, r, NonUniformRandom(r, 255, 223, 0, 999));
}

inline ALWAYS_INLINE size_t GetNonUniformCustomerLastNameRun(char* buf, fast_random& r)
{
    return GetNonUniformCustomerLastNameRun((uint8_t*)buf, r);
}

inline ALWAYS_INLINE std::string GetNonUniformCustomerLastNameRun(fast_random& r)
{
    return GetCustomerLastName(r, NonUniformRandom(r, 255, 223, 0, 999));
}
constexpr inline ALWAYS_INLINE size_t NumDistrictsPerWarehouse_() { return MAX_THREAD/2; }
// following oltpbench, we really generate strings of len - 1...
inline std::string RandomStr(fast_random& r, uint len)
{
    // this is a property of the oltpbench implementation...
    if (!len)
        return "";

    uint i = 0;
    std::string buf(len - 1, 0);
    while (i < (len - 1)) {
        const char c = (char)r.next_char();
        // XXX(stephentu): oltpbench uses java's Character.isLetter(), which
        // is a less restrictive filter than isalnum()
        if (!isalnum(c))
            continue;
        buf[i++] = c;
    }
    return buf;
}

// RandomNStr() actually produces a string of length len
inline std::string RandomNStr(fast_random& r, uint len)
{
    const char base = '0';
    std::string buf(len, 0);
    for (uint i = 0; i < len; i++)
        buf[i] = (char)(base + (r.next() % NumDistrictsPerWarehouse()));
    return buf;
}

#endif /* __TPCC_COMMON_H__ */
