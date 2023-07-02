
#include <bits/stdc++.h>
#include "peer.h"


using namespace std;
int main(int argc, char** argv){

  if(argc < 3){
    cerr << "Usage: " << argv[0] << " <host> <port>" << endl;
    exit(1);
  }

  string host = argv[1];
  int port = atoi(argv[2]);

  Peer* peer = new Peer(host, port);

  for(int i = 0; i < 10; i++){
    string hash = string(host + ":" + to_string(port) + ":" + to_string(i));
  
    Block* block = new Block(i, hash);

    peer->blocks.push_back(block);
  }

  peer->dispatch(); 

  return 0;
}