#include "nodehttp.h"

int main() {
  n_http_server_t server = n_create_server([](n_http_request_t* req) {

  });

  return n_listen(server, 3000);
}