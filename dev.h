#ifndef SPDK_APP_DEV_STRUCT_H
#define SPDK_APP_DEV_STRUCT_H

#include "spdk/bdev.h"
#include <stdbool.h>

enum dev_state { DEV_UNINIT, DEV_INITED, DEV_STATES_NUM };

struct dev_struct {
  enum dev_state state;
  char bdev_name[32];
  struct spdk_bdev *bdev;
  struct spdk_bdev_desc *bdev_desc;
  struct spdk_io_channel *bdev_io_channel;
  char *buff;
  struct spdk_bdev_io_wait_entry bdev_io_wait;
  size_t num_blk;
  size_t blk_size;
};

struct dev_struct *get_device(const char *bdev_name);
int dev_init(const char *bdev_name);
int dev_exit(const char *bdev_name);

#endif // SPDK_APP_DEV_STRUCT_H
