#include <billing.h>

// implementation
#if defined(__APPLE__)

#include <TargetConditionals.h>

#endif

#if defined(__ANDROID__)

#include "billing_android.c.h"

#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR

#include "billing_ios.m.h"

#else

#include "billing_sim.c.h"

#endif

struct billing_callbacks g_billing;
