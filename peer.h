#ifndef PEER_H
#define PEER_H

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <arpa/inet.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <bits/stdc++.h>

using namespace std;

class Block {
  public: 
    int id;
    char hash[100];

    Block(){}
    Block(int id, string hash){
      this->id = id;
      if(hash.size() < 100)
        strcpy(this->hash, hash.c_str());
    }
};

class Message {
  public:
    int id;
    int type;
    char info[100];
    char hostName[100];
    
    int blockCount;
    vector<Block*> blocks;

    Message(){}
    Message(
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
};

class Peer{
  public:
    string host;
    int port;
    int receivedId = 0;
    int sentId = 0;
    string hostName;
    
    vector<string> peersAddresses;
    vector<Block*> blocks;

    struct event_base* base;
    struct event* stdinEvent;
    struct evconnlistener* listener;
    struct sockaddr_in sin;

    Peer(string host, int port);
    static void acceptConnection_cb(
      struct evconnlistener *listener,
      evutil_socket_t fd,
      struct sockaddr *address,
      int socklen,
      void *ctx
    );

    static void read_cb(struct bufferevent* bev, void *ctx);
    static void readKeyboard_cb(int fd, short kind, void *ctx);
    static void event_cb(struct bufferevent *bev, short events, void *ctx);
    static void acceptError_cb(
      struct evconnlistener *listener,
      void *ctx
    );
    static void clientRead_cb(struct bufferevent* bev, void *ctx);
    
    void dispatch();
    void sendMessage(Message* message, string adrr);
    Message* messageFromBuffer(evbuffer* input);
    void messageToBuffer(evbuffer* output, Message* message);
};

class Context {
  public:
    Peer* peer;
    Message* message;

    Context(Peer* peer, Message* message){
      this->peer = peer;
      this->message = message;
    }
};


#endif