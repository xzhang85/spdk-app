#include "spdk/bdev.h"
#include "spdk/bdev_module.h"
#include "spdk/env.h"
#include "spdk/log.h"
#include "spdk/stdinc.h"
#include <time.h>

#include "dev.hpp"
#include "perf.hpp"

struct poor_perf_data {
  Dev *dev;
  int count;
  size_t total_bytes;
  struct timespec start;
  struct timespec end;
};

static void dev_write_buf(void *arg);

static void write_comp_cb(struct spdk_bdev_io *bdev_io, bool success,
                          void *cb_arg) {
  auto data = static_cast<struct poor_perf_data *>(cb_arg);

  data->total_bytes += data->dev->m_blk_size;

  spdk_bdev_free_io(bdev_io);

  if (!success) {
    SPDK_ERRLOG("bdev io write error: %d\n", EIO);
    return;
  }

  data->count--;
  if (data->count >= 0) {
    dev_write_buf(data);
  } else {
    clock_gettime(CLOCK_REALTIME, &data->end);
    SPDK_PRINTF("Throughput is %f MB/s\n",
                data->total_bytes / 1000.0 / 1000 /
                    (data->end.tv_sec - data->start.tv_sec));
    free(data);
  }
}

static void dev_write_buf(void *arg) {
  int rc;
  auto data = static_cast<struct poor_perf_data *>(arg);
  Dev *dev = data->dev;

  rc = spdk_bdev_write(dev->m_bdev_desc, dev->m_bdev_io_channel, dev->m_buff, 0,
                       dev->m_blk_size, write_comp_cb, data);

  if (rc == -ENOMEM) {
    /* In case we cannot perform I/O now, queue I/O */
    dev->m_bdev_io_wait.bdev = dev->m_bdev;
    dev->m_bdev_io_wait.cb_fn = dev_write_buf;
    dev->m_bdev_io_wait.cb_arg = data;
    if (spdk_bdev_queue_io_wait(dev->m_bdev, dev->m_bdev_io_channel,
                                &dev->m_bdev_io_wait))
      SPDK_ERRLOG("spdk_bdev_write failed with -ENOMEM and "
                  "spdk_bdev_queue_io_wait also failed\n");
  } else if (rc) {
    SPDK_ERRLOG("%s error while writing to bdev: %d\n", strerror(-rc), rc);
  }
}

int poor_perf(const char *bdev_name, int count) {
  Dev *dev = find_device(bdev_name);
  auto data = static_cast<struct poor_perf_data *>(
      malloc(sizeof(struct poor_perf_data)));

  memset(data, 0, sizeof(struct poor_perf_data));
  data->dev = dev;
  data->count = count;
  clock_gettime(CLOCK_REALTIME, &data->start);

  dev_write_buf(data);

  return 0;
}