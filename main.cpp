
#include <bits/stdc++.h>
#include "peer.h"
#include "interface.h"

using namespace std;

int main(int argc, char** argv){
  srand(time(NULL));

  if(argc < 3){
    cerr << "Usage: " << argv[0] << " <host> <port>" << endl;
    exit(1);
  }

  string host = argv[1];
  int port = atoi(argv[2]);

  Peer* peer = new Peer(host, port);

  peer->interface->listCommands();
  peer->dispatch(); 

  return 0;
}