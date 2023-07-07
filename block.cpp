#include <openssl/sha.h>
#include "block.h"

Transaction::Transaction(){}

Transaction::Transaction(string from, string to, int amount){
  strcpy(this->from, from.c_str());
  strcpy(this->to, to.c_str());
  this->amount = amount;
}

Block::Block(){
  this->transaction = Transaction("", "", 0);
}

Block::Block(string previousHash){
  this->transaction = Transaction("", "", 0);
  this->nonce = Block::randomInt();

  strcpy(this->previousHash, previousHash.c_str());

  this->hashMe(); 
}

Block::Block(
  string previousHash, 
  unsigned int nonce,
  string from,
  string to,
  int amount
){
  this->transaction = Transaction(from, to, amount);
  this->nonce = nonce;

  strcpy(this->previousHash, previousHash.c_str());

  this->hashMe(); 
}

bool validHash(unsigned char* hash, int difficulty){
  for(int i = 0; i < difficulty; i++){
    if(hash[i] != '0'){
      return false;
    }
  }

  return true;
}

void Block::hashMe(){
  unsigned int nonce = this->nonce;
  string hash;

  do {
    this->nonce = nonce;

    string blockData = 
      this->previousHash + 
      string(this->transaction.from) + 
      string(this->transaction.to) + 
      to_string(this->transaction.amount) + 
      to_string(this->nonce);

    hash = Block::hashString(blockData);
    nonce = (nonce + 1) % UINT_MAX;
  } while (
    !validHash((unsigned char*)hash.c_str(), 5)
  );

  strcpy(this->hash, hash.c_str());
}

string Block::hashString(string str){
  unsigned char hash[SHA256_DIGEST_LENGTH];

  SHA256((unsigned char*)str.c_str(), str.length(), hash);

  stringstream ss;

  for(int i = 0; i < SHA256_DIGEST_LENGTH; i++){
    ss << hex << setw(2) << setfill('0') << static_cast<int>(hash[i]);
  }

  return ss.str();
}

Block* Block::genesisBlock(){
  string genesisRef 
    = "never, under any circumstances, open this: https://youtu.be/dQw4w9WgXcQ";
  
  string previousHash = Block::hashString(genesisRef);

  Block* block = new Block(
    previousHash, 
    268091161, 
    "rick",
    "bala",
    1000
  );
  
  return block;
}

unsigned int Block::randomInt(){
  return rand() % UINT_MAX;
}

void Block::print(bool isGenesis){
  !isGenesis ? cout << "Block:" << endl
  : cout << "Genesis block:" << endl;
  
  cout << "\tHash: " << this->hash << endl;
  cout << "\tPrevious block's hash: " << this->previousHash << endl;
  cout << "\tNonce: " << this->nonce << endl;
  cout << "\tTransaction:" << endl;
  cout << "\t\tFrom: " << this->transaction.from << endl;
  cout << "\t\tTo: " << this->transaction.to << endl;
  cout << "\t\tAmount: " << this->transaction.amount << endl;
}