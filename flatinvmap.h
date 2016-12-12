#ifndef __INVFLATMAP_H_INCLUDED__
#define __INVFLATMAP_H_INCLUDED__

#include <utility>

#include "bloom_filter.h"

using ElemT = std::pair<int, int>;

class FlatInvMap {
public:
    FlatInvMap() = default;

    FlatInvMap(const int arr[], int size);
    virtual ~FlatInvMap() = default;

    FlatInvMap(const FlatInvMap&) = delete;
    FlatInvMap& operator=(const FlatInvMap&) = delete;

    FlatInvMap(FlatInvMap&&) = default;
    FlatInvMap& operator=(FlatInvMap&&) = default;

    ElemT* cend() const;
    ElemT* find(int) const;

protected:
    ElemT* find_in_range(int, ElemT*, ElemT*) const;

    int size = 0;
    ElemT* invarr = nullptr;
};

class FlatBFInvMap: public FlatInvMap {
public:
    FlatBFInvMap(const int arr[], int size);
    virtual ~FlatBFInvMap() = default;

    FlatBFInvMap(const FlatBFInvMap&) = delete;
    FlatBFInvMap& operator=(const FlatBFInvMap&) = delete;

    FlatBFInvMap(FlatBFInvMap&&) = default;
    FlatBFInvMap& operator=(FlatBFInvMap&&) = default;

    ElemT* find(int) const;

private:
    BloomFilter bf;
};

class FlatBSInvMap: public FlatInvMap {
public:
    FlatBSInvMap() = default;

    FlatBSInvMap(const int arr[], int size);
    virtual ~FlatBSInvMap() = default;

    FlatBSInvMap(const FlatBSInvMap&) = delete;
    FlatBSInvMap& operator=(const FlatBSInvMap&) = delete;

    FlatBSInvMap(FlatBSInvMap&&) = default;
    FlatBSInvMap& operator=(FlatBSInvMap&&) = default;

    ElemT* find(int) const;

private:
    std::vector<bool> table;
};

class FlatSummarizerInvMap: public FlatInvMap {
public:
    FlatSummarizerInvMap() = default;

    FlatSummarizerInvMap(const int arr[], int size);
    virtual ~FlatSummarizerInvMap() = default;

    FlatSummarizerInvMap(const FlatSummarizerInvMap&) = delete;
    FlatSummarizerInvMap& operator=(const FlatSummarizerInvMap&) = delete;

    FlatSummarizerInvMap(FlatSummarizerInvMap&&) = default;
    FlatSummarizerInvMap& operator=(FlatSummarizerInvMap&&) = default;

    ElemT* find(int) const;

private:
    int step = 0;
    int summary_size = 0;
    int* summary = nullptr;
};
#endif // __INVFLATMAP_H_INCLUDED__
