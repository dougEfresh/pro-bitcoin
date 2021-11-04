package=prometheus-cpp
$(package)_version=0.12.3
$(package)_download_path=https://github.com/jupp0r/prometheus-cpp/releases/download/v$($(package)_version)/
$(package)_file_name=$(package)-with-submodules.tar.gz
$(package)_sha256_hash=8e6e69b125c6ac60f573914e4246aa4b697598d2a225171719b895bc8963d651
$(package)_dependencies=zlib

define $(package)_set_vars
  $(package)_config_opts=--without-docs --disable-shared --disable-curve --disable-curve-keygen --disable-perf
  $(package)_config_opts += --without-libsodium --without-libgssapi_krb5 --without-pgm --without-norm --without-vmci
  $(package)_config_opts += --disable-libunwind --disable-radix-tree --without-gcov --disable-dependency-tracking
  $(package)_config_opts += --disable-Werror --disable-drafts --enable-option-checking
  $(package)_config_opts_linux=--with-pic
  $(package)_config_opts_android=--with-pic
  $(package)_cxxflags=-std=c++17
endef

define $(package)_config_cmds
  $($(package)_cmake)
endef

#define $(package)_config_cmds
#  mkdir _build && cd _build && cmake .. -DCMAKE_BUILD_TYPE="Release"  -DCMAKE_INSTALL_PREFIX=$($(package)_staging_dir)
#endef

define $(package)_build_cmds
 cmake --build .
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

