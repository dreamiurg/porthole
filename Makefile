# porthole: shared entry points for every app under apps/.
#
#   make check                  lint + fast tests for every app (what pre-commit runs)
#   make ci                     everything CI runs, minus firmware builds
#   make firmware APP=biscuit   build one app's firmware
#   make flash APP=pets-club [PORT=/dev/ttyUSB0]   build and flash it
#   make monitor [APP=...] [PORT=...]               serial console, 115200 baud
#
# Each app provides the same targets in its own Makefile (see docs/new-app.md).

APPS := $(notdir $(patsubst %/Makefile,%,$(wildcard apps/*/Makefile)))
LIZARD_ARGS := -C 15 -L 60 -a 5 -w -W whitelizard.txt \
	-x '*/generated/*' -x '*/node_modules/*' -x '*/.pio/*' -x '*/build/*' -x '*/sprites.h' -x '*/content*.h'

.PHONY: help check ci test lint coverage complexity firmware flash monitor hooks apps

help:
	@sed -n '2,10p' Makefile | sed 's/^# \{0,1\}//'
	@echo "apps: $(APPS)"

apps:
	@echo $(APPS)

check lint test coverage ci:
	@set -e; for app in $(APPS); do echo "==> $$app: make $@"; $(MAKE) --no-print-directory -C apps/$$app $@; done

complexity:
	lizard $(LIZARD_ARGS) apps

define need_app
	@test -n "$(APP)" || { echo "usage: make $@ APP=<one of: $(APPS)>"; exit 2; }
	@test -f apps/$(APP)/Makefile || { echo "unknown app '$(APP)'; apps: $(APPS)"; exit 2; }
endef

firmware flash:
	$(need_app)
	$(MAKE) --no-print-directory -C apps/$(APP) $@ PORT=$(PORT)

monitor:
	pio device monitor -b 115200 $(if $(PORT),-p $(PORT))

hooks:
	pre-commit install --hook-type pre-commit --hook-type pre-push
