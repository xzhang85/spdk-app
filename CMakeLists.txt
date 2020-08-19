cmake_minimum_required(VERSION 3.12)
project(spdk-app)

set(SPDK_ROOT "" CACHE STRING "spdk install dir")
set(SPDK_DPDK_ROOT "" CACHE STRING "spdk's dpdk install dir")

add_executable(spdk-app)
target_sources(spdk-app PRIVATE main.cpp dev.cpp dev.hpp perf.cpp perf.hpp utils.cpp utils.hpp)
target_compile_options(spdk-app PRIVATE -Wall -Werror -fPIC -pthread)
target_include_directories(spdk-app PRIVATE ${SPDK_ROOT}/include ${SPDK_DPDK_ROOT}/include)
target_link_directories(spdk-app PRIVATE ${SPDK_ROOT}/lib ${SPDK_ROOT}/../isa-l/.libs ${SPDK_DPDK_ROOT}/lib)
target_link_libraries(spdk-app
        -Wl,--whole-archive
        spdk_bdev spdk_bdev_rpc spdk_event spdk_accel spdk_log spdk_rpc spdk_thread spdk_util
        spdk_sock spdk_json spdk_jsonrpc spdk_notify spdk_trace spdk_conf
        spdk_bdev_malloc spdk_bdev_null spdk_nvme spdk_bdev_nvme spdk_bdev_ftl spdk_bdev_aio spdk_ftl
        spdk_sock_posix spdk_bdev_error spdk_bdev_split spdk_bdev_gpt spdk_bdev_lvol spdk_bdev_passthru
        spdk_lvol spdk_bdev_zone_block spdk_event_vmd spdk_event_sock spdk_event_bdev spdk_event_accel
        spdk_net spdk_blob spdk_blob_bdev spdk_vmd
        -Wl,--no-whole-archive
)
target_link_libraries(spdk-app
        spdk_env_dpdk
        -Wl,--whole-archive
        librte_eal.a librte_mempool.a librte_ring.a librte_mempool_ring.a librte_pci.a librte_bus_pci.a librte_kvargs.a librte_telemetry.a
        -Wl,--no-whole-archive
)
target_link_libraries(spdk-app dl numa rt pthread uuid crypto aio isal)
