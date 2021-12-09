## A tiny http server

Node.js style http server implemented in c, based on libuv and llhttp.（It is mainly used for learning 📖 ✍️）

### Example

```c
#include "nodehttp.h"

char* text = "HTTP/1.1 201 OK\r\n"
             "Content-Length: 9\r\n"
             "\r\n"
             "uh, meow?";

int main() {
  n_http_server_t* server =
      n_create_server([](n_http_request_t* req, n_http_response_t* res) {
        printf("req.url: %s\n", req->url);

        n_end(res, text);
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

### Benchmark
In fact, when the concurrency becomes higher, the program will often make mistakes. As my first c project, I will optimize it later.
```bash
ab -c 3 -n 100 http://127.0.0.1:3000
```
#### nodehttp.h
```
This is ApacheBench, Version 2.3 <$Revision: 1826891 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient).....done


Server Software:
Server Hostname:        127.0.0.1
Server Port:            3000

Document Path:          /
Document Length:        9 bytes

Concurrency Level:      3
Time taken for tests:   0.007 seconds
Complete requests:      100
Failed requests:        0
Total transferred:      4700 bytes
HTML transferred:       900 bytes
Requests per second:    13611.00 [#/sec] (mean)
Time per request:       0.220 [ms] (mean)
Time per request:       0.073 [ms] (mean, across all concurrent requests)
Transfer rate:          624.72 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       0
Processing:     0    0   0.0      0       0
Waiting:        0    0   0.0      0       0
Total:          0    0   0.1      0       0

Percentage of the requests served within a certain time (ms)
  50%      0
  66%      0
  75%      0
  80%      0
  90%      0
  95%      0
  98%      0
  99%      0
 100%      0 (longest request)
```
#### Node.js (v16.5.0)
```
This is ApacheBench, Version 2.3 <$Revision: 1826891 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient).....done


Server Software:
Server Hostname:        127.0.0.1
Server Port:            3000

Document Path:          /
Document Length:        9 bytes

Concurrency Level:      3
Time taken for tests:   0.085 seconds
Complete requests:      100
Failed requests:        0
Total transferred:      8400 bytes
HTML transferred:       900 bytes
Requests per second:    1169.65 [#/sec] (mean)
Time per request:       2.565 [ms] (mean)
Time per request:       0.855 [ms] (mean, across all concurrent requests)
Transfer rate:          95.95 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.1      0       0
Processing:     1    2   4.3      1      41
Waiting:        0    1   3.7      1      35
Total:          1    2   4.3      1      41

Percentage of the requests served within a certain time (ms)
  50%      1
  66%      1
  75%      1
  80%      2
  90%      2
  95%      3
  98%     11
  99%     41
 100%     41 (longest request)
```

### Features

Possible functions to be implemented

- http
- https
- http2
