#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h> 

using namespace std;

// SHA256 
string sha256(const string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)data.c_str(), data.size(), hash);
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    return ss.str();
}

// l'arbre de Merkle ---
string merkleRoot(vector<string> transactions) {
    if (transactions.empty()) return "";

    // Hachage de chaque transaction
    vector<string> layer;
    for (const auto& tx : transactions) layer.push_back(sha256(tx));

    // Combinaison jusqu'à une seule racine
    while (layer.size() > 1) {
        if (layer.size() % 2 != 0)
            layer.push_back(layer.back()); // duplique si nombre impair
        vector<string> next;
        for (size_t i = 0; i < layer.size(); i += 2)
            next.push_back(sha256(layer[i] + layer[i + 1]));
        layer = next;
    }
    return layer[0];
}

int main() {
    cout << "=== Arbre de Merkle (C++) ===\n";
    vector<string> transactions = {
        "Diae -> Aymane : 10",
        "Aymane -> Ayoub : 5",
        "Ayoub -> Imad : 2",
        "Imad -> Mouad : 1"
    };

    cout << "Transactions :\n";
    for (auto& tx : transactions)
        cout << "  " << tx << "\n";

    string root = merkleRoot(transactions);
    cout << "\nMerkle Root : " << root << "\n";

    return 0;
}
