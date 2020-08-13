#include "spdk/bdev.h"
#include "spdk/bdev_module.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/log.h"
#include "spdk/rpc.h"
#include "spdk/stdinc.h"
#include "spdk/string.h"
#include "spdk/thread.h"
#include <signal.h>

#include "dev.h"
#include "perf.h"

void sig_handler(int sig) { spdk_app_stop(0); }

struct start_rpc_msg {
  char *bdev_name;
};

static const struct spdk_json_object_decoder start_rpc_decoder[] = {
    {"bdev_name", offsetof(struct start_rpc_msg, bdev_name),
     spdk_json_decode_string},
};

static void dev_init_rpc(struct spdk_jsonrpc_request *request,
                         const struct spdk_json_val *params) {
  int rc;
  struct spdk_json_write_ctx *w;
  struct start_rpc_msg req = {};

  if (spdk_json_decode_object(params, start_rpc_decoder,
                              SPDK_COUNTOF(start_rpc_decoder), &req)) {
    spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INVALID_PARAMS,
                                     "invalid parameters");
  }

  rc = dev_init(req.bdev_name);

  w = spdk_jsonrpc_begin_result(request);
  spdk_json_write_object_begin(w);
  spdk_json_write_named_int32(w, "ret_code", rc);
  spdk_json_write_object_end(w);
  spdk_jsonrpc_end_result(request, w);
}

static void dev_exit_rpc(struct spdk_jsonrpc_request *request,
                         const struct spdk_json_val *params) {
  int rc;
  struct spdk_json_write_ctx *w;
  struct start_rpc_msg req = {};

  if (spdk_json_decode_object(params, start_rpc_decoder,
                              SPDK_COUNTOF(start_rpc_decoder), &req)) {
    spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INVALID_PARAMS,
                                     "invalid parameters");
  }

  rc = dev_exit(req.bdev_name);

  w = spdk_jsonrpc_begin_result(request);
  spdk_json_write_object_begin(w);
  spdk_json_write_named_int32(w, "ret_code", rc);
  spdk_json_write_object_end(w);
  spdk_jsonrpc_end_result(request, w);
}

struct poor_perf_msg {
  char *bdev_name;
  int count;
};

static const struct spdk_json_object_decoder poor_perf_decoder[] = {
    {"bdev_name", offsetof(struct poor_perf_msg, bdev_name),
     spdk_json_decode_string},
    {"count", offsetof(struct poor_perf_msg, count), spdk_json_decode_int32},
};

static void poor_perf_rpc(struct spdk_jsonrpc_request *request,
                          const struct spdk_json_val *params) {
  int rc;
  struct spdk_json_write_ctx *w;
  struct poor_perf_msg req = {};

  if (spdk_json_decode_object(params, poor_perf_decoder,
                              SPDK_COUNTOF(poor_perf_decoder), &req)) {
    spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INVALID_PARAMS,
                                     "invalid parameters");
  }

  rc = poor_perf(req.bdev_name, req.count);

  w = spdk_jsonrpc_begin_result(request);
  spdk_json_write_object_begin(w);
  spdk_json_write_named_int32(w, "ret_code", rc);
  spdk_json_write_object_end(w);
  spdk_jsonrpc_end_result(request, w);
}

static void init_everything(void *arg1) {
  spdk_rpc_register_method("device_init", &dev_init_rpc, SPDK_RPC_RUNTIME);
  spdk_rpc_register_method("device_exit", &dev_exit_rpc, SPDK_RPC_RUNTIME);
  spdk_rpc_register_method("poor_perf", &poor_perf_rpc, SPDK_RPC_RUNTIME);
}

int main(int argc, char **argv) {
  struct spdk_app_opts opts = {};
  int rc = 0;

  spdk_app_opts_init(&opts);
  opts.name = "bdev_perf";

  if ((rc = spdk_app_parse_args(argc, argv, &opts, NULL, NULL, NULL, NULL)) !=
      SPDK_APP_PARSE_ARGS_SUCCESS) {
    exit(rc);
  }

  signal(SIGINT, sig_handler);

  rc = spdk_app_start(&opts, init_everything, NULL);
  if (rc) {
    SPDK_ERRLOG("ERROR starting application\n");
  }

  spdk_app_fini();

  return rc;
}
