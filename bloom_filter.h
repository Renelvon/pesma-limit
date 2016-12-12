#ifndef __BLOOM_H_INCLUDED__
#define __BLOOM_H_INCLUDED__

#include <cstdint>
#include <vector>

struct BloomParams final {
    const uint32_t bit_count;
    const unsigned int hash_count;
};

BloomParams get_params_for(unsigned int, double);

class BloomFilter final {
public:
    BloomFilter() = default;

    BloomFilter(uint32_t size, unsigned int hash_count);
    BloomFilter(const BloomParams& params);
    //BloomFilter () = default;

    BloomFilter(const BloomFilter&) = delete;
    BloomFilter operator=(const BloomFilter&) = delete;

    BloomFilter(BloomFilter&&) = default;
    BloomFilter& operator= (BloomFilter&& other) = default;

    uint32_t get_size() const;
    uint32_t get_hash_count() const;

    static uint32_t hasher1(uint32_t);

    void insert(uint32_t num);
    bool contains(uint32_t num) const;

private:
    uint32_t modulo = 0;
    unsigned int hash_count = 0;
    std::vector<bool> table{};
};

#endif // __BLOOM_H_INCLUDED__
