## A tiny http server

Node.js style http server implemented in c, based on libuv and llhttp.（It is mainly used for learning 📖 ✍️）

### Example

```c
#include "nodehttp.h"

char *text = "HTTP/1.1 201 OK\r\n"
    "Content-Length: 9\r\n"
    "\r\n"
    "uh, meow?";

int main() {
  n_http_server_t* server = n_create_server([](n_http_request_t* req) {
    printf(">>> req.url: %s\n", req->url);

    n_end(req, text);
  });

  return n_listen(server, 3000);
}
```

As much as possible like the following Node.js syntax

```js
const http = require('http');

const server = http.createServer((req, res) => {
  console.log('>>> req.url: %s\n', req.url);

  res.end('uh, meow?');
});

server.listen(3000);
```

### Features

Possible functions to be implemented

- http
- https
- http2
