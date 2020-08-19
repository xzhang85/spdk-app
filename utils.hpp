#ifndef SPDK_APP_UTILS_HPP
#define SPDK_APP_UTILS_HPP

#include "spdk/cpuset.h"

extern spdk_cpuset g_cpuset;

extern void init_cpuset(void);

#endif // SPDK_APP_UTILS_HPP
