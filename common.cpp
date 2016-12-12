#include <cassert>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <tuple>

#include <unistd.h>
#include <getopt.h>

#include "common.h"

argument_t parse_cli(const int argc, char* argv[]) {
    static struct option long_options[] = {
        {"N",       required_argument, nullptr, 'N'},
        {"n",       required_argument, nullptr, 'n'},
        {"K",       required_argument, nullptr, 'K'},
        {"seed",    required_argument, nullptr, 'd'},
        {"alpha",   required_argument, nullptr, 'a'},
        {"p",       required_argument, nullptr, 'p'},
        {nullptr,   0,                 nullptr, 0}
    };

    auto N = 0;
    auto n = 0;
    auto K = 0;
    auto seed = 42LL;
    auto alpha = 0.0;
    auto p = 0.0;

    auto option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "", long_options, &option_index)) != -1) {
        switch (c) {
            case 'N':
                N = std::stoi(optarg);
                break;
            case 'n':
                n = std::stoi(optarg);
                break;
            case 'K':
                K = std::stoi(optarg);
                break;
            case 'd':
                seed = std::stoll(optarg);
                break;
            case 'a':
                alpha = std::stod(optarg);
                break;
            case 'p':
                p = std::stod(optarg);
                break;
            default:
                printf ("?? getopt returned character code 0%o ??\n", c);
                std::exit(EXIT_FAILURE);
        }
    }

    assert(0 <= K && K <= n && n <= N);
    assert(0.0 < alpha);
    assert(0.0 <= p);
    assert(0.0 == alpha || 0.0 == p);
    assert(0LL < seed);

    return argument_t(N, n, K, alpha, p, seed);
}

void string_to_int_arr(const std::string& s, int mem[], int K) {
    auto cs = s.c_str();
    char *end;
    for (auto j = 0; j < K; ++j) {
        mem[j] = std::strtol(cs, &end, 10);
        cs = end;
    }
}

input_t read_instance() {
    int N, K;
    std::ios_base::sync_with_stdio(false);
    std::cin >> N >> K;
    assert(0 < K && K <= N);
    
    auto mem = new int[2 * N * K];

    std::string temp;
    std::getline(std::cin, temp);

#pragma omp parallel for ordered schedule(static, 1) \
    default(none) shared(N, K, mem, std::cin) private(temp)
    for (auto i = 0L; i < 2 * N; ++i) {
#pragma omp ordered
        std::getline(std::cin, temp);

        string_to_int_arr(temp, &mem[i * K], K);
    }

    return input_t(N, K, mem);
}

long long int full_avalanche_hash(long long int num) {
    num = (num + 0x7ed55d16) + (num << 12);
    num = (num ^ 0xc761c23c) ^ (num >> 19);
    num = (num + 0x165667b1) + (num << 5);
    num = (num + 0xd3a2646c) ^ (num << 9);
    num = (num + 0xfd7046c5) + (num << 3);
    num = (num ^ 0xb55a4f09) ^ (num >> 16);
    return num;
}

input_t create_instance(argument_t arguments) {
    int N, n, K;
    double alpha, p;
    long long int seed;
    std::tie(N, n, K, alpha, p, seed) = arguments;
    assert(0 < K);

    auto mem = new int[2 * N * K];
    const auto idx_to_weight = choose_weight_fun(N, alpha, p);

#pragma omp parallel default(none) shared(N, n, K, seed, mem)
    {
        pq_type pq;
#pragma omp for schedule(static)
        for (auto i = 0L; i < 2 * N; ++i) {
            const auto hash_seed = full_avalanche_hash(seed + i);
            sample(N, n, K, idx_to_weight, hash_seed, &mem[i * K], pq);
        }
    }
    return input_t(N, K, mem);
}

input_t get_instance(int argc, char* argv[]) {
    const auto arguments = parse_cli(argc, argv);
    const auto N = std::get<0>(arguments);
    return (N == 0) ? read_instance() : create_instance(arguments);
}

double idx_to_uniform_weight() {
    return 1.0;
}

double idx_to_inverse_weight(const double alpha, const int i) {
    return 1.0 / std::pow(i + 1, alpha);
}

double idx_to_contested_weight(const double p, const int N, const int i) {
    return (i < p * N) ? 100.0 : 1.0;
}

std::function<double(int)>
choose_weight_fun(const int N, const double alpha, const double p) {
    using namespace std::placeholders;
    if (alpha != 0.0) {
        return std::bind(idx_to_inverse_weight, alpha, _1);
    }

    if (p != 0.0) {
        return std::bind(idx_to_contested_weight, p, N, _1);
    }

    return std::bind(idx_to_uniform_weight);
}

double make_key(double weight, double u) {
    const auto key = exp(log(u) / weight);
    return key;
}

void sample(const int max_int, const int sample_size, const int max_pref,
               const std::function<double(int)>& idx_to_weight,
               const long long int seed, int mem[], pq_type& pq) {
    using uld_dist = std::uniform_real_distribution<double>;

    // Weighted Reservoir Sampling with Exponential Random Jumps
    std::mt19937_64 eng(seed);
    uld_dist dist;
    double randvar;
    double weight;
    auto cur = 0;
    for (; cur < sample_size; ++cur) {
        weight = idx_to_weight(cur);
        do {
            randvar = dist(eng);
        } while (randvar == 0.0);
        pq.emplace(make_key(weight, randvar), cur);
    }

    while (cur < max_int) {
        do {
            randvar = dist(eng);
        } while (randvar == 0.0);
        const auto Tw = pq.top().first;
        const auto Xw = log(randvar) / log(Tw);
        auto weight = 0.0;
        auto next_weight = idx_to_weight(cur);
        while (cur < max_int && weight + next_weight < Xw) {
            weight += next_weight;
            ++cur;
            next_weight = idx_to_weight(cur);
        }

        if (cur >= max_int) {
            break;
        }

        const auto tw = pow(Tw, next_weight);
        assert(0.0 < tw && tw < 1.0);
        dist.param(uld_dist::param_type(tw, 1.0));
        randvar = dist(eng);
        dist.param(uld_dist::param_type(0.0, 1.0));

        pq.pop();
        pq.emplace(make_key(next_weight, randvar), cur);
        ++cur;
    }

    for (auto i = sample_size; i-- > 0;) {
        if (i < max_pref) {
            mem[i] = pq.top().second;
        }
        pq.pop();
    }
}
bool is_root(const int rank) {
    return rank == ROOT;
}

Set pick_proposers(const int rounds, const bool flag_a, const bool flag_b) {
    return (flag_a) ? Set::set_b
         : (flag_b) ? Set::set_a
         : (sin(rounds * rounds) >= 0) ? Set::set_a
         : Set::set_b;
}
