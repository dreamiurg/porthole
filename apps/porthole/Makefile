# Porthole: the device firmware, with Pets Club as its first game. Host simulator, pet self-test,
# checks and coverage; firmware through PlatformIO.
# The monorepo's root Makefile and CI call these as `make -C apps/porthole <target>`:
#   lint check test coverage ci firmware flash monitor   (plus sim snap snap-asan playtest art webemu clean)
# CXX is make's default (g++; Apple clang on macOS) unless you pass one.
# os/ is the shared runtime (renderer, input, palette, widgets, the App contract); shell/ is the profile picker and
# launcher; games/<name>/ compile against os/ only.
GAME := games/pets-club
INCLUDES := -Ios -Ishell -I$(GAME)
# Warnings are errors in every host build: sim, snap, snap-asan, the tests and the coverage objects.
WARNINGS := -Wall -Wextra -Werror -Wno-unused-parameter -Wno-missing-field-initializers
CXXFLAGS ?= -std=c++17 -O1 -g
CXXFLAGS += $(WARNINGS) $(INCLUDES) -DHOST_BUILD
# GCC documents that sanitizers cause -Wmaybe-uninitialized false positives (gcc 13 flags the screen dispatch
# tables under ASan); the plain snap and test builds keep that warning fatal. Clang has no such flag.
ASAN_WNO := $(if $(shell $(CXX) --version 2>/dev/null | grep -q clang || echo gcc),-Wno-maybe-uninitialized)
# Python warnings are errors too: -W error raises them, and the wrapper fails on the ones Python can only print
# (a ResourceWarning from a finalizer). Our scripts only; PlatformIO and gcovr run their own interpreters.
PY := ../../tools/fail-on-warning.sh '[A-Za-z]+Warning: ' python3 -W error
SRC := $(wildcard os/*.cpp) $(wildcard shell/*.cpp) $(wildcard $(GAME)/*.cpp)
PET_TEST_SRC := $(wildcard os/*.cpp) $(GAME)/pet.cpp
SHELL_TEST_SRC := shell/profiles.cpp shell/migrate.cpp $(GAME)/pet.cpp
SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
SDL_LIBS := $(shell sdl2-config --libs 2>/dev/null)
BUILD := build/host

all: sim test

$(BUILD):
	mkdir -p $(BUILD)

sim: $(BUILD) $(SRC) host/sim.cpp
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -DHAVE_SDL $(SRC) host/sim.cpp -o $(BUILD)/sim $(SDL_LIBS)

snap: $(BUILD) $(SRC) host/sim.cpp
	$(CXX) $(CXXFLAGS) $(SRC) host/sim.cpp -o $(BUILD)/snap

# Same headless sim under AddressSanitizer + UBSan, for the chaos-monkey playtests.
snap-asan: $(BUILD) $(SRC) host/sim.cpp
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer $(ASAN_WNO) $(SRC) host/sim.cpp -o $(BUILD)/snap-asan

# Scripted playthroughs, UI audit and monkey runs (tests/playtests/*.txt); report in build/playtest/report.md.
playtest: snap snap-asan
	$(PY) tools/playtest.py

test: $(BUILD) $(SRC) host/test_pet.cpp host/test_shell.cpp
	$(CXX) $(CXXFLAGS) $(PET_TEST_SRC) host/test_pet.cpp -o $(BUILD)/test_pet && $(BUILD)/test_pet
	$(CXX) $(CXXFLAGS) $(SHELL_TEST_SRC) host/test_shell.cpp -o $(BUILD)/test_shell && $(BUILD)/test_shell

# App-specific static checks. ruff/shellcheck run from the repo-root pre-commit, not here.
# snap is here so check compiles every source (with -Werror) in seconds.
lint: snap
	$(PY) $(GAME)/tools/check_content.py
	$(PY) $(GAME)/tools/art.py --check
	$(PY) tools/shell_art.py --check

# What pre-commit runs: seconds, not minutes.
check: lint test

# Everything CI runs for this app except the firmware build.
ci: check playtest coverage

# Line coverage of os/, shell/ and games/*/ from test_pet, test_shell plus the non-monkey playtests, as Cobertura XML.
# Separate objects keep the .gcno/.gcda names predictable for both gcc and clang. COV_MIN: baseline
# 93.9% (gcc 13, ubuntu-24.04) / 94.3% (Apple clang) at import, minus 2 points.
COV := build/coverage
COV_MIN := 91
COV_CXXFLAGS := -std=c++17 -O0 -g --coverage $(WARNINGS) $(INCLUDES) -DHOST_BUILD
COV_OBJ = $(addprefix $(COV)/obj/,$(notdir $(1:.cpp=.o)))
COV_SCENARIOS := $(foreach f,$(wildcard tests/playtests/*.txt),$(if $(findstring monkey,$(f)),,$(basename $(notdir $(f)))))
# gcovr has no switch to fail on its own (WARNING) lines, so tools/fail-on-warning.sh does it.
GCOVR ?= $(shell command -v gcovr 2>/dev/null || echo uvx gcovr)
# ponytail: keyed on the OS, so it assumes Apple clang on macOS and gcc elsewhere; pass GCOV=... for other mixes.
ifeq ($(shell uname -s),Darwin)
GCOV ?= xcrun llvm-cov gcov
else
GCOV ?= gcov
endif

coverage:
	rm -rf $(COV) && mkdir -p $(COV)/obj
	for f in $(SRC) host/sim.cpp host/test_pet.cpp host/test_shell.cpp; do $(CXX) $(COV_CXXFLAGS) -c $$f -o $(COV)/obj/$$(basename $$f .cpp).o || exit 1; done
	$(CXX) --coverage $(call COV_OBJ,$(PET_TEST_SRC)) $(COV)/obj/test_pet.o -o $(COV)/test_pet
	$(CXX) --coverage $(call COV_OBJ,$(SHELL_TEST_SRC)) $(COV)/obj/test_shell.o -o $(COV)/test_shell
	$(CXX) --coverage $(call COV_OBJ,$(SRC)) $(COV)/obj/sim.o -o $(COV)/snap
	$(COV)/test_pet
	$(COV)/test_shell
	PLAYTEST_SNAP=$(COV)/snap PLAYTEST_OUT=$(COV)/playtest $(PY) tools/playtest.py $(COV_SCENARIOS)
	../../tools/fail-on-warning.sh '\((WARNING|ERROR)\)' $(GCOVR) --root . --filter '(os|shell|games)/.*\.cpp$$' --gcov-executable "$(GCOV)" --cobertura $(COV)/coverage.xml \
	  --txt --print-summary --fail-under-line $(COV_MIN) $(COV)/obj

art:
	python3 $(GAME)/tools/art.py
	python3 tools/shell_art.py

webemu: snap
	python3 tools/webemu.py

# PlatformIO exits 0 on its own "Warning!" lines (an unknown platformio.ini option, say); the wrapper fails on them.
firmware:
	../../tools/fail-on-warning.sh '^Warning!' pio run -e firmware

factory:
	pio run -e firmware -t factory

flash:
	pio run -e firmware -t upload $(if $(PORT),--upload-port $(PORT))

monitor:
	pio device monitor -b 115200 $(if $(PORT),-p $(PORT))

clean:
	rm -rf build

.PHONY: all sim snap snap-asan playtest test lint check ci coverage art webemu firmware factory flash monitor clean
