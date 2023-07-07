#ifndef MESSAGE_H
#define MESSAGE_H

#include <bits/stdc++.h>
#include "block.h"

using namespace std;

enum MessageType {
  HELLO,
  QUIT,
  ERROR
};

class Message {
  public:
    int id;
    MessageType type;
    char info[100];
    char hostName[100];
    
    
    int blockCount;
    vector<Block*> blocks;

    Message();
    Message(
      int id, 
      MessageType type, 
      string info, 
      string hostName,
      vector<Block*> &blocks,
      int blockCount
    );
};


#endif 