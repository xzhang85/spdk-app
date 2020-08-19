#ifndef SPDK_APP_DEV_STRUCT_H
#define SPDK_APP_DEV_STRUCT_H

#include "spdk/bdev.h"
#include <string>

class Dev;

Dev *find_device(const std::string &bdev_name);
int create_device(const std::string &bdev_name);
void remove_device(const std::string &bdev_name);
void remove_all_devices();

class Dev {
public:
  Dev() = default;
  ~Dev() { close(); }

  std::string m_bdev_name;
  struct spdk_bdev *m_bdev{};
  struct spdk_bdev_desc *m_bdev_desc{};
  struct spdk_io_channel *m_bdev_io_channel{};
  void *m_buff{};
  struct spdk_bdev_io_wait_entry m_bdev_io_wait {};
  size_t m_num_blk{};
  size_t m_blk_size{};
  size_t m_data_blk_size{};

private:
  bool m_inited{};

  int open(const std::string &bdev_name);
  void close();
  static void dev_event_cb(enum spdk_bdev_event_type type,
                           struct spdk_bdev *bdev, void *event_ctx);

  friend Dev *find_device(const std::string &bdev_name);
  friend int create_device(const std::string &bdev_name);
  friend void remove_device(const std::string &bdev_name);
  friend void remove_all_devices();
};

#endif // SPDK_APP_DEV_STRUCT_H
