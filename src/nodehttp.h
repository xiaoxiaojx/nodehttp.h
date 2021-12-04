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
  llhttp_t* parser;
  void (*request_handler)(n_http_request_t*);
} n_accept_req_t;

typedef void (*request_handler_t)(n_http_request_t*);

static std::map<int, n_accept_req_t> n_accept_list;

static void alloc_buffer(uv_handle_t* handle,
                         size_t suggested_size,
                         uv_buf_t* buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = suggested_size;
}

static void on_close(uv_handle_t* peer) {
  free(peer);
}

static int handle_on_message_complete(llhttp_t* parser) {
  fprintf(stdout,
          "Method: %d, StatusCode: %d",
          parser->method,
          parser->status_code);
  return 0;
}

static void after_shutdown(uv_shutdown_t* req, int status) {
  assert(status == 0);
  uv_close((uv_handle_t*)req->handle, on_close);
  free(req);
}

static void after_read(uv_stream_t* handle,
                       ssize_t nread,
                       const uv_buf_t* buf) {
  int i;
  uv_shutdown_t* sreq;
  int shutdown = 0;
  enum llhttp_errno llhttp_err;

  fprintf(stdout,
          ">>> Request content: %s, length: %d, alloc_length: %d, nread: %d\n",
          buf->base,
          sizeof(buf->base),
          buf->len,
          nread);

  if (nread < 0) {
    /* Error or EOF */
    assert(nread == UV_EOF);

    free(buf->base);
    sreq = (uv_shutdown_t*)malloc(sizeof *sreq);
    if (uv_is_writable(handle)) {
      assert(0 == uv_shutdown(sreq, handle, after_shutdown));
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
    n_accept_list.erase(handle->io_watcher.fd);
    llhttp_err = llhttp_finish(req.parser);
  } else {
    llhttp_err = llhttp_execute(req.parser, buf->base, sizeof(buf->base));
  }

  if (llhttp_err == HPE_OK) {
    /* Successfully parsed! */
    static n_http_request_t* user_req =
        (n_http_request_t*)malloc(sizeof(n_http_request_t));

    user_req->client = handle;

    req.request_handler(user_req);
  } else {
    fprintf(stderr,
            "Parse error: %s %s\n",
            llhttp_errno_name(llhttp_err),
            req.parser->reason);
  }
  free(buf->base);
  return;
}

static llhttp_t* create_http_parser() {
  static llhttp_t parser;

  static llhttp_settings_t settings;

  /* Set user callback */
  settings.on_message_complete = handle_on_message_complete;

  /* Initialize user callbacks and settings */
  llhttp_settings_init(&settings);

  /* Initialize the parser in HTTP_BOTH mode, meaning that it will select
   * between HTTP_REQUEST and HTTP_RESPONSE parsing automatically while reading
   * the first input.
   */
  llhttp_init(&parser, HTTP_BOTH, &settings);

  return &parser;
}

static void on_new_connection(uv_stream_t* server, int status) {
  if (status < 0) {
    // error!

    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    return;
  }

  n_http_server_t* serv = container_of(server, n_http_server_t, uv_server);

  uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(serv->uv_loop, client);
  if (uv_accept(server, (uv_stream_t*)client) == 0) {
    uv_read_start((uv_stream_t*)client, alloc_buffer, after_read);

    n_accept_req_t req;
    req.parser = (llhttp_t*)malloc(sizeof(llhttp_t));
    req.parser = create_http_parser();
    req.request_handler = serv->request_handler;

    n_accept_list[client->io_watcher.fd] = req;
  }
}

void n_end(n_http_request_t* req, char* data) {
  int r;
  uv_buf_t buf;
  do {
    buf = uv_buf_init(data, sizeof(data));
    r = uv_try_write(req->client, &buf, 1);
    assert(r > 0 || r == UV_EAGAIN);
    if (r > 0) {
      break;
    }
  } while (1);

  do {
    buf = uv_buf_init("", 0);
    r = uv_try_write(req->client, &buf, 1);
  } while (r != 0);
  uv_close((uv_handle_t*)req->client, on_close);
}

int n_listen(n_http_server_t* server, int port) {
  sockaddr_in addr;

  uv_tcp_init(server->uv_loop, &server->uv_server);
  uv_ip4_addr(DEFAULT_HOST, port, &addr);
  uv_tcp_bind(&server->uv_server, (const struct sockaddr*)&addr, 0);

  int r = uv_listen(
      (uv_stream_t*)&server->uv_server, DEFAULT_BACKLOG, on_new_connection);

  if (r) {
    fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    return 1;
  }
  fprintf(stdout, "Server running at http://localhost:%d/ 🚀🚀🚀\n", port);

  return uv_run(server->uv_loop, UV_RUN_DEFAULT);
}

n_http_server_t* n_create_server(void (*handler)(n_http_request_t*)) {
  static uv_tcp_t uv_server;

  n_http_server_t* server = (n_http_server_t*)malloc(sizeof(n_http_server_t));

  assert(server != NULL);

  server->uv_loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
  assert(server->uv_loop != NULL);

  server->uv_loop = uv_default_loop();
  server->request_handler = handler;
  server->uv_server = uv_server;

  return server;
}