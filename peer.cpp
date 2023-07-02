#include "peer.h"

struct hostent* getHost(string host){
  struct hostent *hp;

  gethostname((char*)host.c_str(), sizeof(host));

  hp = gethostbyname((char*)host.c_str());

  if(!hp){
    cerr << "Could not resolve host" << endl;
    exit(1);
  }

  return hp;
}

struct sockaddr_in createPeerSocket(int port, string hostname){
  struct sockaddr_in sin;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);

  struct hostent *hp = getHost(hostname);
  memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);

  return sin;
}

void createListener(Peer* peer){
  peer->sin = createPeerSocket(peer->port, peer->host);

  peer->listener = evconnlistener_new_bind(
    peer->base,
    NULL,
    NULL,
    LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
    -1,
    (struct sockaddr*)&peer->sin,
    sizeof(peer->sin)
  );

  if(!peer->listener){
    cerr << "Failed to create listener" << endl;
    exit(1);
  }

  evconnlistener_set_cb(
    peer->listener,
    peer->acceptConnection_cb,
    peer
  ); 

  evconnlistener_set_error_cb(
    peer->listener,
    peer->acceptError_cb
  );
}

void createBase(Peer* peer){
  peer->base = event_base_new();
  if(!peer->base){
    cerr << "Failed to create base" << endl;
    exit(1);
  }
}

void createKeyboardEvent(Peer* peer){
  peer->stdinEvent = event_new(
    peer->base,
    STDIN_FILENO,
    EV_READ | EV_PERSIST,
    peer->readKeyboard_cb,
    peer
  );

  if(!peer->stdinEvent){
    cerr << "Failed to create keyboard event" << endl;
    exit(1);
  }

  event_add(peer->stdinEvent, NULL);
}

void getPeersAddresses(Peer* peer){
  ifstream file("peers.txt");
  string line;

  string myAddress = peer->host + ":" + to_string(peer->port);

  while(getline(file, line)){
    if(line == myAddress) continue;
    peer->peersAddresses.push_back(line);
  }
}

// * Constructor
Peer::Peer(string host, int port){
  this->host = host;
  this->port = port;
  this->hostName = host + ":" + to_string(port);

  createBase(this);
  createListener(this);
  createKeyboardEvent(this);
  getPeersAddresses(this);

  cout << "Listening on " << host << ":" << port << endl;
}

void Peer::acceptConnection_cb(
  struct evconnlistener *listener,
  evutil_socket_t fd,
  struct sockaddr *address,
  int socklen,
  void *ctx
){
  Peer* peer = (Peer*)ctx;

  struct event_base* base = evconnlistener_get_base(listener);
  struct bufferevent* bev = bufferevent_socket_new(
    base, 
    fd, 
    BEV_OPT_CLOSE_ON_FREE
  );
  
  bufferevent_setcb(
    bev, 
    peer->read_cb, 
    NULL,
    peer->event_cb, 
    peer
  );
  
  bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void Peer::dispatch(){
  event_base_dispatch(this->base);
}

void Peer::messageToBuffer(evbuffer *output, Message* message){
  evbuffer_add(output, message, sizeof(Message) - sizeof(vector<Block*>));

  for(Block* block : message->blocks){
    evbuffer_add(output, block, sizeof(Block));
  }

  delete message;
}

void Peer::read_cb(struct bufferevent* bev, void *ctx){
  Peer* peer = (Peer*)ctx;

  struct evbuffer* output = bufferevent_get_output(bev);
  struct evbuffer* input = bufferevent_get_input(bev);
  
  Message* message = peer->messageFromBuffer(input);

  cout << "read: " << message->blockCount << " blocks from" << message->hostName << endl;

  for(Block* block : message->blocks){
    cout << "\tblock: " << block->id << " " << block->hash << endl;
  }

  delete message;

  string info = string("Response to ") + to_string(++(peer->receivedId)) + " from " + peer->hostName;

  Message* response = new Message(
    peer->sentId,
    1,
    info,
    peer->blocks,
    string(peer->host + ":" + to_string(peer->port)),
    peer->blocks.size()
  );

  peer->messageToBuffer(output, response);
}

void Peer::event_cb(struct bufferevent *bev, short events, void *ctx){
  Peer* peer = (Peer*) ctx;

  if(events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)){
    bufferevent_free(bev);
  }

  if(events & BEV_EVENT_ERROR){
    cerr << "Error from bufferevent" << endl;
  } 

  if(events & BEV_EVENT_CONNECTED){
    Context *c = (Context*)ctx;
    Message *message = c->message;

    struct evbuffer* output = bufferevent_get_output(bev);

    peer->messageToBuffer(output, message);
  }

}

void Peer::acceptError_cb(
  struct evconnlistener *listener,
  void *ctx
){
  Peer* peer = (Peer*)ctx;

  struct event_base* base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();
  cerr << "Error: " << err << " = " << evutil_socket_error_to_string(err) << endl;
  event_base_loopexit(base, NULL);
}

void Peer::readKeyboard_cb(int fd, short kind, void *ctx){
  Peer* peer = (Peer*)ctx;

  char buf[1024];
  int n = read(fd, buf, sizeof(buf));

  if(n <= 0){
    cerr << "Error reading from keyboard" << endl;
    return;
  }

  buf[n] = '\0';

  cout << "read: " << buf << endl;

  if(strcmp(buf, "quit\n") == 0){
    event_base_loopexit(peer->base, NULL);
    return;
  }

  
  for(string addr : peer->peersAddresses){
    peer->sentId++;
    
    Message* message = new Message(
      peer->sentId,
      1,
      "Hello",
      peer->blocks,
      string(peer->host + ":" + to_string(peer->port)),
      peer->blocks.size()
    );

    peer->sendMessage(message, addr);
  }
}

Message* Peer::messageFromBuffer(evbuffer* input){
  Message* message = new Message();
  evbuffer_remove(input, message, sizeof(Message) - sizeof(vector<Block*>));

  message->blocks = vector<Block*>();
  for(int i = 0; i < message->blockCount; i++){
    Block* block = new Block();
    evbuffer_remove(input, block, sizeof(Block));
    message->blocks.push_back(block);
  }

  return message;
}

// get response and close connection
void Peer::clientRead_cb(struct bufferevent* bev, void *ctx){
  Context* c = (Context*)ctx;
  Peer* peer = c->peer;

  struct evbuffer* input = bufferevent_get_input(bev);

  Message* message = peer->messageFromBuffer(input);
  
  cout << "read response as client: " << message->info << endl; 

  bufferevent_free(bev);
  delete c;
}

void Peer::sendMessage(Message* message, string addr){
  string hostname = addr.substr(0, addr.find(":"));
  int port = atoi(addr.substr(addr.find(":") + 1).c_str());

  struct sockaddr_in sin = createPeerSocket(port, hostname);  

  struct bufferevent* bev = bufferevent_socket_new(
    this->base, 
    -1, 
    BEV_OPT_CLOSE_ON_FREE
  );

  Context *ctx = new Context(
    this,
    message
  );

  bufferevent_setcb(
    bev,
    this->clientRead_cb,
    NULL,
    this->event_cb,
    ctx
  );

  bufferevent_enable(bev, EV_READ | EV_WRITE);

  int ret = bufferevent_socket_connect(bev, (struct sockaddr*)&sin, sizeof(sin));

  if(ret < 0){
    cerr << "Error connecting to " << addr << endl;
    
    bufferevent_free(bev);
  }
  
}
