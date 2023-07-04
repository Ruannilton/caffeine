#pragma once

#include <caffeine_core.h>
#include <caffeine_container_type.h>

struct caffeine_ord_container_s {
	struct caffeine_container_s;
	cff_order_function comparer;
};

typedef struct caffeine_ord_container_s cff_ord_container_s;