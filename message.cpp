#include "message.h"

Message::Message(){}

Message::Message(
  int id, 
  MessageType type, 
  string info, 
  string hostName,
  vector<Block*> &blocks,
  int blockCount
){
  this->id = id;
  this->type = type;
  strcpy(this->info, info.c_str());

  strcpy(this->hostName, hostName.c_str());
  this->blockCount = blockCount;

  this->blocks = blocks;
}