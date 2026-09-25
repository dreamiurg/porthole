# Pets Club: host simulator, pet self-test, checks and coverage; firmware through PlatformIO.
# The monorepo's root Makefile and CI call these as `make -C apps/pets-club <target>`:
#   lint check test coverage ci firmware flash monitor   (plus sim snap snap-asan playtest art webemu play serve-play clean)
# CXX is make's default (g++; Apple clang on macOS) unless you pass one.
CXXFLAGS ?= -std=c++17 -O1 -g -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Isrc/game -DHOST_BUILD
GAME_SRC := $(wildcard src/game/*.cpp)
SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
SDL_LIBS := $(shell sdl2-config --libs 2>/dev/null)
BUILD := build/host

all: sim test

$(BUILD):
	mkdir -p $(BUILD)

sim: $(BUILD) $(GAME_SRC) host/sim.cpp
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -DHAVE_SDL $(GAME_SRC) host/sim.cpp -o $(BUILD)/sim $(SDL_LIBS)

snap: $(BUILD) $(GAME_SRC) host/sim.cpp
	$(CXX) $(CXXFLAGS) $(GAME_SRC) host/sim.cpp -o $(BUILD)/snap

# Same headless sim under AddressSanitizer + UBSan, for the chaos-monkey playtests.
snap-asan: $(BUILD) $(GAME_SRC) host/sim.cpp
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer $(GAME_SRC) host/sim.cpp -o $(BUILD)/snap-asan

# Scripted playthroughs, UI audit and monkey runs (tests/playtests/*.txt); report in build/playtest/report.md.
playtest: snap snap-asan
	python3 tools/playtest.py

test: $(BUILD) $(GAME_SRC) host/test_pet.cpp
	$(CXX) $(CXXFLAGS) $(filter-out src/game/game.cpp,$(GAME_SRC)) host/test_pet.cpp -o $(BUILD)/test_pet && $(BUILD)/test_pet

# App-specific static checks. ruff/shellcheck run from the repo-root pre-commit, not here.
lint: $(BUILD)
	python3 tools/check_content.py
	python3 tools/art.py --check
	$(CXX) $(CXXFLAGS) -Werror $(GAME_SRC) host/sim.cpp -o $(BUILD)/snap-werror

# What pre-commit runs: seconds, not minutes.
check: lint test

# Everything CI runs for this app except the firmware build.
ci: check playtest coverage

# Line coverage of src/game/*.cpp from test_pet plus the non-monkey playtests, as Cobertura XML.
# Separate objects keep the .gcno/.gcda names predictable for both gcc and clang. COV_MIN: baseline
# 93.9% (gcc 13, ubuntu-24.04) / 94.3% (Apple clang) at import, minus 2 points.
COV := build/coverage
COV_MIN := 91
COV_CXXFLAGS := -std=c++17 -O0 -g --coverage -Wno-unused-parameter -Isrc/game -DHOST_BUILD
COV_SCENARIOS := $(foreach f,$(wildcard tests/playtests/*.txt),$(if $(findstring monkey,$(f)),,$(basename $(notdir $(f)))))
GCOVR ?= $(shell command -v gcovr 2>/dev/null || echo uvx gcovr)
# ponytail: keyed on the OS, so it assumes Apple clang on macOS and gcc elsewhere; pass GCOV=... for other mixes.
ifeq ($(shell uname -s),Darwin)
GCOV ?= xcrun llvm-cov gcov
else
GCOV ?= gcov
endif

coverage:
	rm -rf $(COV) && mkdir -p $(COV)/obj
	for f in $(GAME_SRC) host/sim.cpp host/test_pet.cpp; do $(CXX) $(COV_CXXFLAGS) -c $$f -o $(COV)/obj/$$(basename $$f .cpp).o || exit 1; done
	$(CXX) --coverage $(patsubst src/game/%.cpp,$(COV)/obj/%.o,$(filter-out src/game/game.cpp,$(GAME_SRC))) $(COV)/obj/test_pet.o -o $(COV)/test_pet
	$(CXX) --coverage $(patsubst src/game/%.cpp,$(COV)/obj/%.o,$(GAME_SRC)) $(COV)/obj/sim.o -o $(COV)/snap
	$(COV)/test_pet
	PLAYTEST_SNAP=$(COV)/snap PLAYTEST_OUT=$(COV)/playtest python3 tools/playtest.py $(COV_SCENARIOS)
	$(GCOVR) --root . --filter 'src/game/.*\.cpp$$' --gcov-executable "$(GCOV)" --cobertura $(COV)/coverage.xml \
	  --txt --print-summary --fail-under-line $(COV_MIN) $(COV)/obj

art:
	python3 tools/art.py

webemu: snap
	python3 tools/webemu.py

# The game in a browser: host/web.cpp compiled to WebAssembly with Emscripten, plus web/index.html.
PLAY := build/play
play: $(GAME_SRC) host/web.cpp web/index.html
	@command -v em++ >/dev/null || { echo "em++ not found: install Emscripten (brew install emscripten)" >&2; exit 1; }
	mkdir -p $(PLAY)
	em++ -std=c++17 -O2 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Isrc/game \
	  $(GAME_SRC) host/web.cpp -sENVIRONMENT=web -o $(PLAY)/pets-club.js
	cp web/index.html $(PLAY)/index.html

serve-play: play
	python3 -m http.server -d $(PLAY) 8200

firmware:
	pio run -e firmware

factory:
	pio run -e firmware -t factory

flash:
	pio run -e firmware -t upload $(if $(PORT),--upload-port $(PORT))

monitor:
	pio device monitor -b 115200 $(if $(PORT),-p $(PORT))

clean:
	rm -rf build

.PHONY: all sim snap snap-asan playtest test lint check ci coverage art webemu play serve-play firmware factory flash monitor clean
