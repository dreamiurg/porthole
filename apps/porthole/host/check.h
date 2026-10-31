// Every host test includes this instead of <assert.h>. The tests are assert()s: built with NDEBUG they would pass
// without checking anything. The Makefile passes -UNDEBUG; this stops any other build that defines it.
#pragma once
#ifdef NDEBUG
#error "the host tests are assert()s: build them without NDEBUG"
#endif
#include <assert.h>

// An assert that also counts, for a test's closing "N checks" line; the test defines `checks`.
#define CHECK(c) (assert(c), checks++)
