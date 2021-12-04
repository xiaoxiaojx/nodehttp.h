#include "nodehttp.h"

int main() {
  n_http_server_t* server = n_create_server(
      [](n_http_request_t* req) { n_end(req, "HTTP/1.1 201 OK\r\n"); });

  return n_listen(server, 3000);
}