# Porthole: the device firmware, with Pets Club as its first game. Host simulator, pet self-test,
# checks and coverage; firmware through PlatformIO.
# The monorepo's root Makefile and CI call these as `make -C apps/porthole <target>`:
#   lint check test coverage ci firmware flash monitor   (plus sim snap snap-asan playtest art webemu clean)
# CXX is make's default (g++; Apple clang on macOS) unless you pass one.
# os/ is the shared runtime (renderer, input, palette, widgets, the App contract); shell/ is the profile picker and
# launcher; games/<name>/ compile against os/ only. Every directory under games/ is built in: a game includes its own
# headers by bare name (same directory), hosts and tests by path (games/pets-club/game.h), hence -I.
INCLUDES := -Ios -Ishell -I.
# Warnings are errors in every host build: sim, snap, snap-asan, the tests and the coverage objects.
WARNINGS := -Wall -Wextra -Werror -Wno-unused-parameter -Wno-missing-field-initializers
CXXFLAGS ?= -std=c++17 -O1 -g
CXXFLAGS += $(WARNINGS) $(INCLUDES) -DHOST_BUILD
# snap-asan keeps -Wmaybe-uninitialized fatal too: under -fsanitize=bounds, gcc 13.3/14.2 flag `(this->*TABLE[i])()`
# with it, and on aarch64 that call really is miscompiled (screens stuck, issue #22). Bind the entry to a local first.
# Python warnings are errors too: -W error raises them, and the wrapper fails on the ones Python can only print
# (a ResourceWarning from a finalizer). Our scripts only; PlatformIO and gcovr run their own interpreters.
PY := ../../tools/fail-on-warning.sh '[A-Za-z]+Warning: ' python3 -W error
# Biscuit's art generator is Node (22+, no npm packages), a dev-machine tool: lint skips it, loudly, where node is missing.
NODE_ART := games/biscuit/tools/art/export-assets.mjs
SRC := $(wildcard os/*.cpp) $(wildcard shell/*.cpp) $(wildcard games/*/*.cpp)
# Host self-checks: every host/test_<name>.cpp, linked with test_<name>_SRC. A new test is its file plus that one line.
TESTS := $(notdir $(basename $(wildcard host/test_*.cpp)))
test_pet_SRC := $(wildcard os/*.cpp) games/pets-club/pet.cpp
test_shell_SRC := shell/profiles.cpp shell/migrate.cpp games/pets-club/pet.cpp
# games/biscuit/pet.h is header-only: test_biscuit links nothing else
test_biscuit_SRC :=
test_biscuit_content_SRC := games/biscuit/personalize.cpp games/biscuit/art.cpp
test_gfx565_SRC := $(wildcard os/*.cpp)
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
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer $(SRC) host/sim.cpp -o $(BUILD)/snap-asan

# Scripted playthroughs, UI audit and monkey runs (tests/playtests/*.txt); report in build/playtest/report.md.
playtest: snap snap-asan
	$(PY) tools/playtest.py

test: $(BUILD) $(SRC) $(TESTS:%=host/%.cpp)
	$(foreach t,$(TESTS),$(CXX) $(CXXFLAGS) $($(t)_SRC) host/$(t).cpp -o $(BUILD)/$(t) && $(BUILD)/$(t) &&) true

# App-specific static checks. ruff/shellcheck run from the repo-root pre-commit, not here.
# snap is here so check compiles every source (with -Werror) in seconds.
lint: snap
	$(PY) games/pets-club/tools/check_content.py
	$(PY) games/pets-club/tools/art.py --check
	$(PY) tools/shell_art.py --check
	$(PY) games/biscuit/tools/check_content.py
	$(PY) games/biscuit/tools/icon.py --check
	@command -v node >/dev/null || { echo "node not found: skipping $(NODE_ART) --check (install Node 22+ to run it)"; exit 0; }; \
	  ../../tools/fail-on-warning.sh 'Warning: ' node $(NODE_ART) --check

# What pre-commit runs: seconds, not minutes.
check: lint test

# Everything CI runs for this app except the firmware build.
ci: check playtest coverage

# Line coverage of os/, shell/ and games/*/ from the host tests plus the non-monkey playtests, as Cobertura XML.
# Separate objects keep the .gcno/.gcda names predictable for both gcc and clang; they mirror the source tree
# (build/coverage/obj/games/<game>/pet.o) because two games can both have a pet.cpp. COV_MIN: baseline
# 93.9% (gcc 13, ubuntu-24.04) / 94.3% (Apple clang) at import, minus 2 points.
COV := build/coverage
COV_MIN := 91
COV_CXXFLAGS := -std=c++17 -O0 -g --coverage $(WARNINGS) $(INCLUDES) -DHOST_BUILD
COV_OBJ = $(addprefix $(COV)/obj/,$(1:.cpp=.o))
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
	for f in $(SRC) host/sim.cpp $(TESTS:%=host/%.cpp); do o=$(COV)/obj/$${f%.cpp}.o; mkdir -p $$(dirname $$o) && $(CXX) $(COV_CXXFLAGS) -c $$f -o $$o || exit 1; done
	$(foreach t,$(TESTS),$(CXX) --coverage $(call COV_OBJ,$($(t)_SRC) host/$(t).cpp) -o $(COV)/$(t) && $(COV)/$(t) &&) true
	$(CXX) --coverage $(call COV_OBJ,$(SRC) host/sim.cpp) -o $(COV)/snap
	PLAYTEST_SNAP=$(COV)/snap PLAYTEST_OUT=$(COV)/playtest $(PY) tools/playtest.py $(COV_SCENARIOS)
	../../tools/fail-on-warning.sh '\((WARNING|ERROR)\)' $(GCOVR) --root . --filter '(os|shell|games)/.*\.cpp$$' --gcov-executable "$(GCOV)" --cobertura $(COV)/coverage.xml \
	  --txt --print-summary --fail-under-line $(COV_MIN) $(COV)/obj

art:
	python3 games/pets-club/tools/art.py
	python3 tools/shell_art.py
	python3 games/biscuit/tools/icon.py
	node $(NODE_ART)

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
