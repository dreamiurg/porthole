# Biscuit: browser game (Node 22, no runtime deps) + ESP32-S3 firmware (PlatformIO).
# Root Makefile and CI call `make -C apps/biscuit <target>`. Run `npm ci` once first (Biome).
# Warnings are errors everywhere: biome, node tests (tests/fail-on-warning.js), C++ (-Werror), Python (-W error, plus
# the wrapper for warnings Python can only print, like a ResourceWarning from a finalizer).

NODE_TEST = node --import ./tests/fail-on-warning.js --test
PY = ../../tools/fail-on-warning.sh '[A-Za-z]+Warning: ' python3 -W error

COVERAGE_FLAGS = --experimental-test-coverage \
  --test-coverage-include='src/**' --test-coverage-exclude=src/app.js \
  --test-coverage-lines=97 --test-coverage-branches=95 --test-coverage-functions=97

.PHONY: check ci lint test coverage fonts-check firmware firmware-test factory flash monitor clean

check: lint test

ci: check coverage fonts-check

lint:
	@for f in server.js src/*.js tests/*.js firmware/tools/*.mjs; do node --check "$$f" || exit 1; done
	node_modules/.bin/biome lint --error-on-warnings
	$(PY) firmware/tests/partitions_test.py

test: build/pet_test
	$(NODE_TEST) tests/*.test.js
	build/pet_test

build/pet_test: firmware/tests/pet_test.cpp firmware/include/pet.h
	@mkdir -p build
	$(CXX) -std=c++17 -Wall -Wextra -Werror -pedantic -Ifirmware/include $< -o $@

coverage:
	@mkdir -p build/coverage
	$(NODE_TEST) $(COVERAGE_FLAGS) \
	  --test-reporter=spec --test-reporter-destination=stdout \
	  --test-reporter=lcov --test-reporter-destination=build/coverage/lcov.info \
	  tests/*.test.js

# The LVGL font C files are committed; regenerating them fetches lv_font_conv, so this needs network.
fonts-check:
	node firmware/tools/export-fonts.mjs --check

# PlatformIO exits 0 on its own "Warning!" lines (an unknown platformio.ini option, say); the wrapper fails on them.
firmware:
	../../tools/fail-on-warning.sh '^Warning!' pio run -d firmware -e firmware

factory:
	pio run -d firmware -e firmware -t factory

firmware-test:
	bash firmware/tools/check-layout.sh

flash:
	pio run -d firmware -e firmware -t upload $(if $(PORT),--upload-port $(PORT))

monitor:
	pio device monitor -d firmware -e firmware $(if $(PORT),--port $(PORT))

clean:
	rm -rf build
