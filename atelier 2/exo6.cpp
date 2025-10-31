#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <cassert>
#include <iomanip>
#include <sstream>

//-------------------------
// AC_HASH functions
//-------------------------
static std::vector<int> string_to_bits(const std::string& s) {
    std::vector<int> bits;
    bits.reserve(s.size() * 8);
    for (unsigned char c : s)
        for (int i = 7; i >= 0; --i)
            bits.push_back((c >> i) & 1);
    return bits;
}

static std::vector<int> init_state_from_bits(const std::vector<int>& bits, size_t state_size = 256) {
    std::vector<int> state(state_size, 0);
    for (size_t i = 0; i < bits.size(); ++i)
        state[i % state_size] ^= (bits[i] & 1);
    uint64_t len = bits.size();
    for (size_t i = 0; i < 64 && i < state_size; ++i)
        state[i] ^= ((len >> i) & 1);
    return state;
}

static std::vector<int> evolve_once(const std::vector<int>& state, uint8_t rule8) {
    size_t n = state.size();
    std::vector<int> next(n, 0);
    for (size_t i = 0; i < n; ++i) {
        int left = state[(i + n - 1) % n];
        int self = state[i];
        int right = state[(i + 1) % n];
        int pattern = (left << 2) | (self << 1) | right;
        next[i] = (rule8 >> pattern) & 1;
    }
    return next;
}

static std::vector<int> evolve_steps(std::vector<int> state, uint8_t rule8, size_t steps) {
    for (size_t t = 0; t < steps; ++t)
        state = evolve_once(state, rule8);
    return state;
}

static std::vector<int> ac_hash_state(const std::string& input, uint32_t rule = 30, size_t steps = 128) {
    uint8_t rule8 = static_cast<uint8_t>(rule & 0xFF);
    std::vector<int> bits = string_to_bits(input);
    std::vector<int> state = init_state_from_bits(bits, 256);
    return evolve_steps(state, rule8, steps);
}

//-------------------------
// MAIN: Distribution analysis
//-------------------------
int main() {
    const uint32_t RULE = 30;
    const size_t STEPS = 128;
    const size_t TARGET_BITS = 100000; // at least 10^5 bits
    const size_t STATE_SIZE = 256;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> len_dist(1, 64);
    std::uniform_int_distribution<int> byte_dist(0, 255);

    size_t total_bits = 0;
    size_t ones_count = 0;
    size_t messages_used = 0;

    while (total_bits < TARGET_BITS) {
        // Generate random message
        size_t len = len_dist(rng);
        std::string msg;
        msg.reserve(len);
        for (size_t i = 0; i < len; ++i)
            msg.push_back(static_cast<char>(byte_dist(rng)));

        std::vector<int> hash_state = ac_hash_state(msg, RULE, STEPS);
        for (int bit : hash_state)
            ones_count += bit;

        total_bits += hash_state.size();
        messages_used++;
    }

    double percent_ones = 100.0 * ones_count / total_bits;

    std::cout << "=== AC_HASH Bit Distribution Test ===\n";
    std::cout << "Messages used     : " << messages_used << "\n";
    std::cout << "Total bits tested : " << total_bits << "\n";
    std::cout << "Bits set to 1     : " << ones_count << "\n";
    std::cout << "Percentage of 1s  : " << std::fixed << std::setprecision(3)
        << percent_ones << " %\n";

    if (percent_ones > 48.0 && percent_ones < 52.0)
        std::cout << "→ Distribution équilibrée (≈50%) ✅\n";
    else
        std::cout << "→ Distribution non équilibrée ❌\n";

    return 0;
}
