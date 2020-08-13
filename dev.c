#include "dev.h"
#include "spdk/bdev.h"
#include "spdk/bdev_module.h"
#include "spdk/env.h"
#include "spdk/log.h"
#include "spdk/stdinc.h"

static struct dev_struct g_dev = {};

struct dev_struct *get_device(const char *bdev_name) {
  return &g_dev;
}

static void dev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev,
                         void *event_ctx) {
  struct dev_struct *dev = event_ctx;
  SPDK_NOTICELOG("device event callback called\n");
  spdk_dma_free(dev->buff);
  spdk_put_io_channel(dev->bdev_io_channel);
  spdk_bdev_close(dev->bdev_desc);
  memset(dev, 0, sizeof(struct dev_struct));
}

int dev_init(const char *bdev_name) {
  int rc;
  uint32_t buf_align;
  struct dev_struct *dev = get_device(bdev_name);

  if (!dev || dev->state == DEV_INITED) {
    SPDK_ERRLOG("dev already initialized\n");
    return -EEXIST;
  }

  dev->bdev = spdk_bdev_get_by_name(bdev_name);
  if (!dev->bdev) {
    SPDK_ERRLOG("Could not find the bdev: %s\n", bdev_name);
    return -EINVAL;
  }

  strncpy(dev->bdev_name, bdev_name, 31);
  dev->bdev_name[31] = '\0';

  rc = spdk_bdev_open_ext(dev->bdev_name, true, dev_event_cb, dev,
                          &dev->bdev_desc);
  if (rc) {
    SPDK_ERRLOG("Could not open bdev: %s\n", dev->bdev_name);
    return -EBUSY;
  }

  /* Open I/O channel */
  dev->bdev_io_channel = spdk_bdev_get_io_channel(dev->bdev_desc);
  if (dev->bdev_io_channel == NULL) {
    SPDK_ERRLOG("Could not create bdev I/O channel!!\n");
    spdk_bdev_close(dev->bdev_desc);
    return -1;
  }

  dev->num_blk = spdk_bdev_get_num_blocks(dev->bdev);
  dev->blk_size = spdk_bdev_get_block_size(dev->bdev);
  buf_align = spdk_bdev_get_buf_align(dev->bdev);
  SPDK_NOTICELOG("num_blk = %zu, blk_size = %zu, buf_align = %u\n",
                 dev->num_blk, dev->blk_size, buf_align);
  dev->buff = spdk_dma_zmalloc(dev->blk_size, buf_align, NULL);
  if (!dev->buff) {
    SPDK_ERRLOG("Failed to allocate buffer\n");
    spdk_put_io_channel(dev->bdev_io_channel);
    spdk_bdev_close(dev->bdev_desc);
    return -1;
  }

  SPDK_NOTICELOG("dev_init() successfully run\n");

  return 0;
}

int dev_exit(const char *bdev_name) {
  struct dev_struct *dev = get_device(bdev_name);

  if (!dev || dev->state != DEV_INITED)
    return -EEXIST;

  spdk_dma_free(dev->buff);
  spdk_put_io_channel(dev->bdev_io_channel);
  spdk_bdev_close(dev->bdev_desc);

  memset(dev, 0, sizeof(struct dev_struct));

  SPDK_NOTICELOG("dev_exit() successfully run\n");

  return 0;
}
