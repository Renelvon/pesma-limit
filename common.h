#ifndef __COMMON_H_INCLUDED__
#define __COMMON_H_INCLUDED__

#include <functional>
#include <queue>
#include <string>
#include <utility>
#include <vector>

using argument_t = std::tuple<int, int, int, double, double, long long int>;
using input_t = std::tuple<int, int, int*>;

using pqelem = std::pair<double, int>;
using pq_type = std::priority_queue<pqelem, std::vector<pqelem>, std::greater<pqelem>>;

enum class Set {set_a, set_b};

constexpr auto ROOT = 0;

argument_t parse_cli(int argc, char* argv[]);

void string_to_int_arr(const std::string& s, int[], int);
input_t read_instance();
long long int full_avalanche_hash(long long int);
input_t create_instance(argument_t);
input_t get_instance(int, char*[]);

double idx_to_uniform_weight();
double idx_to_inverse_weight(double, int);
double idx_to_contested_weight(double, int, int);
std::function<double(int)> choose_weight_fun(int, double, double);
double make_key(double, double);
void sample(int, int, int, const std::function<double(int)>&, long long int,
            int[], pq_type&);

bool is_root(int);
Set pick_proposers(int, bool, bool);

#endif // __UTILS_H_INCLUDED__
