#include "nodehttp.h"

char* text = "HTTP/1.1 201 OK\r\n"
             "Content-Length: 9\r\n"
             "\r\n"
             "uh, meow?";

int main() {
  n_http_server_t* server = n_create_server([](n_http_request_t* req) {
    printf("req.url: %s\n", req->url);
    n_end(req, text);
  });

  return n_listen(server, 3000);
}