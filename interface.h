#ifndef INTERFACE_H
#define INTERFACE_H

#include <bits/stdc++.h>

#include "peer.h"

using namespace std;

class Peer;
class Interface {
  public:
    Peer *peer;
    
    Interface(Peer *peer);
    void listCommands();
    string readFromTerminal(int fd);
    void treatInput(string input);
};

#endif