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

void Peer::read_cb(struct bufferevent* bev, void *ctx){
  Peer* peer = (Peer*)ctx;

  struct evbuffer* input = bufferevent_get_input(bev);
  struct evbuffer* output = bufferevent_get_output(bev);

  cout << "read: " << evbuffer_pullup(input, -1) << endl;
  
  peer->receivedId++;

  evbuffer_add_printf(
    output, 
    "Response to %d from %s:%d\n", 
    peer->receivedId, 
    peer->host.c_str(), 
    peer->port
  );
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
    string* message = (string* )ctx;

    cout << *message << endl;

    struct evbuffer* output = bufferevent_get_output(bev);

    evbuffer_add_printf(output, "%s", message->c_str());
    delete message;
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
    string *message = new string(
      string("Message ") 
      + to_string(peer->sentId)
      + " from " 
      + peer->host 
      + ":" 
      + to_string(peer->port) 
      + "\n"
      );
    
    peer->sendMessage(message, addr);
  }
}

// get response and close connection
void Peer::clientRead_cb(struct bufferevent* bev, void *ctx){
  Peer* peer = (Peer*)ctx;

  struct evbuffer* input = bufferevent_get_input(bev);

  cout << "read response as client: " << evbuffer_pullup(input, -1) << endl;

  bufferevent_free(bev);
}

void Peer::sendMessage(string* message, string addr){
  string hostname = addr.substr(0, addr.find(":"));
  int port = atoi(addr.substr(addr.find(":") + 1).c_str());

  struct sockaddr_in sin = createPeerSocket(port, hostname);  

  struct bufferevent* bev = bufferevent_socket_new(
    this->base, 
    -1, 
    BEV_OPT_CLOSE_ON_FREE
  );

  bufferevent_setcb(
    bev,
    this->clientRead_cb,
    NULL,
    this->event_cb,
    message
  );

  bufferevent_enable(bev, EV_READ | EV_WRITE);

  int ret = bufferevent_socket_connect(bev, (struct sockaddr*)&sin, sizeof(sin));

  if(ret < 0){
    cerr << "Error connecting to " << addr << endl;
    
    bufferevent_free(bev);
  }
  
}
