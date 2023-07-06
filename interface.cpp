#include "interface.h"

void Interface::listCommands(){
  cout << "Commands:" << endl;
  cout << "\tshow blockchain: shows all blocks" << endl;
  cout << "\tchain summary: shows a summary of the chain" << endl;
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
  if(input == "exit" || input == "quit"){
    cout << "Bye!" << endl;

    event_base_loopexit(this->peer->base, NULL);
  }
  else if(input == "show blockchain"){
    cout << "Blockchain:" << endl;
    for(unsigned long int i = 0; i < this->peer->blocks.size(); i++){
      this->peer->blocks[i]->print(i == 0);
    }
  }
  else if(input == "help"){
    this->listCommands();
  }
  else if(input == "chain summary"){
    cout << "Chain summary:" << endl;
    cout << "\tBlocks: " << this->peer->blocks.size() << endl;
    for(unsigned long int i = 0; i < this->peer->blocks.size(); i++){
      cout << "\t\tBlock " << i << ": " << this->peer->blocks[i]->hash << endl;
    }
  }
  else{
    cout << input << " is not a valid command" << endl;
  }

}