#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <openssl/sha.h> // Utilisation de SHA256 d'OpenSSL

using namespace std;
using namespace std::chrono;

// ----------- Fonction utilitaire : SHA256 -----------
string sha256(const string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)data.c_str(), data.size(), hash);
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    return ss.str();
}

// ----------- Structure Transaction -----------
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

// ----------- Structure Block -----------
struct Block {
    int index;
    string prevHash;
    vector<Transaction> transactions;
    long long timestamp;
    long long nonce;
    string hash;

    Block(int idx, string prev, vector<Transaction> txs)
        : index(idx), prevHash(prev), transactions(txs), nonce(0) {
        timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        hash = calculateHash();
    }

    string calculateHash() const {
        stringstream ss;
        ss << index << prevHash << timestamp << nonce;
        for (auto& t : transactions)
            ss << t.toString();
        return sha256(ss.str());
    }

    void mineBlock(int difficulty) {
        string target(difficulty, '0'); // hash doit commencer par N zéros
        do {
            nonce++;
            hash = calculateHash();
        } while (hash.substr(0, difficulty) != target);
    }
};

// ----------- Structure Blockchain -----------
struct Blockchain {
    vector<Block> chain;

    Blockchain() {
        vector<Transaction> genesisTx = { {"genesis", "network", 0.0} };
        Block genesis(0, "0", genesisTx);
        chain.push_back(genesis);
    }

    Block getLastBlock() const { return chain.back(); }

    void addBlock(Block newBlock) {
        newBlock.prevHash = getLastBlock().hash;
        chain.push_back(newBlock);
    }

    bool isValid() const {
        for (size_t i = 1; i < chain.size(); ++i) {
            if (chain[i].prevHash != chain[i-1].hash)
                return false;
            if (chain[i].hash != chain[i].calculateHash())
                return false;
        }
        return true;
    }
};

// ----------- Programme principal -----------
int main() {
    cout << "=== Proof of Work Demo (C++ + OpenSSL) ===\n\n";

    // transactions d'exemple
    vector<Transaction> txs = {
        {"Diae", "Aymane", 5.0},
        {"Aymane", "Mouad", 2.5},
        {"Mouad", "Imad", 1.0}
    };

    // différents niveaux de difficulté
    vector<int> difficulties = {1, 2, 3, 4};

    for (int diff : difficulties) {
        Blockchain bc;
        Block newBlock(1, bc.getLastBlock().hash, txs);

        cout << "⛏️  Mining block with difficulty " << diff << "..." << endl;
        auto start = high_resolution_clock::now();
        newBlock.mineBlock(diff);
        auto end = high_resolution_clock::now();

        auto duration = duration_cast<milliseconds>(end - start).count();
        bc.addBlock(newBlock);

        cout << "✅ Block mined!" << endl;
        cout << "   Nonce: " << newBlock.nonce << endl;
        cout << "   Hash : " << newBlock.hash << endl;
        cout << "   Time : " << duration << " ms\n\n";
    }

    cout << "Proof of Work demonstration complete ✅\n";
    cout << "Try increasing difficulty to 5 or 6 to see exponential growth in time!\n";
    return 0;
}
