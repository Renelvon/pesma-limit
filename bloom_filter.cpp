#include <cassert>
#include <cmath>

#include "bloom_filter.h"

BloomParams get_params_for(const unsigned int elem_count, const double error_prob) {
    assert(0.0 < error_prob && error_prob < 1.0);
    constexpr auto ln2 = std::log(2);
    const auto bit_count_dbl = elem_count * std::log(error_prob) / (ln2 * ln2);
    const uint32_t bit_count = 1 << std::ilogb(- bit_count_dbl);
    const auto hash_count_dbl = bit_count * ln2 / elem_count;
    const unsigned int hash_count = std::lround(hash_count_dbl);
    return BloomParams{bit_count, hash_count};
}

BloomFilter::BloomFilter(const uint32_t size, const unsigned int hash_count = 1u):
    modulo(size - 1), hash_count(hash_count), table(size)
{ /* empty */ }


BloomFilter::BloomFilter(const BloomParams& params):
    BloomFilter(params.bit_count, params.hash_count)
{ /* empty */ }

uint32_t BloomFilter::get_size() const {
    return modulo + 1;
}

uint32_t BloomFilter::get_hash_count() const {
    return hash_count;
}

/* Hash copied from http://burtleburtle.net/bob/hash/integer.html
 * Magic constant modified for use with BF with size a power of 2.
 */
uint32_t BloomFilter::hasher1(uint32_t num) {
    num = num ^ (num >> 4);
    num = (num ^ 0xf3c1ffc0) + (num << 5);
    return num ^ (num >> 11);
}

void BloomFilter::insert(const uint32_t num) {
    table[hasher1(num) & modulo] = true;
}

bool BloomFilter::contains(const uint32_t num) const {
    return table[hasher1(num) & modulo];
}
