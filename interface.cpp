#include "interface.h"

void Interface::listCommands(){
  cout << "Commands:" << endl;
  cout << "\tblockchain: shows all blocks" << endl;
  cout << "\tblocks: shows a summary of the chain" << endl;
  cout << "\tusers: shows all neighbour nodes" << endl;
  cout << "\thelp: shows command list" << endl;
  cout << "\texit or quit: closes terminal " << endl << endl;
}

Interface::Interface(Peer *peer){
  this->peer = peer;
}

string Interface::readFromTerminal(int fd){
  char buffer[1024];

  int n = read(fd, buffer, sizeof(buffer));
  buffer[n - 1] = '\0';

  if(n <= 0){
    cerr << "Error reading from keyboard" << endl;
    return "";
  }
  
  string input = buffer;
  return input;
}

void Interface::treatInput(string input){
  cout << endl;
  
  if(input == "exit" || input == "quit"){
    cout << "Finalizing everything.." << endl;

    string filename = string("blocks:") + this->peer->hostName + ".txt";

    ofstream file(filename);

    for(unsigned long int i = 1; i < this->peer->blocks.size(); i++){
      Block* block = this->peer->blocks[i];
      
      string info = 
        to_string(block->nonce)
        + ","
        + string(block->transaction.from)
        + ","
        + string(block->transaction.to)
        + ","
        + to_string(block->transaction.amount);
      
      file << info << endl;
    }

    this->peer->quit();
  }
  else if(input == "blockchain"){
    cout << "Blockchain:" << endl;
    for(unsigned long int i = 0; i < this->peer->blocks.size(); i++){
      this->peer->blocks[i]->print(i == 0);
    }
  }
  else if(input == "help"){
    this->listCommands();
  }
  else if(input == "blocks"){
    cout << "blocks:" << endl;
    cout << "\tBlocks: " << this->peer->blocks.size() << endl;
    for(unsigned long int i = 0; i < this->peer->blocks.size(); i++){
      cout << "\t\tBlock " << i << ": " << this->peer->blocks[i]->hash << endl;
    }
  }
  else if(input == "users"){
    cout << "Users: " << endl;
    for(auto it : peer->peersAddresses){
      cout << "\t" << it.second << " at " << it.first << endl;
    }
  }
  else{
    cout << input << " is not a valid command" << endl;
  }

  cout << endl;

}