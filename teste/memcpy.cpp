#include <bits/stdc++.h>

using namespace std;

class Block {
  public:
    int id;
    string hash;

    Block(){}
    Block(int id, string hash){
      this->id = id;
      this->hash = hash;
    }
};

int main() {
  Block* a = new Block(1, "a");

  char* buffer = new char[sizeof(Block)];

  memcpy(buffer, a, sizeof(Block));

  Block* b = new Block();

  memcpy(b, buffer, sizeof(Block));

  cout << b->id << " " << b->hash << endl;

  return 0;
}