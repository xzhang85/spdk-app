#include "spdk/bdev.h"
#include "spdk/bdev_module.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/log.h"
#include "spdk/rpc.h"
#include "spdk/stdinc.h"
#include "spdk/string.h"
#include "spdk/thread.h"

#include "dev.hpp"
#include "perf.hpp"
#include "utils.hpp"

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

  rc = create_device(req.bdev_name);

  w = spdk_jsonrpc_begin_result(request);
  spdk_json_write_object_begin(w);
  spdk_json_write_named_int32(w, "ret_code", rc);
  spdk_json_write_object_end(w);
  spdk_jsonrpc_end_result(request, w);
}

static void dev_exit_rpc(struct spdk_jsonrpc_request *request,
                         const struct spdk_json_val *params) {
  struct spdk_json_write_ctx *w;
  struct start_rpc_msg req = {};

  if (spdk_json_decode_object(params, start_rpc_decoder,
                              SPDK_COUNTOF(start_rpc_decoder), &req)) {
    spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INVALID_PARAMS,
                                     "invalid parameters");
  }

  remove_device(req.bdev_name);

  w = spdk_jsonrpc_begin_result(request);
  spdk_json_write_object_begin(w);
  spdk_json_write_named_int32(w, "ret_code", 0);
  spdk_json_write_object_end(w);
  spdk_jsonrpc_end_result(request, w);
}

struct poor_perf_msg {
  char *bdev_name;
  uint64_t count;
  char *type;
  uint64_t size;
};

static const struct spdk_json_object_decoder poor_perf_decoder[] = {
    {"bdev_name", offsetof(struct poor_perf_msg, bdev_name),
     spdk_json_decode_string},
    {"count", offsetof(struct poor_perf_msg, count), spdk_json_decode_uint64},
    {"type", offsetof(struct poor_perf_msg, type), spdk_json_decode_string},
    {"size", offsetof(struct poor_perf_msg, size), spdk_json_decode_uint64},
};

static enum spdk_bdev_io_type convert_type(const char *str) {
  enum spdk_bdev_io_type type = SPDK_BDEV_IO_TYPE_WRITE;
  if (!strcmp(str, "read"))
    type = SPDK_BDEV_IO_TYPE_READ;
  return type;
}

static void poor_perf1_rpc(struct spdk_jsonrpc_request *request,
                           const struct spdk_json_val *params) {
  int rc;
  struct spdk_json_write_ctx *w;
  struct poor_perf_msg req = {};

  if (spdk_json_decode_object(params, poor_perf_decoder,
                              SPDK_COUNTOF(poor_perf_decoder), &req)) {
    spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INVALID_PARAMS,
                                     "invalid parameters");
  }

  rc = poor_perf1(req.bdev_name, req.count, convert_type(req.type), req.size);

  w = spdk_jsonrpc_begin_result(request);
  spdk_json_write_object_begin(w);
  spdk_json_write_named_int32(w, "ret_code", rc);
  spdk_json_write_object_end(w);
  spdk_jsonrpc_end_result(request, w);
}

static void poor_perf2_rpc(struct spdk_jsonrpc_request *request,
                           const struct spdk_json_val *params) {
  int rc;
  struct spdk_json_write_ctx *w;
  struct poor_perf_msg req = {};

  if (spdk_json_decode_object(params, poor_perf_decoder,
                              SPDK_COUNTOF(poor_perf_decoder), &req)) {
    spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INVALID_PARAMS,
                                     "invalid parameters");
  }

  rc = poor_perf2(req.bdev_name, req.count, convert_type(req.type), req.size);

  w = spdk_jsonrpc_begin_result(request);
  spdk_json_write_object_begin(w);
  spdk_json_write_named_int32(w, "ret_code", rc);
  spdk_json_write_object_end(w);
  spdk_jsonrpc_end_result(request, w);
}

static void poor_perf3_rpc(struct spdk_jsonrpc_request *request,
                           const struct spdk_json_val *params) {
  int rc;
  struct spdk_json_write_ctx *w;
  struct poor_perf_msg req = {};

  if (spdk_json_decode_object(params, poor_perf_decoder,
                              SPDK_COUNTOF(poor_perf_decoder), &req)) {
    spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INVALID_PARAMS,
                                     "invalid parameters");
  }

  rc = poor_perf3(req.bdev_name, req.count, convert_type(req.type), req.size);

  w = spdk_jsonrpc_begin_result(request);
  spdk_json_write_object_begin(w);
  spdk_json_write_named_int32(w, "ret_code", rc);
  spdk_json_write_object_end(w);
  spdk_jsonrpc_end_result(request, w);
}

static void init_rpc() {
  spdk_rpc_register_method("device_init", &dev_init_rpc, SPDK_RPC_RUNTIME);
  spdk_rpc_register_method("device_exit", &dev_exit_rpc, SPDK_RPC_RUNTIME);
  spdk_rpc_register_method("poor_perf1", &poor_perf1_rpc, SPDK_RPC_RUNTIME);
  spdk_rpc_register_method("poor_perf2", &poor_perf2_rpc, SPDK_RPC_RUNTIME);
  spdk_rpc_register_method("poor_perf3", &poor_perf3_rpc, SPDK_RPC_RUNTIME);
}

static void app_run(void *arg1) {
  init_cpuset();
  init_rpc();
}

int main(int argc, char **argv) {
  struct spdk_app_opts opts = {};
  int rc = 0;

  spdk_app_opts_init(&opts);
  opts.name = "bdev_test";

  if ((rc = spdk_app_parse_args(argc, argv, &opts, NULL, NULL, NULL, NULL)) !=
      SPDK_APP_PARSE_ARGS_SUCCESS) {
    exit(rc);
  }

  rc = spdk_app_start(&opts, app_run, NULL);
  if (rc) {
    SPDK_ERRLOG("ERROR starting application\n");
  }

  remove_all_devices();
  spdk_app_fini();

  return rc;
}
