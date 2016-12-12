#ifndef __AGENT4_H_INCLUDED__
#define __AGENT4_H_INCLUDED__

#include <algorithm>
#include <cassert>

#include "flatinvmap.h"

constexpr auto NotAnAgentId = std::numeric_limits<int>::min();

class Agent final {
public:
    Agent() = default;

    Agent(const int K, int id_of_fit[]):
        cur_fit(K), id_of_fit(id_of_fit), fit_of_id(id_of_fit, K)
    { /* empty */ }

    Agent(const Agent& other) = delete;
    Agent& operator=(const Agent& other) = delete;

    Agent(Agent&&) = default;
    Agent& operator=(Agent&&) = default;

    int  get_crush_id()  const      {return crush_id;}
    int  get_suitor_id() const      {return suitor_id;}
    int  get_antagonist_id() const  {return antagonist_id;}
    int  get_cur_fit()   const      {return cur_fit;}
    int  get_max_sat_fit()   const  {return max_sat_fit;}

    void clear_antagonist_id() {this->antagonist_id = NotAnAgentId;}

    static void set_max_pref(const int max_pref) {K = max_pref;}

    bool is_content()    const  {return cur_fit <= sat_fit;}
    bool is_divorced()   const  {return divorced;}
    bool is_accepted()   const  {return accepted;}

    void inform_of_acceptance() {accepted = true;}
    void inform_of_divorce()    {if (!accepted) divorced = true;}

    void clear_accept_status()  {accepted = false;}

    int target_critical_suitor();
    int screen(int);
    bool judge(int, int, int);
    void engage_target_proposer();
    void engage_target_proposee();
    void rebound_from_crush();
    void break_up_with_current();

private:
    bool divorced = false;                          // Did I got divorced in current round?
    bool accepted = false;                          // Did I got accepted in current round?
    int crush_id = NotAnAgentId;                    // id of zesty flirt
    int suitor_id = NotAnAgentId;                   // id of current suitor
    int antagonist_id = NotAnAgentId;               // id of best proposer's current suitor
    int cur_fit = std::numeric_limits<int>::max();  // fitness of current suitor
    int sat_fit = 0;                                // maximum suitor fitness allowing satisfaction
    int max_sat_fit = 0;                            // maximum suitor fitness ever
    static int K;                                   // maximum accepted suitor fitness
    int *id_of_fit = nullptr;                       // id of i-th suitor in order of fitness
    FlatSummarizerInvMap fit_of_id{};               // fitness of suitor with given id
};

#endif // __AGENT4_H_INCLUDED__
