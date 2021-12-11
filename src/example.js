const http = require('http');

const server = http.createServer((req, res) => {
  console.log('req.url: %s\n', req.url);

  res.statusCode = 200;
  res.setHeader('Content-Type', 'text/plain');
  res.setHeader('Content-Length', '9');
  res.end('uh, meow?');
});

server.listen(3000);
