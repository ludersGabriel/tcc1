#ifndef BLOCK_H
#define BLOCK_H

#include <openssl/sha.h>
#include <bits/stdc++.h>

using namespace std;

class Transaction {
  public:
    string from;
    string to;
    int amount;

    Transaction();
    Transaction(string from, string to, int amount);
};

class Block {
  public: 
    Transaction transaction;
    unsigned int nonce;

    char previousHash[65];
    char hash[65];

    Block();
    Block(string previousHash);
    Block(
      string previousHash, 
      unsigned int nonce,
      string from,
      string to,
      int amount
    );

    static Block* genesisBlock();
    static string hashString(string str);
    static unsigned int randomInt();

    void hashMe();
    void print(bool isGenesis = false);
};

#endif