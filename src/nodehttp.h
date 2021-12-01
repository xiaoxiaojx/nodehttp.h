#include <stdlib.h>
#include "uv.h"

#define DEFAULT_PORT 3000

#define DEFAULT_BACKLOG 128

#define DEFAULT_HOST "0.0.0.0"

struct node_http_request_t {};

struct node_http_server {
  int port;
  int loop;
  void (*request_handler)(node_http_request_t*);
  struct sockaddr_in addr;
};

node_http_server* create_server(void (*handler)(node_http_request_t*)) {
  node_http_server* serv = (node_http_server*)malloc(sizeof(node_http_server));;
}