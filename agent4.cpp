#include "agent4.h"

int Agent::K = 0;

int Agent::target_critical_suitor() {
    crush_id = id_of_fit[sat_fit];
    return crush_id;
}

int Agent::screen(const int proposer_id) {
    const auto it = fit_of_id.find(proposer_id);
    if (it == fit_of_id.cend()) {
        return -1;
    }

    return it->second;
}

bool Agent::judge(const int proposer_id, const int fit, const int antagonist_id) {
    if (fit < cur_fit) {
        crush_id = proposer_id;
        cur_fit = fit;
        sat_fit = std::min(sat_fit, fit + 1);
        max_sat_fit = std::max(max_sat_fit, sat_fit);
        this->antagonist_id = antagonist_id;
        return true;
    }
    return false;
}

void Agent::engage_target_proposer() {
    suitor_id = crush_id;
    crush_id = NotAnAgentId;
    accepted = false;
    divorced = false;
}

void Agent::engage_target_proposee() {
    cur_fit = sat_fit;
    assert(cur_fit == fit_of_id.find(crush_id)->second);
    engage_target_proposer();
}

void Agent::rebound_from_crush() {
    crush_id = NotAnAgentId;
    sat_fit = std::min(sat_fit + 1, K - 1);
    max_sat_fit = std::max(max_sat_fit, sat_fit);
}

void Agent::break_up_with_current() {
    suitor_id = NotAnAgentId;
    cur_fit = K;
    divorced = false;
}
