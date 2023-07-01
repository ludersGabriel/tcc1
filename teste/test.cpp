#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <arpa/inet.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

struct event_base* base;


static void echoReadCb(struct bufferevent* bev, void *ctx){

  // grab both buffers
  struct evbuffer* input = bufferevent_get_input(bev);
  struct evbuffer* output = bufferevent_get_output(bev);

  // read from input and write to stdout
  printf("read: %s\n", evbuffer_pullup(input, -1));

  // write to output
  evbuffer_add_printf(output, "hey, its me, the server!\n");
}

static void echoEventCb(struct bufferevent *bev, short events, void *ctx){
  if(events & BEV_EVENT_ERROR){
    perror("Error from bufferevent");
  }

  if(events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)){
    bufferevent_free(bev);
  }

  if(events && BEV_EVENT_CONNECTED){
    struct evbuffer* output = bufferevent_get_output(bev);
    evbuffer_add_printf(output, "hey, its me, the client!\n");
  }
}

static void acceptConnCb(
  struct evconnlistener *listener,
  evutil_socket_t fd,
  struct sockaddr *address,
  int socklen,
  void *ctx
){
  // gets the base event
  struct event_base* base = evconnlistener_get_base(listener);
  
  // creates new bufferevent for that conn
  struct bufferevent* bev = bufferevent_socket_new(
    base, 
    fd, 
    BEV_OPT_CLOSE_ON_FREE
  );

  // set callbacks
  bufferevent_setcb(
    bev,
    echoReadCb,
    NULL,
    echoEventCb,
    NULL
  );

  // enable read and write
  bufferevent_enable(bev, EV_READ | EV_WRITE);
}

static void acceptErrorCb(
  struct evconnlistener *listener,
  void *ctx
){
  struct event_base* base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();
  fprintf(stderr, "Error = %d = \"%s\"\n", err, evutil_socket_error_to_string(err));

  event_base_loopexit(base, NULL);
}

static void clientReadCb(struct bufferevent* bev, void *ctx){
  struct evbuffer* input = bufferevent_get_input(bev);
  struct evbuffer* output = bufferevent_get_output(bev);

  printf("got as client: %s\n", evbuffer_pullup(input, -1));

  // due close on free, here we also close the connection
  // so we are sending a message, getting a response and closing the connection
  bufferevent_free(bev);

}

static void readStdin(int fd, short kind, void *userp){
  char buf[1024];
  int n = read(fd, buf, sizeof(buf));
  if(n < 0){
    perror("read");
  }else{
    buf[n] = '\0';
    printf("read: %s\n", buf);
  }

  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(0);
  sin.sin_port = htons(4040);

  struct bufferevent* bev = bufferevent_socket_new(
    base, 
    -1, 
    BEV_OPT_CLOSE_ON_FREE
  );
  
  bufferevent_setcb(
    bev,
    clientReadCb,
    NULL,
    echoEventCb,
    NULL
  );

  bufferevent_enable(bev, EV_READ | EV_WRITE);

  if (
    bufferevent_socket_connect(bev,
    (struct sockaddr *)&sin, sizeof(sin)) < 0
  ){
      /* Error starting connection */
      bufferevent_free(bev);
  }
}

int main(int argc, char** argv){
  struct evconnlistener* listener;
  struct sockaddr_in sin;

  int port = 8080;
  
  if(argc > 1){
    port = atoi(argv[1]);
  }

  base = event_base_new();
  if(!base){
    fprintf(stderr, "Could not initialize libevent!\n");
    return 1;
  }

  // cleaning the socket
  memset(&sin, 0, sizeof(sin));

  // setting the socket
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(0);
  sin.sin_port = htons(port);

  listener = evconnlistener_new_bind(
    base,
    acceptConnCb,
    NULL,
    LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
    -1,
    (struct sockaddr*)&sin,
    sizeof(sin)
  );

  if(!listener){
    perror("Could not create a listener!\n");
    return 1;
  }

  evconnlistener_set_error_cb(listener, acceptErrorCb);

  struct event *stdinEvent = event_new(
    base,
    0, // for stdin
    EV_READ | EV_PERSIST,
    readStdin,
    NULL
  );

  event_add(stdinEvent, NULL);

  event_base_dispatch(base);

  return 0;
}