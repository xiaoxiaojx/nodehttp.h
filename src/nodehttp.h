#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>

#include "llhttp.h"
#include "uv.h"

#define container_of(ptr, type, member)                                        \
  ((type*)((char*)(ptr)-offsetof(type, member)))

#define DEFAULT_BACKLOG 128

#define DEFAULT_HOST "0.0.0.0"

typedef struct n_http_request_s {
  uv_stream_t* client;
} n_http_request_t;

typedef struct n_http_server_s {
  void (*request_handler)(n_http_request_t*);

  uv_tcp_t uv_server;

  uv_loop_t* uv_loop;
} n_http_server_t;

typedef struct n_accept_req_s {
  llhttp_t parser;
  uv_stream_t* client;
  void (*request_handler)(n_http_request_t*);
} n_accept_req_t;

static std::map<int, n_accept_req_t> n_accept_list;

static void alloc_cb(uv_handle_t* handle,
                     size_t suggested_size,
                     uv_buf_t* buf) {
  buf->base = (char*)malloc(suggested_size);
  // suggested_size 是一个比较大的估值, 并非实际会传输的大小
  buf->len = suggested_size;

  fprintf(stdout, ">>> alloc_cb suggested_size: %d\n", suggested_size);
}

static void close_cb(uv_handle_t* handle) {
  free(handle);
}

static void shutdown_cb(uv_shutdown_t* req, int status) {
  assert(status == 0);
  uv_close((uv_handle_t*)req->handle, close_cb);
  free(req);
}

static void write_cb(uv_write_t* req, int status) {
  assert(status == 0);

  uv_close((uv_handle_t*)(req->handle), close_cb);
}

static void read_cb(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
  int i;
  uv_shutdown_t* sreq;
  int shutdown = 0;
  enum llhttp_errno llhttp_err;

  fprintf(stdout, ">>> read_cb  nread: %d, bufLen: %d\n", nread, buf->len);
  // fprintf(stdout, ">>> read_cb  buf: %s\n", buf->base);

  if (nread < 0) {
    /* Error or EOF */
    assert(nread == UV_EOF);

    free(buf->base);
    sreq = (uv_shutdown_t*)malloc(sizeof *sreq);
    if (uv_is_writable(handle)) {
      assert(0 == uv_shutdown(sreq, handle, shutdown_cb));
    }
    return;
  }

  if (nread == 0) {
    /* Everything OK, but nothing read. */
    free(buf->base);
    return;
  }

  n_accept_req_t req = n_accept_list[handle->io_watcher.fd];

  if (buf->base == nullptr) {
    fprintf(stdout, ">>> llhttp_finish\n");

    // n_accept_list.erase(handle->io_watcher.fd);
    llhttp_err = llhttp_finish(&req.parser);
  } else {
    fprintf(stdout, ">>> llhttp_execute\n");

    llhttp_err = llhttp_execute(&req.parser, buf->base, nread);
  }

  fprintf(stdout,
          ">>> llhttp_err %d, finish %d\n",
          llhttp_err,
          (&req.parser)->finish);

  if (llhttp_err == HPE_OK) {
    /* Successfully parsed! */
  } else {
    fprintf(stderr,
            ">>> Parse error: %s %s\n",
            llhttp_errno_name(llhttp_err),
            (&req.parser)->reason);
  }
  free(buf->base);
  return;
}

static int on_message_complete(llhttp_t* parser) {
  fprintf(stdout,
          ">>> on_message_complete method: %d, status: %d\n",
          parser->method,
          parser->status_code);

  n_accept_req_t* req = container_of(parser, n_accept_req_t, parser);

  static n_http_request_t* user_req =
      (n_http_request_t*)malloc(sizeof(n_http_request_t));

  user_req->client = req->client;

  req->request_handler(user_req);

  return 0;
}

static int on_body(llhttp_t* parser, const char* body, size_t length) {
  fprintf(stdout, ">>> on_body  length: %d\n", length);

  return 0;
}

static int on_headers_complete(llhttp_t* parser) {
  fprintf(stdout,
          ">>> on_headers_complete  content_length: %d\n",
          parser->content_length);

  return 0;
}

static llhttp_t create_http_parser() {
  static llhttp_t parser;
  
  // 1. 声明 settings
  static llhttp_settings_t settings;

  // 2. Initialize user callbacks and settings
  llhttp_settings_init(&settings);

  /* 3. Set user callback */
  settings.on_message_complete = on_message_complete;
  settings.on_headers_complete = on_headers_complete;
  settings.on_body = on_body;

  /* Initialize the parser in HTTP_BOTH mode, meaning that it will select
   * between HTTP_REQUEST and HTTP_RESPONSE parsing automatically while reading
   * the first input.
   */
  llhttp_init(&parser, HTTP_BOTH, &settings);

  return parser;
}

static void on_new_connection(uv_stream_t* server, int status) {
  if (status < 0) {
    // error!

    fprintf(stderr, ">>> on_new_connection error %s\n", uv_strerror(status));
    return;
  }

  n_http_server_t* serv = container_of(server, n_http_server_t, uv_server);

  uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(serv->uv_loop, client);
  if (uv_accept(server, (uv_stream_t*)client) == 0) {
    uv_read_start((uv_stream_t*)client, alloc_cb, read_cb);

    n_accept_req_t req;
    req.parser = create_http_parser();
    req.request_handler = serv->request_handler;
    req.client = (uv_stream_t*)client;

    n_accept_list[client->io_watcher.fd] = req;
  }
}

void n_end(n_http_request_t* req, char* data) {
  uv_write_t write_req;
  uv_buf_t buf = {.base = data, .len = strlen(data)};

  fprintf(stdout, ">>> n_end  data: %s, len: %d\n", data, strlen(data));

  uv_read_stop((uv_stream_t*)req->client);

  uv_write(&write_req, (uv_stream_t*)req->client, &buf, 1, write_cb);
}

int n_listen(n_http_server_t* server, int port) {
  sockaddr_in addr;

  uv_tcp_init(server->uv_loop, &server->uv_server);
  uv_ip4_addr(DEFAULT_HOST, port, &addr);
  uv_tcp_bind(&server->uv_server, (const struct sockaddr*)&addr, 0);

  int listen_err = uv_listen(
      (uv_stream_t*)&server->uv_server, DEFAULT_BACKLOG, on_new_connection);

  if (listen_err) {
    fprintf(stderr, "Listen error %s\n", uv_strerror(listen_err));
    return 1;
  }
  fprintf(stdout, "Server running at http://localhost:%d/ 🚀🚀🚀\n", port);

  return uv_run(server->uv_loop, UV_RUN_DEFAULT);
}

n_http_server_t* n_create_server(void (*handler)(n_http_request_t*)) {
  uv_tcp_t uv_server;

  n_http_server_t* server = (n_http_server_t*)malloc(sizeof(n_http_server_t));

  assert(server != NULL);

  server->uv_loop = uv_default_loop();
  server->request_handler = handler;
  server->uv_server = uv_server;

  return server;
}