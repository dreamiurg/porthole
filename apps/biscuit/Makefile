# Biscuit: browser game (Node 22, no runtime deps) + ESP32-S3 firmware (PlatformIO).
# Root Makefile and CI call `make -C apps/biscuit <target>`. Run `npm ci` once first (Biome).

COVERAGE_FLAGS = --experimental-test-coverage \
  --test-coverage-include='src/**' --test-coverage-exclude=src/app.js \
  --test-coverage-lines=97 --test-coverage-branches=95 --test-coverage-functions=97

.PHONY: check ci lint test coverage fonts-check firmware firmware-test factory flash monitor clean

check: lint test

ci: check coverage fonts-check

lint:
	@for f in server.js src/*.js tests/*.js firmware/tools/*.mjs; do node --check "$$f" || exit 1; done
	node_modules/.bin/biome lint
	python3 firmware/tests/partitions_test.py

test: build/pet_test
	node --test tests/*.test.js
	build/pet_test

build/pet_test: firmware/tests/pet_test.cpp firmware/include/pet.h
	@mkdir -p build
	$(CXX) -std=c++17 -Wall -Wextra -Werror -pedantic -Ifirmware/include $< -o $@

coverage:
	@mkdir -p build/coverage
	node --test $(COVERAGE_FLAGS) \
	  --test-reporter=spec --test-reporter-destination=stdout \
	  --test-reporter=lcov --test-reporter-destination=build/coverage/lcov.info \
	  tests/*.test.js

# The LVGL font C files are committed; regenerating them fetches lv_font_conv, so this needs network.
fonts-check:
	node firmware/tools/export-fonts.mjs --check

firmware:
	pio run -d firmware -e firmware

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
