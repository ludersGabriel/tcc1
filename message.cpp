#include "message.h"

Message::Message(){}

Message::Message(
  int id, 
  int type, 
  string info, 
  vector<Block*> &blocks,
  string hostName,
  int blockCount
){
  this->id = id;
  this->type = type;
  strcpy(this->info, info.c_str());

  strcpy(this->hostName, hostName.c_str());
  this->blockCount = blockCount;

  this->blocks = blocks;
}