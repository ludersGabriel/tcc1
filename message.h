#ifndef MESSAGE_H
#define MESSAGE_H

#include <bits/stdc++.h>
#include "block.h"

using namespace std;

class Message {
  public:
    int id;
    int type;
    char info[100];
    char hostName[100];
    
    int blockCount;
    vector<Block*> blocks;

    Message();
    Message(
      int id, 
      int type, 
      string info, 
      vector<Block*> &blocks,
      string hostName,
      int blockCount
    );
};


#endif 