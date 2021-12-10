const http = require('http');

const server = http.createServer((req, res) => {
  console.log('>>> req.url: %s\n', req.url);

  res.end('uh, meow?');
});

server.listen(3000);
