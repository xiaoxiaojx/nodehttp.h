## A tiny http server

Node.js style http server implemented in c, based on libuv and llhttp.（It is mainly used for learning 📖 ✍️）

### Example

```c
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
```

As much as possible like the following Node.js syntax

```js
const http = require('http');

const server = http.createServer((req, res) => {
  console.log('req.url: %s\n', req.url);

  res.statusCode = 200;
  res.setHeader('Content-Type', 'text/plain');
  res.setHeader('Content-Length', '9');
  res.end('uh, meow?');
});

server.listen(3000);
```

### Benchmark

```bash
ab -c 100 -n 10000 http://127.0.0.1:3000/
```

#### nodehttp.h

```
This is ApacheBench, Version 2.3 <$Revision: 1826891 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 1000 requests
Completed 2000 requests
Completed 3000 requests
Completed 4000 requests
Completed 5000 requests
Completed 6000 requests
Completed 7000 requests
Completed 8000 requests
Completed 9000 requests
Completed 10000 requests
Finished 10000 requests


Server Software:
Server Hostname:        127.0.0.1
Server Port:            3000

Document Path:          /
Document Length:        9 bytes

Concurrency Level:      100
Time taken for tests:   0.740 seconds
Complete requests:      10000
Failed requests:        0
Total transferred:      470000 bytes
HTML transferred:       90000 bytes
Requests per second:    13514.46 [#/sec] (mean)
Time per request:       7.399 [ms] (mean)
Time per request:       0.074 [ms] (mean, across all concurrent requests)
Transfer rate:          620.29 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    3   1.0      3       9
Processing:     1    4   1.2      4      11
Waiting:        0    3   1.1      3       9
Total:          3    7   1.4      7      13

Percentage of the requests served within a certain time (ms)
  50%      7
  66%      8
  75%      8
  80%      8
  90%      9
  95%     10
  98%     11
  99%     11
 100%     13 (longest request)
```

#### Node.js (v16.5.0)

```
This is ApacheBench, Version 2.3 <$Revision: 1826891 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 1000 requests
Completed 2000 requests
Completed 3000 requests
Completed 4000 requests
Completed 5000 requests
Completed 6000 requests
Completed 7000 requests
Completed 8000 requests
Completed 9000 requests
Completed 10000 requests
Finished 10000 requests


Server Software:
Server Hostname:        127.0.0.1
Server Port:            3000

Document Path:          /
Document Length:        9 bytes

Concurrency Level:      100
Time taken for tests:   1.619 seconds
Complete requests:      10000
Failed requests:        0
Total transferred:      840000 bytes
HTML transferred:       90000 bytes
Requests per second:    6176.86 [#/sec] (mean)
Time per request:       16.189 [ms] (mean)
Time per request:       0.162 [ms] (mean, across all concurrent requests)
Transfer rate:          506.70 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   0.8      1      12
Processing:     5   15   6.3     13      56
Waiting:        2    9   4.5      8      43
Total:          6   16   6.3     14      56

Percentage of the requests served within a certain time (ms)
  50%     14
  66%     16
  75%     18
  80%     19
  90%     23
  95%     26
  98%     40
  99%     48
 100%     56 (longest request)
```

### Features

Possible functions to be implemented

- http
- https
- http2
