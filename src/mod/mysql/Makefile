CC = gcc
CXX = gcc
CXXFLAGS = -O3 -fno-exceptions -DDBUG_OFF
PLATFORM = $(shell uname -s)
CXXFLAGS += -Iinclude -Iextlib/regex -Iextlib/zlib -Imysys -Ivio

ifeq (,$(findstring MINGW,$(PLATFORM)))
LDFLAGS = -lpthread -lm
else
CXXFLAGS += -DNO_OLDNAMES
LDFLAGS = -lws2_32 #should be added at the end of command string
endif

OBJS =

#strings
STRINGS_DIR = strings
STRINGS_OBJS = bchange-standalone.o bcmp-standalone.o bfill-standalone.o bmove512-standalone.o bmove_upp-standalone.o \
				ctype-bin-standalone.o ctype-extra-standalone.o ctype-mb-standalone.o ctype-utf8-standalone.o ctype-latin1-standalone.o ctype-standalone.o ctype-simple-standalone.o \
				decimal-standalone.o dtoa-standalone.o int2str-standalone.o \
				is_prefix-standalone.o llstr-standalone.o longlong2str-standalone.o my_strtoll10-standalone.o my_vsnprintf-standalone.o r_strinstr-standalone.o \
				str2int-standalone.o str_alloc-standalone.o strcend-standalone.o strend-standalone.o strfill-standalone.o strmake-standalone.o strmov-standalone.o strnmov-standalone.o \
				strtol-standalone.o strtoll-standalone.o strtoul-standalone.o strtoull-standalone.o strxmov-standalone.o strxnmov-standalone.o xml-standalone.o \
				my_strchr-standalone.o strcont-standalone.o strinstr-standalone.o strnlen-standalone.o strappend-standalone.o

#extlib/regex
REGEX_DIR = extlib/regex
REGEX_OBJS = debug-standalone.o regcomp-standalone.o regerror-standalone.o regexec-standalone.o regfree-standalone.o reginit-standalone.o split-standalone.o

#extlib/zlib
ZLIB_DIR = extlib/zlib
ZLIB_OBJS = adler32-standalone.o compress-standalone.o crc32-standalone.o deflate-standalone.o gzio-standalone.o infback-standalone.o inffast-standalone.o inflate-standalone.o \
           inftrees-standalone.o trees-standalone.o uncompr-standalone.o  zutil-standalone.o

#mysys
MYSYS_DIR = mysys
MYSYS_OBJS = array-standalone.o charset-def-standalone.o charset-standalone.o checksum-standalone.o default-standalone.o default_modify-standalone.o \
			errors-standalone.o hash-standalone.o list-standalone.o md5-standalone.o mf_brkhant-standalone.o mf_cache-standalone.o mf_dirname-standalone.o mf_fn_ext-standalone.o \
			mf_format-standalone.o mf_getdate-standalone.o mf_iocache-standalone.o mf_iocache2-standalone.o mf_keycache-standalone.o my_safehash-standalone.o \
			mf_keycaches-standalone.o mf_loadpath-standalone.o mf_pack-standalone.o mf_path-standalone.o mf_qsort-standalone.o mf_qsort2-standalone.o \
			mf_radix-standalone.o mf_same-standalone.o mf_sort-standalone.o mf_soundex-standalone.o mf_strip-standalone.o mf_arr_appstr-standalone.o mf_tempdir-standalone.o \
			mf_tempfile-standalone.o mf_unixpath-standalone.o mf_wcomp-standalone.o mf_wfile-standalone.o mulalloc-standalone.o my_access-standalone.o \
			my_aes-standalone.o my_alarm-standalone.o my_alloc-standalone.o my_append-standalone.o my_bit-standalone.o my_bitmap-standalone.o my_chmod-standalone.o my_chsize-standalone.o \
			my_clock-standalone.o my_compress-standalone.o my_conio-standalone.o my_copy-standalone.o my_create-standalone.o my_delete-standalone.o \
			my_div-standalone.o my_error-standalone.o my_file-standalone.o my_fopen-standalone.o my_fstream-standalone.o my_gethostbyname-standalone.o \
			my_gethwaddr-standalone.o my_getopt-standalone.o my_getsystime-standalone.o my_getwd-standalone.o my_init-standalone.o \
			my_lib-standalone.o my_lock-standalone.o my_lockmem-standalone.o my_malloc-standalone.o my_messnc-standalone.o my_dup-standalone.o \
			my_mkdir-standalone.o my_mmap-standalone.o my_net-standalone.o my_once-standalone.o my_open-standalone.o my_pread-standalone.o my_pthread-standalone.o \
			my_quick-standalone.o my_read-standalone.o my_realloc-standalone.o my_redel-standalone.o my_rename-standalone.o my_seek-standalone.o my_sleep-standalone.o \
			my_static-standalone.o my_symlink-standalone.o my_symlink2-standalone.o my_sync-standalone.o my_thr_init-standalone.o my_wincond-standalone.o \
			my_winerr-standalone.o my_winfile-standalone.o \
			my_windac-standalone.o my_winthread-standalone.o my_write-standalone.o ptr_cmp-standalone.o queues-standalone.o  \
			rijndael-standalone.o safemalloc-standalone.o sha1-standalone.o string-standalone.o thr_alarm-standalone.o thr_lock-standalone.o thr_mutex-standalone.o \
			thr_rwlock-standalone.o tree-standalone.o typelib-standalone.o my_vle-standalone.o base64-standalone.o my_memmem-standalone.o my_getpagesize-standalone.o \
			lf_alloc-pin-standalone.o lf_dynarray-standalone.o lf_hash-standalone.o \
			my_atomic-standalone.o my_getncpus-standalone.o my_rnd-standalone.o \
			my_uuid-standalone.o wqueue-standalone.o waiting_threads-standalone.o my_port-standalone.o

#vio
VIO_DIR = vio
VIO_OBJS = vio-standalone.o viosocket-standalone.o

#libmysql
LIBMYSQL_DIR = libmysql
LIBMYSQL_OBJS = client-standalone.o errmsg-standalone.o get_password-standalone.o libmysql-standalone.o my_time-standalone.o net_serv-standalone.o pack-standalone.o password-standalone.o

OBJS += $(foreach v,$(STRINGS_OBJS),$(STRINGS_DIR)/$(v))
OBJS += $(foreach v,$(REGEX_OBJS),$(REGEX_DIR)/$(v))
OBJS += $(foreach v,$(ZLIB_OBJS),$(ZLIB_DIR)/$(v))
OBJS += $(foreach v,$(MYSYS_OBJS),$(MYSYS_DIR)/$(v))
OBJS += $(foreach v,$(VIO_OBJS),$(VIO_DIR)/$(v))
OBJS += $(foreach v,$(LIBMYSQL_OBJS),$(LIBMYSQL_DIR)/$(v))


OBJS += test-standalone.o

default: $(OBJS)
	$(CXX) $(CXXFLAGS) -o test.exe $(OBJS) $(LDFLAGS)

all: default

%-standalone.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst -standalone.o,.c,$@)

clean:
	-$(RM) $(OBJS)
