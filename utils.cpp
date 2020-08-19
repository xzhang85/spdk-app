#include "spdk/env.h"
#include "utils.hpp"

spdk_cpuset g_cpuset;

void init_cpuset() {
  uint32_t i;
  spdk_cpuset_zero(&g_cpuset);
  SPDK_ENV_FOREACH_CORE(i) {
    spdk_cpuset_set_cpu(&g_cpuset, i, true);
  }
}