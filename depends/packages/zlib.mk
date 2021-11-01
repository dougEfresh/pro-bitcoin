package=zlib
$(package)_version=1.2.11
$(package)_download_path=https://www.zlib.net/
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1

define $(package)_set_vars
$(package)_config_opts=--static
#$(package)_config_opts_linux=--64
#$(package)_config_opts_darwin=target-os=darwin runtime-link=shared
#$(package)_config_opts_mingw32=target-os=windows binary-format=pe threadapi=win32 runtime-link=static
#$(package)_config_opts_x86_64=architecture=x86 address-model=64
#$(package)_config_opts_i686=architecture=x86 address-model=32
#$(package)_config_opts_aarch64=address-model=64
#$(package)_config_opts_armv7a=address-model=32

endef

define $(package)_config_cmds
 ./configure --static --prefix=/
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  mkdir -p $($(package)_staging_prefix_dir)/include $($(package)_staging_prefix_dir)/lib $($(package)_staging_prefix_dir)/lib/pkgconfig &&\
  install *.h $($(package)_staging_prefix_dir)/include &&\
  install libz.a $($(package)_staging_prefix_dir)/lib &&\
  install *.pc $($(package)_staging_prefix_dir)/lib/pkgconfig
endef

