#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "llhttp.h"
#include "uv.h"

#define container_of(ptr, type, member)                                        \
  ((type*)((char*)(ptr)-offsetof(type, member)))

#define DEFAULT_BACKLOG 128

#define DEFAULT_HOST "0.0.0.0"

typedef struct n_http_request_s {
} n_http_request_t;

void n_request_handler_t(n_http_request_t*);

typedef struct n_http_server_s {
  void (*request_handler)(n_http_request_t*);

  uv_tcp_t* uv_server;

  uv_loop_t* uv_loop;
} n_http_server_t;

llhttp_t parser;

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
  // Event needs to be fired after the parser is done.
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

  llhttp_settings_t settings;

  settings.on_message_complete = handle_on_message_complete;

  /* Initialize user callbacks and settings */
  llhttp_settings_init(&settings);

  /* Set user callback */

  /* Initialize the parser in HTTP_BOTH mode, meaning that it will select
   * between HTTP_REQUEST and HTTP_RESPONSE parsing automatically while reading
   * the first input.
   */
  llhttp_init(&parser, HTTP_BOTH, &settings);

  enum llhttp_errno err = llhttp_execute(&parser, buf->base, buf->len);

  fprintf(stdout, ">>>>>>>>>>>>>>>>>>>> hi: %s\n", buf->base);

  if (err == HPE_OK) {
    /* Successfully parsed! */
  } else {
    fprintf(
        stderr, "Parse error: %s %s\n", llhttp_errno_name(err), parser.reason);
  }

  free(buf->base);
  return;
}

static void on_new_connection(uv_stream_t* server, int status) {
  // error!
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    return;
  }

  n_http_server_t* serv = container_of(server, n_http_server_t, uv_server);

  uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(serv->uv_loop, client);
  if (uv_accept(server, (uv_stream_t*)client) == 0) {
    uv_read_start((uv_stream_t*)client, alloc_buffer, after_read);
  }
}

int n_listen(n_http_server_t* server, int port) {
  sockaddr_in addr;

  uv_tcp_init(server->uv_loop, server->uv_server);
  uv_ip4_addr(DEFAULT_HOST, port, &addr);
  uv_tcp_bind(server->uv_server, (const struct sockaddr*)&addr, 0);

  int r = uv_listen(
      (uv_stream_t*)server->uv_server, DEFAULT_BACKLOG, on_new_connection);

  if (r) {
    fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    return 1;
  }
  fprintf(stdout, "Server running at http://localhost:%d/ 🚀🚀🚀", port);

  return uv_run(server->uv_loop, UV_RUN_DEFAULT);
}

n_http_server_t* n_create_server(void (*handler)(n_http_request_t*)) {
  static uv_tcp_t uv_server;

  n_http_server_t* server = (n_http_server_t*)malloc(sizeof(n_http_server_t));

  assert(server != NULL);

  server->uv_loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
  assert(server->uv_loop != NULL);

  server->uv_server = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  assert(server->uv_server != NULL);


  server->uv_loop = uv_default_loop();
  server->request_handler = handler;
  server->uv_server = &uv_server;

  return server;
}