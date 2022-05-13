/*
 * @Author: your name
 * @Date: 2021-04-07 11:12:42
 * @LastEditTime: 2021-07-13 14:08:59
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /sto/sto-core/Interface.hh
 */
#pragma once
#include <cstdint>
#include <cassert>
#include <iostream>
#include <random>

#include "config.h"
#include "compiler.hh"

class Transaction;
class TransItem;

class TObject {
public:
    virtual ~TObject() = default;

    virtual bool lock(TransItem& item, Transaction& txn) = 0;
    virtual bool check_predicate(TransItem& item, Transaction& txn, bool committing) {
        (void) item, (void) txn, (void) committing;
        // always_assert(false);
        return false;
    }
    virtual bool check(TransItem& item, Transaction& txn) = 0;
    virtual void install(TransItem& item, Transaction& txn) = 0;
    virtual void unlock(TransItem& item) = 0;
    virtual void cleanup(TransItem& item, bool committed) {
        (void) item, (void) committed;
    }
    virtual void print(std::ostream& w, const TransItem& item) const;
};

typedef TObject Shared;
