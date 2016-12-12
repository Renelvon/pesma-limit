#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <memory>
#include <tuple>

#include <omp.h>

#include "common.h"

int main(int argc, char* argv[]) {
    int N, n, K;
    double alpha, p;
    long long int seed;
    std::tie(N, n, K, alpha, p, seed) = parse_cli(argc, argv);
    assert(0 < K);

    const auto parallelism = std::min(2 * N, omp_get_max_threads());
    auto mem = std::unique_ptr<int[]>(new int[parallelism * K]);
    const auto idx_to_weight = choose_weight_fun(N, alpha, p);

    std::ios_base::sync_with_stdio(false);
    std::ostream_iterator<int> ot(std::cout, " ");
    std::cout << N << " " << K << std::endl;

#pragma omp parallel default(none) shared(N, n, K, seed, mem, ot, std::cout)
    {
        pq_type pq;
#pragma omp for ordered schedule(static, 1)
        for (auto i = 0; i < 2 * N; ++i) {
            const long int row = omp_get_thread_num();
            const auto hash_seed = full_avalanche_hash(seed + i);
            sample(N, n, K, idx_to_weight, hash_seed, &mem[row * K], pq);
#pragma omp ordered
            {
                std::copy(&mem[row * K], &mem[(row + 1) * K], ot);
                std::cout << std::endl;
            }
        }
    }
}
