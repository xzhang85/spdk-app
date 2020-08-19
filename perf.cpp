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
  size_t count;
  size_t size;
  enum spdk_bdev_io_type type;
  int test_no;
  size_t finished;
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

  data->finished++;
  if (data->test_no == 1 && data->finished < data->count) {
    dev_write_buf(data);
  }
  if (data->finished == data->count) {
    clock_gettime(CLOCK_MONOTONIC, &data->end);
    auto duration_ns = data->end.tv_nsec - data->start.tv_nsec +
                       1'000'000'000 * (data->end.tv_sec - data->start.tv_sec);
    SPDK_PRINTF("Throughput is %f MB/s\n",
                data->total_bytes * 1000.0 / duration_ns);
    free(data);
  }
}

static void dev_write_buf(void *arg) {
  int rc;
  auto data = static_cast<struct poor_perf_data *>(arg);
  Dev *dev = data->dev;
  static uint64_t idx = 0;

  rc = spdk_bdev_write(dev->m_bdev_desc, dev->m_bdev_io_channel, dev->m_buff,
                       idx * dev->m_blk_size, data->size, write_comp_cb, data);

  if (rc == -ENOMEM) {
    /* In case we cannot perform I/O now, queue I/O */
    dev->m_bdev_io_wait.bdev = dev->m_bdev;
    dev->m_bdev_io_wait.cb_fn = dev_write_buf;
    dev->m_bdev_io_wait.cb_arg = data;
    if (spdk_bdev_queue_io_wait(dev->m_bdev, dev->m_bdev_io_channel,
                                &dev->m_bdev_io_wait))
      SPDK_ERRLOG("spdk_bdev_write failed with -ENOMEM and "
                  "spdk_bdev_queue_io_wait also failed\n");
    return;
  } else if (rc) {
    SPDK_ERRLOG("%s error while writing to bdev: %d\n", strerror(-rc), rc);
    return;
  }

  idx++;
  if (idx >= dev->m_num_blk)
    idx = 0;
}

static void dev_write_buf2(void *arg) {
  int rc;
  auto data = static_cast<struct poor_perf_data *>(arg);
  Dev *dev = data->dev;
  static uint64_t idx = 0;

  while (idx < dev->m_num_blk && idx < data->count) {
    rc = spdk_bdev_write(dev->m_bdev_desc, dev->m_bdev_io_channel, dev->m_buff,
                         idx++ * dev->m_blk_size, data->size, write_comp_cb,
                         data);

    if (rc == -ENOMEM) {
      /* In case we cannot perform I/O now, queue I/O */
      idx -= 1;
      dev->m_bdev_io_wait.bdev = dev->m_bdev;
      dev->m_bdev_io_wait.cb_fn = dev_write_buf2;
      dev->m_bdev_io_wait.cb_arg = data;
      if (spdk_bdev_queue_io_wait(dev->m_bdev, dev->m_bdev_io_channel,
                                  &dev->m_bdev_io_wait))
        SPDK_ERRLOG("spdk_bdev_write failed with -ENOMEM and "
                    "spdk_bdev_queue_io_wait also failed\n");
      break;
    } else if (rc) {
      SPDK_ERRLOG("%s error while writing to bdev: %d\n", strerror(-rc), rc);
      break;
    }
  }
}

static void dev_write_buf3(void *arg) {
  int rc;
  auto data = static_cast<struct poor_perf_data *>(arg);
  Dev *dev = data->dev;
  static uint64_t idx = 0;

  while (idx < dev->m_num_blk && idx < data->count) {
    rc = spdk_bdev_write(dev->m_bdev_desc, dev->m_bdev_io_channel, dev->m_buff,
                         idx++ * dev->m_blk_size, data->size, write_comp_cb,
                         data);

    if (rc == -ENOMEM) {
      /* In case we cannot perform I/O now, queue I/O */
      idx -= 1;
      dev->m_bdev_io_wait.bdev = dev->m_bdev;
      dev->m_bdev_io_wait.cb_fn = dev_write_buf3;
      dev->m_bdev_io_wait.cb_arg = data;
      if (spdk_bdev_queue_io_wait(dev->m_bdev, dev->m_bdev_io_channel,
                                  &dev->m_bdev_io_wait))
        SPDK_ERRLOG("spdk_bdev_write failed with -ENOMEM and "
                    "spdk_bdev_queue_io_wait also failed\n");
      break;
    } else if (rc) {
      SPDK_ERRLOG("%s error while writing to bdev: %d\n", strerror(-rc), rc);
      break;
    }
  }
}

int poor_perf1(const char *bdev_name, size_t count, enum spdk_bdev_io_type type,
               size_t size) {
  Dev *dev = find_device(bdev_name);
  auto data = static_cast<struct poor_perf_data *>(
      malloc(sizeof(struct poor_perf_data)));

  memset(data, 0, sizeof(struct poor_perf_data));
  data->dev = dev;
  data->count = count;
  data->type = type;
  data->size = size;
  data->finished = 0;
  data->test_no = 1;
  clock_gettime(CLOCK_MONOTONIC, &data->start);

  dev_write_buf(data);

  return 0;
}

int poor_perf2(const char *bdev_name, size_t count, enum spdk_bdev_io_type type,
               size_t size) {
  Dev *dev = find_device(bdev_name);
  auto data = static_cast<struct poor_perf_data *>(
      malloc(sizeof(struct poor_perf_data)));

  memset(data, 0, sizeof(struct poor_perf_data));
  data->dev = dev;
  data->count = count;
  data->type = type;
  data->size = size;
  data->finished = 0;
  data->test_no = 2;
  clock_gettime(CLOCK_MONOTONIC, &data->start);

  dev_write_buf2(data);

  return 0;
}

int poor_perf3(const char *bdev_name, size_t count, enum spdk_bdev_io_type type,
               size_t size) {
  Dev *dev = find_device(bdev_name);
  auto data = static_cast<struct poor_perf_data *>(
      malloc(sizeof(struct poor_perf_data)));

  memset(data, 0, sizeof(struct poor_perf_data));
  data->dev = dev;
  data->count = count;
  data->type = type;
  data->size = size;
  data->finished = 0;
  data->test_no = 3;
  clock_gettime(CLOCK_MONOTONIC, &data->start);

  dev_write_buf3(data);

  return 0;
}
