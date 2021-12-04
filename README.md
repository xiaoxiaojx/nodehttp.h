## A tiny http server
Node.js style http server implemented in c, based on libuv and llhttp.（It is mainly used for learning 📖 ✍️）

### Example
```c
#include "nodehttp.h"

int main() {
  n_http_server_t* server = n_create_server([](n_http_request_t* req) {
   n_end(req, "HTTP/1.1 201 OK\r\n"); 
  });

  return n_listen(server, 3000);
}
```
As much as possible like the following Node.js syntax
```js
const http = require('http');

const server = http.createServer((req, res) => {
  res.writeHead(200, { 'Content-Type': 'application/json' });
  res.end(JSON.stringify({
    data: 'Hello World!'
  }));
});

server.listen(3000);
```

### Features
Possible functions to be implemented
* http
* https
* http2