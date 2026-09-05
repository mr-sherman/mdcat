# Convenience wrapper around the CMake/vcpkg build described in README.md.
#
# Override VCPKG_ROOT if vcpkg isn't cloned at ~/vcpkg, e.g.:
#   make static VCPKG_ROOT=/opt/vcpkg

VCPKG_ROOT      ?= $(HOME)/vcpkg
TOOLCHAIN       := $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
BUILD_DIR       := build
STATIC_BUILD_DIR := build-static
INSTALL_BIN_DIR := $(HOME)/.local/bin

.PHONY: all build static install clean

all: build

build:
	cmake -S . -B $(BUILD_DIR) -G Ninja \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR)

# Boost is already linked statically by vcpkg's default x64-linux triplet;
# what's left is the C++ runtime. Statically linking libstdc++/libgcc means
# the resulting binary only depends on libc/libm and the dynamic
# linker itself, so it can be copied to another Linux machine without
# worrying about a mismatched libstdc++ version.
static:
	cmake -S . -B $(STATIC_BUILD_DIR) -G Ninja \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++"
	cmake --build $(STATIC_BUILD_DIR)

install: static
	install -Dm755 $(STATIC_BUILD_DIR)/mdcat $(INSTALL_BIN_DIR)/mdcat
	@echo "Installed mdcat to $(INSTALL_BIN_DIR)"
	@echo "Add it to your PATH if it isn't already, e.g.:"
	@echo '  export PATH="$(INSTALL_BIN_DIR):$$PATH"'

clean:
	rm -rf $(BUILD_DIR) $(STATIC_BUILD_DIR)
