#include <algorithm>
#include <cassert>
#include <limits>

#include "flatinvmap.h"

using ElemT = std::pair<int, int>;

FlatInvMap::FlatInvMap(const int arr[], const int size):
    size(size), invarr(new ElemT[size]) {
    for (auto i = 0; i < size; ++i) {
        invarr[i] = {arr[i], i};
    }
    std::sort(invarr, invarr + size);
}

ElemT* FlatInvMap::cend() const {
    return nullptr;
}

ElemT* FlatInvMap::find(const int key) const {
    return find_in_range(key, invarr, invarr + size);
}

ElemT* FlatInvMap::find_in_range(const int key, ElemT* first, ElemT* last) const {
    while (last - first > 0) {
        const auto middle = first + (last - first) / 2;
        if (middle->first < key) {
            first = middle + 1;
        } else if (middle->first > key) {
            last = middle;
        } else {
            return middle;
        }
    }
    return nullptr;
}

FlatBFInvMap::FlatBFInvMap(const int arr[], const int size):
    FlatInvMap(arr, size), bf(get_params_for(size, 0.1)) {
    for (auto i = 0; i < size; ++i) {
        bf.insert(arr[i]);
    }
}

ElemT* FlatBFInvMap::find(const int key) const {
    if (bf.contains(key)) {
        return FlatInvMap::find(key);
    }
    return cend();
}

FlatBSInvMap::FlatBSInvMap(const int arr[], const int size):
    FlatInvMap(arr, size), table(100000) {
    for (auto i = 0; i < size; ++i) {
        table[arr[i]] = true;
    }
}

ElemT* FlatBSInvMap::find(const int key) const {
    if (table[key]) {
        return FlatInvMap::find(key);
    }
    return cend();
}

FlatSummarizerInvMap::FlatSummarizerInvMap(const int arr[], const int size):
    FlatInvMap(arr, size), step(std::cbrt(size)),
    summary_size((size + step - 1) / step + 1),
    summary(new int[summary_size]) {
    for (auto i = 0, j = 0; i < size; i += step, ++j) {
        summary[j] = invarr[i].first;
    }
    summary[summary_size - 1] = std::numeric_limits<int>::max();
}

ElemT* FlatSummarizerInvMap::find(const int key) const {
    auto first = 0;
    auto last = summary_size;
    while (last - first > 1) {
        const auto middle = first + (last - first) / 2;
        if (summary[middle] <= key) {
            first = middle;
        } else {
            last = middle;
        }
    }

    assert(first * step < size);
    return FlatInvMap::find_in_range(
        key,
        invarr + first * step, 
        invarr + std::min(size, last * step)
    );
}
