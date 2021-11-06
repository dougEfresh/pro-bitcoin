package=prometheus-cpp
$(package)_version=0.12.3
$(package)_download_path=https://github.com/jupp0r/prometheus-cpp/releases/download/v$($(package)_version)/
$(package)_file_name=$(package)-with-submodules.tar.gz
$(package)_sha256_hash=8e6e69b125c6ac60f573914e4246aa4b697598d2a225171719b895bc8963d651

define $(package)_set_vars
  $(package)_cxxflags=-std=c++17 -O2 -fPIC
endef

define $(package)_config_cmds
 $($(package)_cmake) -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_FLAGS='$(($package)_cxxflags)' -DCMAKE_C_FLAGS='-O2 -g -fPIC' -DENABLE_COMPRESSION=OFF -DENABLE_PUSH=OFF -DBUILD_SHARED_LIBS=OFF -DENABLE_TESTING=OFF
# $($(package)_cmake) -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_COMPILER='clang++' -DCMAKE_C_COMPILER=clang   -DCMAKE_CXX_FLAGS='$(($package)_cxxflags)' -DCMAKE_C_FLAGS='-O2 -g -fPIC' -DENABLE_COMPRESSION=OFF -DENABLE_PUSH=OFF -DBUILD_SHARED_LIBS=OFF -DENABLE_TESTING=OFF
endef

define $(package)_build_cmds
 cmake --build . --parallel 4
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

