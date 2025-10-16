#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>
#include <openssl/sha.h>
using namespace std;
using namespace std::chrono;

// ---------- SHA256 ----------
string sha256(const string &data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)data.c_str(), data.size(), hash);
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    return ss.str();
}

// ---------- Transaction ----------
struct Transaction {
    string id, sender, receiver;
    double amount;

    string toString() const {
        stringstream ss;
        ss << id << "|" << sender << "->" << receiver << ":" << fixed << setprecision(2) << amount;
        return ss.str();
    }
};

// ---------- Merkle Tree ----------
string merkleRoot(const vector<Transaction> &txs) {
    if (txs.empty()) return "";
    vector<string> layer;
    for (auto &t : txs)
        layer.push_back(sha256(t.toString()));

    while (layer.size() > 1) {
        if (layer.size() % 2 != 0)
            layer.push_back(layer.back());
        vector<string> next;
        for (size_t i = 0; i < layer.size(); i += 2)
            next.push_back(sha256(layer[i] + layer[i + 1]));
        layer = next;
    }
    return layer[0];
}

// ---------- Block ----------
struct Block {
    int index;
    string prevHash;
    vector<Transaction> transactions;
    string merkleRootHash;
    long long timestamp;
    long long nonce;
    string validator;
    string hash;

    Block(int idx, string prev, vector<Transaction> txs)
        : index(idx), prevHash(prev), transactions(txs), nonce(0), validator("") {
        timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        merkleRootHash = merkleRoot(transactions);
        hash = calculateHash();
    }

    string calculateHash() const {
        stringstream ss;
        ss << index << prevHash << merkleRootHash << timestamp << nonce << validator;
        for (auto &t : transactions)
            ss << t.toString();
        return sha256(ss.str());
    }

    // --- Proof of Work ---
    void mineBlock(int difficulty) {
        string target(difficulty, '0');
        do {
            nonce++;
            hash = calculateHash();
        } while (hash.substr(0, difficulty) != target);
    }

    // --- Proof of Stake ---
    void validatePoS(const string &val) {
        validator = val;
        hash = calculateHash();
    }
};

// ---------- Blockchain ----------
struct Blockchain {
    vector<Block> chain;

    Blockchain() {
        vector<Transaction> genesisTx = {{"0", "genesis", "network", 0.0}};
        Block genesis(0, "0", genesisTx);
        chain.push_back(genesis);
    }

    Block getLastBlock() const { return chain.back(); }

    void addBlock(const Block &b) { chain.push_back(b); }

    bool isValid() const {
        for (size_t i = 1; i < chain.size(); ++i) {
            if (chain[i].prevHash != chain[i - 1].hash)
                return false;
            if (chain[i].hash != chain[i].calculateHash())
                return false;
        }
        return true;
    }
};

// ---------- Sélection de validateur (PoS) ----------
string selectValidator(const vector<pair<string, double>> &validators) {
    double total = 0;
    for (auto &v : validators) total += v.second;

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dist(0.0, total);

    double r = dist(gen);
    double sum = 0;
    for (auto &v : validators) {
        sum += v.second;
        if (r <= sum)
            return v.first;
    }
    return validators.back().first;
}

// ---------- Programme principal ----------
int main() {
    cout << "=== Mini Blockchain: Merkle + PoW + PoS (C++ + OpenSSL) ===\n\n";

    // Exemple de transactions
    vector<Transaction> txs = {
        {"T1", "Diae", "Aymane", 5.0},
        {"T2", "Aymane", "Mouad", 3.5},
        {"T3", "Imad", "Smail", 2.0}
    };

    // ---------------- Proof of Work ----------------
    Blockchain powChain;
    Block powBlock(1, powChain.getLastBlock().hash, txs);

    int difficulty = 4;
    cout << "⛏️  Mining block (Proof of Work)...\n";
    auto start_pow = high_resolution_clock::now();
    powBlock.mineBlock(difficulty);
    auto end_pow = high_resolution_clock::now();
    powChain.addBlock(powBlock);

    auto pow_time = duration_cast<milliseconds>(end_pow - start_pow).count();

    cout << "✅ PoW mined successfully!\n";
    cout << "   Hash : " << powBlock.hash << "\n";
    cout << "   Nonce: " << powBlock.nonce << "\n";
    cout << "   Time : " << pow_time << " ms\n\n";

    // ---------------- Proof of Stake ----------------
    Blockchain posChain;
    Block posBlock(1, posChain.getLastBlock().hash, txs);

    vector<pair<string, double>> validators = {
        {"Validator_A", 50.0},
        {"Validator_B", 30.0},
        {"Validator_C", 20.0}
    };

    cout << "🏦 Selecting validator (Proof of Stake)...\n";
    auto start_pos = high_resolution_clock::now();
    string chosen = selectValidator(validators);
    posBlock.validatePoS(chosen);
    auto end_pos = high_resolution_clock::now();
    posChain.addBlock(posBlock);

    auto pos_time = duration_cast<milliseconds>(end_pos - start_pos).count();

    cout << "✅ PoS validated by: " << chosen << "\n";
    cout << "   Hash : " << posBlock.hash << "\n";
    cout << "   Time : " << pos_time << " ms\n\n";

    // ---------------- Analyse comparative ----------------
    cout << "=== Résumé ===\n";
    cout << "Proof of Work time: " << pow_time << " ms\n";
    cout << "Proof of Stake time: " << pos_time << " ms\n";
    cout << "=> ";
    if (pow_time > pos_time)
        cout << "⚡ Proof of Stake est plus rapide !\n";
    else
        cout << "⚡ Proof of Work est plus rapide ! (surprise)\n";

    cout << "\nValidation chaînes :\n";
    cout << "PoW chain valid? " << (powChain.isValid() ? "✅" : "❌") << "\n";
    cout << "PoS chain valid? " << (posChain.isValid() ? "✅" : "❌") << "\n";
    return 0;
}
