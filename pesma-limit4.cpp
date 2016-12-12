#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iterator>
#include <mutex>
#include <vector>

#include "agent4.h"
#include "common.h"

#define EMIT_ROUND 0
#define EMIT_OUTPUT 0

void stat_rounds(const int satisfied, const double threshold, const int rounds, int& pos) {
    if (satisfied > threshold && pos == 0) {
        pos = rounds;
    }
}

int main(int argc, char* argv[]) {
    const auto tinit = std::chrono::system_clock::now();

    // Read or create input.
    const auto instance = get_instance(argc, argv);
    const auto N = std::get<0>(instance);
    const auto K = std::get<1>(instance);
    Agent::set_max_pref(K);

    const auto mem = std::get<2>(instance);

    const auto tread = std::chrono::system_clock::now();
    std::chrono::duration<double> dur = tread - tinit;
    std::cerr << "Input setup:  " << dur.count() << "s" << std::endl;

    auto agents = new Agent[2 * N];
    auto locks = new std::mutex[N];

    // Initialize agents.
#pragma omp parallel for schedule(static) default(none) shared(agents)
    for (auto i = 0; i < 2 * N; ++i) {
        agents[i] = Agent(K, &mem[i * K]);
    }

    // Prepare for DESMA.
    auto rounds = 0;
    auto flag_prp = 0;
    auto flag_acc = 0;
    auto prp_offset = 0;
    int rounds_to_percent[] = {0, 0, 0, 0, 0, 0, 0};

    const auto max_rounds = N * std::lround(std::sqrt(N)) + 1;

    const auto tstart = std::chrono::system_clock::now();
    dur = tstart - tread;
    std::cerr << "Agents setup: " << dur.count() << "s" << std::endl;
    
    #pragma omp parallel default(none) \
        shared(prp_offset, flag_acc, flag_prp, rounds, locks, agents, std::cout, rounds_to_percent)
    {
    do {
        const auto proposers = &agents[prp_offset];
        const auto acceptors = &agents[N - prp_offset];

            // Make and evaluate proposals
        #pragma omp for schedule(static)
        for (auto i = 0; i < N; ++i) {
            auto& proposer = proposers[i];
            if (proposer.is_content()) {
                continue;
            }

            const auto proposee_id = proposer.target_critical_suitor();
            auto& acceptor = acceptors[proposee_id];
            const auto fitness = acceptor.screen(i);
            if (fitness < 0) {
                continue;
            }

            const auto antagonist_id = proposer.get_suitor_id();
            {
                std::lock_guard<std::mutex> guard(locks[proposee_id]);
                acceptor.judge(i, fitness, antagonist_id);
            }
        }

        #pragma omp master
        {
#if EMIT_ROUND
            std::cout << "Round: " << rounds
                      << ((prp_offset == 0) ? "A" : "B") << std::endl;
#endif
            // Select proposing set for next round;
            const auto prp_sat = flag_prp == N;
            const auto acc_sat = flag_acc == N;
            const auto result = (prp_offset == 0)
                ? pick_proposers(rounds, prp_sat, acc_sat)
                : pick_proposers(rounds, acc_sat, prp_sat);

            prp_offset = (result == Set::set_a) ? 0 : N;
            ++rounds;
            flag_acc = 0;
            flag_prp = 0;
        }
        
        // Evaluate proposals.
        #pragma omp for schedule(static)
        for (auto i = 0; i < N; ++i) {
            auto& acceptor = acceptors[i];

            const auto crush_id = acceptor.get_crush_id();
            if (crush_id != NotAnAgentId) {
                proposers[crush_id].inform_of_acceptance();

                const auto antagonist_id = acceptor.get_antagonist_id();
                if (antagonist_id != NotAnAgentId) {
                    acceptors[antagonist_id].inform_of_divorce();
                    acceptor.clear_antagonist_id();
                }

                const auto cur_suitor_id = acceptor.get_suitor_id();
                if (cur_suitor_id != NotAnAgentId) {
                    proposers[cur_suitor_id].inform_of_divorce();
                }
            }
        }

        // Update status of each acceptor...
        #pragma omp for reduction(+:flag_acc) schedule(static) nowait
        for (auto i = 0; i < N; ++i) {
            auto& acceptor = acceptors[i];
            if (acceptor.get_crush_id() != NotAnAgentId) {
                acceptor.engage_target_proposer();
            } else if (acceptor.is_divorced()) {
                acceptor.break_up_with_current();
            }
            flag_acc += acceptor.is_content() ? 1 : 0;
        }

        // Update status of each proposer...
        #pragma omp for reduction(+:flag_prp) schedule(static)
        for (auto i = 0; i < N; ++i) {
            auto& proposer = proposers[i];
            if (proposer.is_accepted()) {
                proposer.engage_target_proposee();
            } else {
                if (proposer.get_crush_id() != NotAnAgentId) {
                    proposer.rebound_from_crush();
                }
                if (proposer.is_divorced()) {
                    proposer.break_up_with_current();
                }
            }
            flag_prp += proposer.is_content() ? 1 : 0;
        }

#pragma omp master
        {
            stat_rounds(flag_prp, N * 0.9, rounds, rounds_to_percent[0]);
            stat_rounds(flag_prp, N * 0.95, rounds, rounds_to_percent[1]);
            stat_rounds(flag_prp, N * 0.99, rounds, rounds_to_percent[2]);
            stat_rounds(flag_prp, N * 0.995, rounds, rounds_to_percent[3]);
            stat_rounds(flag_prp, N * 0.999, rounds, rounds_to_percent[4]);
            stat_rounds(flag_prp, N * 0.9995, rounds, rounds_to_percent[5]);
            stat_rounds(flag_prp, N * 0.9999, rounds, rounds_to_percent[6]);
        }
    } while (rounds < max_rounds && (flag_acc < N || flag_prp < N));
    }
    const auto tfinish = std::chrono::system_clock::now();
    dur = tfinish - tstart;
    std::cerr << "Solving:      " << dur.count() << "s" << std::endl
              << "===" << std::endl
              << "Rounds:             " << rounds << std::endl
              << "Rounds till 90%:    " << rounds_to_percent[0] << std::endl
              << "Rounds till 95%:    " << rounds_to_percent[1] << std::endl
              << "Rounds till 99%:    " << rounds_to_percent[2] << std::endl
              << "Rounds till 99.5%:  " << rounds_to_percent[3] << std::endl
              << "Rounds till 99.9%:  " << rounds_to_percent[4] << std::endl
              << "Rounds till 99.95%: " << rounds_to_percent[5] << std::endl
              << "Rounds till 99.99%: " << rounds_to_percent[6] << std::endl
              << "===" << std::endl
              << "Rounds per sec: "  << rounds / dur.count() << std::endl;

#if EMIT_OUTPUT
    // Publish preferences of set A.
    std::transform(
        agents, agents + N,
        std::ostream_iterator<int>(std::cout, " "),
        [](const Agent& agent){return agent.get_suitor_id();}
    );
    std::cout << std::endl;
#endif

    auto results = new int[2 * N]; 
    // Publish preferences of set A.
    std::transform(
        agents, agents + 2 * N,
        results,
        [](const Agent& agent){return agent.get_cur_fit();}
    );

    auto max_sats = new int[2 * N]; 
    std::transform(
        agents, agents + 2 * N,
        max_sats,
        [](const Agent& agent){return agent.get_max_sat_fit();}
    );

    // Group max sat per 10%-centiles.
    int max_sat_percentiles[11];
    std::fill(max_sat_percentiles, max_sat_percentiles + 11, 0);
    for (auto i = 0; i < 2 * N; ++i) {
        const auto percentile = std::lround(std::floor((10.0 * max_sats[i]) / K));
        assert(0 <= percentile && percentile < 11);
        ++max_sat_percentiles[percentile];
    }
    std::partial_sum(max_sat_percentiles, max_sat_percentiles + 11, max_sat_percentiles);
             
    std::cerr << "===" << std::endl
              << "Agents satisfied within 10%  of K: " << max_sat_percentiles[0] << " " << max_sat_percentiles[0] / (2.0 * N) << "%" << std::endl
              << "Agents satisfied within 20%  of K: " << max_sat_percentiles[1] << " " << max_sat_percentiles[1] / (2.0 * N) << "%" << std::endl
              << "Agents satisfied within 30%  of K: " << max_sat_percentiles[2] << " " << max_sat_percentiles[2] / (2.0 * N) << "%" << std::endl
              << "Agents satisfied within 40%  of K: " << max_sat_percentiles[3] << " " << max_sat_percentiles[3] / (2.0 * N) << "%" << std::endl
              << "Agents satisfied within 50%  of K: " << max_sat_percentiles[4] << " " << max_sat_percentiles[4] / (2.0 * N) << "%" << std::endl
              << "Agents satisfied within 60%  of K: " << max_sat_percentiles[5] << " " << max_sat_percentiles[5] / (2.0 * N) << "%" << std::endl
              << "Agents satisfied within 70%  of K: " << max_sat_percentiles[6] << " " << max_sat_percentiles[6] / (2.0 * N) << "%" << std::endl
              << "Agents satisfied within 80%  of K: " << max_sat_percentiles[7] << " " << max_sat_percentiles[7] / (2.0 * N) << "%" << std::endl
              << "Agents satisfied within 90%  of K: " << max_sat_percentiles[8] << " " << max_sat_percentiles[8] / (2.0 * N) << "%" << std::endl
              << "Agents satisfied within 100% of K: " << max_sat_percentiles[9] << " " << max_sat_percentiles[9] / (2.0 * N) << "%" << std::endl;

    // Accumulate decision costs from sets A and B
    const auto cata_f = [](int acc, const Agent& agent){
        return acc + agent.get_cur_fit();
    };
    const auto cost_a = std::accumulate(agents, agents + N, 0, cata_f);
    const auto cost_b = std::accumulate(agents + N, agents + 2 * N, 0, cata_f);

    const auto egal_cost = cost_a + cost_b;
    const auto sex_cost = std::max(cost_a, cost_b) - std::min(cost_a, cost_b);

    std::cerr << "===" << std::endl
              << "Max sat_fitness:  " << *std::max_element(max_sats, max_sats + 2 * N) << std::endl
              << "Regret cost:      " << *std::max_element(results, results + 2 * N) << std::endl
              << "Egalitarian cost: " << egal_cost << std::endl
              << "Sex-ratio cost:   " << sex_cost << std::endl;

    if (rounds == max_rounds) {
        std::cerr << "FAILURE!" << std::endl;
    }

    delete[] locks;
    delete[] agents;
    delete[] mem;
}
