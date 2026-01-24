# C Integration Tests

This directory contains C-based integration tests for Onion components using the [Unity](https://github.com/ThrowTheSwitch/Unity) test framework.

## Running Tests

```bash
# From project root (runs in Docker)
make test-c

# Build/run locally (if on Linux with sqlite3-dev installed)
make run-c-tests
```

## Test Framework

Uses [Unity](https://github.com/ThrowTheSwitch/Unity) (MIT license, git submodule):
- `unity/` - Unity test framework
- Built-in `setUp()` / `tearDown()` lifecycle
- Rich assertions (`TEST_ASSERT_EQUAL`, `TEST_ASSERT_NOT_NULL`, etc.)

## Adding New Tests

1. Create `test_<component>.c`
2. Include `#include "unity.h"`
3. Add `setUp()` and `tearDown()` functions
4. Register tests in `main()` with `RUN_TEST(test_name)`
5. Add target to `Makefile`
