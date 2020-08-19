#ifndef SPDK_APP_PERF_H
#define SPDK_APP_PERF_H

#include "spdk/bdev.h"

extern int poor_perf1(const char *bdev_name, size_t count,
                      enum spdk_bdev_io_type type, size_t size);
extern int poor_perf2(const char *bdev_name, size_t count,
                      enum spdk_bdev_io_type type, size_t size);
extern int poor_perf3(const char *bdev_name, size_t count,
                      enum spdk_bdev_io_type type, size_t size);

#endif // SPDK_APP_PERF_H
