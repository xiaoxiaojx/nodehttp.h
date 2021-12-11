#include "nodehttp.h"

int main() {
  n_http_server_t* server =
      n_create_server([](n_http_request_t* req, n_http_response_t* res) {
        printf("req.url: %s\n", req->url);

        n_set_status_code(res, 200);
        n_set_header(res, "Content-Type", "text/plain");
        n_set_header(res, "Content-Length", "9");
        n_end(res, "uh, meow?");
      });

  return n_listen(server, 3000);
}