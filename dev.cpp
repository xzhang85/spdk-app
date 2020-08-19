#include "dev.hpp"
#include "spdk/bdev.h"
#include "spdk/bdev_module.h"
#include "spdk/env.h"
#include "spdk/log.h"
#include "spdk/stdinc.h"
#include <memory>
#include <string>
#include <unordered_map>

static std::unordered_map<std::string, std::unique_ptr<Dev>> g_devlist;

Dev *find_device(const std::string &bdev_name) {
  auto it = g_devlist.find(bdev_name);

  if (it != g_devlist.end())
    return it->second.get();

  return nullptr;
}

int create_device(const std::string &bdev_name) {
  if (g_devlist.find(bdev_name) != g_devlist.end()) {
    SPDK_ERRLOG("dev already initialized\n");
    return -EEXIST;
  }

  auto dev = std::make_unique<Dev>();
  auto rc = dev->open(bdev_name);
  if (!rc)
    g_devlist.insert({bdev_name, std::move(dev)});

  return rc;
}

void remove_device(const std::string &bdev_name) { g_devlist.erase(bdev_name); }

void remove_all_devices() { g_devlist.clear(); }

void Dev::dev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev,
                       void *event_ctx) {
  auto dev = static_cast<Dev *>(event_ctx);
  SPDK_NOTICELOG("device event callback called\n");
  if (type == SPDK_BDEV_EVENT_REMOVE)
    dev->close();
}

int Dev::open(const std::string &bdev_name) {
  int rc;
  uint32_t buf_align;
  m_bdev = spdk_bdev_get_by_name(bdev_name.c_str());
  if (!m_bdev) {
    SPDK_ERRLOG("Could not find the bdev: %s\n", bdev_name.c_str());
    return -EINVAL;
  }

  m_bdev_name = bdev_name;

  rc = spdk_bdev_open_ext(bdev_name.c_str(), true, dev_event_cb, this,
                          &m_bdev_desc);
  if (rc) {
    SPDK_ERRLOG("Could not open bdev: %s\n", m_bdev_name.c_str());
    return -EBUSY;
  }

  /* Open I/O channel */
  m_bdev_io_channel = spdk_bdev_get_io_channel(m_bdev_desc);
  if (m_bdev_io_channel == NULL) {
    SPDK_ERRLOG("Could not create bdev I/O channel!!\n");
    spdk_bdev_close(m_bdev_desc);
    return -1;
  }

  m_num_blk = spdk_bdev_get_num_blocks(m_bdev);
  m_blk_size = spdk_bdev_get_block_size(m_bdev);
  m_data_blk_size = spdk_bdev_get_data_block_size(m_bdev);
  buf_align = spdk_bdev_get_buf_align(m_bdev);
  SPDK_NOTICELOG(
      "num_blk = %zu, blk_size = %zu, data_blk_size = %zu, buf_align = %u\n",
      m_num_blk, m_blk_size, m_data_blk_size, buf_align);
  m_buff = spdk_dma_zmalloc(m_blk_size, buf_align, NULL);
  if (!m_buff) {
    SPDK_ERRLOG("Failed to allocate buffer\n");
    spdk_put_io_channel(m_bdev_io_channel);
    spdk_bdev_close(m_bdev_desc);
    return -1;
  }

  m_inited = true;

  return 0;
}

void Dev::close() {
  if (m_inited) {
    spdk_dma_free(m_buff);
    spdk_put_io_channel(m_bdev_io_channel);
    spdk_bdev_close(m_bdev_desc);
    m_inited = false;
  }
}
