#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <openssl/sha.h> // Utilisation de la fonction SHA256
#include <random>
using namespace std;
using namespace std::chrono;

// === Fonction SHA256 (via OpenSSL) ===
string sha256(const string &data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)data.c_str(), data.size(), hash);
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    return ss.str();
}

// === Structures de base ===
struct Transaction {
    string from;
    string to;
    double amount;

    string toString() const {
        stringstream ss;
        ss << from << " -> " << to << " : " << fixed << setprecision(2) << amount;
        return ss.str();
    }
};

struct Block {
    int index;
    string prevHash;
    vector<Transaction> transactions;
    long long timestamp;
    long long nonce;
    string validator;
    string hash;

    Block(int idx, string prev, vector<Transaction> txs)
        : index(idx), prevHash(prev), transactions(txs), nonce(0), validator("") {
        timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        hash = calculateHash();
    }

    string calculateHash() const {
        stringstream ss;
        ss << index << prevHash << timestamp << nonce << validator;
        for (auto &t : transactions)
            ss << t.toString();
        return sha256(ss.str());
    }

    // Proof of Work: recherche d'un hash avec N zéros en tête
    void minePoW(int difficulty) {
        string target(difficulty, '0');
        do {
            nonce++;
            hash = calculateHash();
        } while (hash.substr(0, difficulty) != target);
    }

    // Proof of Stake: on "valide" sans miner, juste en signant
    void validatePoS(const string &val) {
        validator = val;
        hash = calculateHash();
    }
};

struct Blockchain {
    vector<Block> chain;

    Blockchain() {
        vector<Transaction> genesisTx = {{"genesis", "network", 0.0}};
        Block genesis(0, "0", genesisTx);
        chain.push_back(genesis);
    }

    Block getLastBlock() const { return chain.back(); }

    void addBlock(const Block &b) {
        chain.push_back(b);
    }

    bool isValid() const {
        for (size_t i = 1; i < chain.size(); i++) {
            if (chain[i].prevHash != chain[i - 1].hash)
                return false;
            if (chain[i].hash != chain[i].calculateHash())
                return false;
        }
        return true;
    }
};

// === Sélection de validateur (Proof of Stake) ===
string selectValidator(const vector<pair<string, double>> &validators) {
    double totalStake = 0;
    for (auto &v : validators)
        totalStake += v.second;

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dist(0.0, totalStake);

    double r = dist(gen);
    double cumulative = 0;
    for (auto &v : validators) {
        cumulative += v.second;
        if (r <= cumulative)
            return v.first;
    }
    return validators.back().first;
}

// === Programme principal ===
int main() {
    cout << "=== Proof of Work vs Proof of Stake Demo (C++ + OpenSSL) ===\n\n";

    vector<Transaction> txs = {
        {"Diae", "Aymane", 5.0},
        {"Aymane", "Mouad", 3.0},
        {"Mouad", "Imad", 1.0}
    };

    // --- Proof of Work ---
    cout << "⛏️  Mining (Proof of Work)...\n";
    Blockchain powChain;
    Block powBlock(1, powChain.getLastBlock().hash, txs);

    int difficulty = 4; // plus de zéros = plus lent
    auto start_pow = high_resolution_clock::now();
    powBlock.minePoW(difficulty);
    auto end_pow = high_resolution_clock::now();

    powChain.addBlock(powBlock);
    auto pow_time = duration_cast<milliseconds>(end_pow - start_pow).count();

    cout << "✅ PoW Block mined!\n";
    cout << "   Hash : " << powBlock.hash << "\n";
    cout << "   Nonce: " << powBlock.nonce << "\n";
    cout << "   Time : " << pow_time << " ms\n\n";

    // --- Proof of Stake ---
    cout << "🏦 Selecting validator (Proof of Stake)...\n";

    vector<pair<string, double>> validators = {
        {"Validator_A", 50.0},
        {"Validator_B", 30.0},
        {"Validator_C", 20.0}
    };

    Blockchain posChain;
    Block posBlock(1, posChain.getLastBlock().hash, txs);

    auto start_pos = high_resolution_clock::now();
    string selected = selectValidator(validators);
    posBlock.validatePoS(selected);
    auto end_pos = high_resolution_clock::now();

    posChain.addBlock(posBlock);
    auto pos_time = duration_cast<milliseconds>(end_pos - start_pos).count();

    cout << "✅ PoS Block validated by: " << selected << "\n";
    cout << "   Hash : " << posBlock.hash << "\n";
    cout << "   Time : " << pos_time << " ms\n\n";

    // --- Comparaison ---
    cout << "=== Résumé des performances ===\n";
    cout << "Proof of Work: " << pow_time << " ms\n";
    cout << "Proof of Stake: " << pos_time << " ms\n";

    if (pow_time > pos_time)
        cout << "⚡ Proof of Stake est plus rapide !\n";
    else
        cout << "⚡ Proof of Work est plus rapide (surprise !)\n";

    return 0;
}
