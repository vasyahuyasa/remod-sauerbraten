USE_GEOIP=true
USE_IRCBOT=true
USE_SQLITE3=true
SQLITE3_USE_SYSTEM_LIBS=false
USE_MYSQL=true
MYSQL_USE_SYSTEM_LIBS=false
USE_EXPEREMENTAL_DISCORD=true

CXXFLAGS=-O3 -fomit-frame-pointer -DSTANDALONE
override CXXFLAGS+= -Wall -fsigned-char -fno-exceptions -fno-rtti

INCLUDES= -Ienet/include -Ishared -Iengine -Ifpsgame -Imod -Imod/hashlib2plus/src -Imod/cryptomod/polarssl/include

STRIP?=strip

PLATFORM?=x86_64-pc-linux-gnu

ifneq (,$(findstring MINGW,$(PLATFORM)))
    ifneq (,$(findstring 64,$(PLATFORM)))
        WINDRES=x86_64-w64-mingw32-windres
        STRIP=x86_64-w64-mingw32-strip
        WINLIB=lib64
		override CXX+= -m64
        override WINDRES+= -F pe-x86-64
    else
        WINDRES=i686-w64-mingw32-windres
        STRIP=i686-w64-mingw32-strip
        WINLIB=lib
        override CXX+= -m32
        override WINDRES+= -F pe-i386
    endif
    STD_LIBS=-static-libgcc -static-libstdc++
endif

#result file name
OUTPUT_FILE=remod
ifneq (,$(findstring 64,$(shell uname -a)))
OUTPUT_FILE=remod64
endif
ifneq (,$(findstring MINGW,$(PLATFORM)))
OUTPUT_FILE+=.exe
endif

ifneq (,$(findstring MINGW,$(PLATFORM)))
SERVER_INCLUDES= -DSTANDALONE $(INCLUDES) -Iinclude
SERVER_LIBS= -mwindows $(STD_LIBS) -L$(WINBIN) -L$(WINLIB) -lzlib1 -lenet -lws2_32 -lwinmm
else
SERVER_INCLUDES= -DSTANDALONE $(INCLUDES)
SERVER_LIBS= -Lenet -lenet -lz
endif

SERVER_OBJS= \
	shared/crypto-standalone.o \
	shared/stream-standalone.o \
	shared/tools-standalone.o \
	engine/worldio-standalone.o \
	engine/command-standalone.o \
	engine/server-standalone.o \
	fpsgame/entities-standalone.o \
	fpsgame/server-standalone.o \
	mod/commandev-standalone.o \
	mod/commandhandler-standalone.o \
	mod/serverctrl-standalone.o \
	mod/remod-standalone.o \
	mod/rconmod-standalone.o \
	mod/rconmod_tcp-standalone.o \
	mod/rconmod_udp-standalone.o \
	mod/varbox-standalone.o \
	mod/cryptomod-standalone.o \
	mod/cryptomod/polarssl/library/md5-standalone.o \
	mod/cryptomod/polarssl/library/sha1-standalone.o \
	mod/cryptomod/polarssl/library/sha2-standalone.o \
	mod/cryptomod/polarssl/library/sha4-standalone.o \
	mod/banlist-standalone.o

# geoip
ifeq ($(USE_GEOIP),true)
override INCLUDES+= -Imod/libmaxminddb/include -Imod/libGeoIP/libGeoIP
override SERVER_LIBS+= -Lmod/libmaxminddb/src/.libs -Lmod/libGeoIP/libGeoIP/.libs -lmaxminddb -lGeoIP
override SERVER_OBJS+= mod/geoipmod-standalone.o
endif
# end geoip

#irc
ifeq ($(USE_IRCBOT),true)
override CXXFLAGS+= -DIRC
override SERVER_OBJS+= mod/irc-standalone.o
endif
#end of irc

#use db
ifeq ($(USE_SQLITE3),true)
override SERVER_OBJS+= mod/db-standalone.o
else 
ifeq ($(USE_MYSQL),true) 
override SERVER_OBJS+= mod/db-standalone.o
endif
endif
#end of use db

#sqlite3
ifeq ($(USE_SQLITE3),true)
override CXXFLAGS+= -DSQLITE3

ifeq ($(SQLITE3_USE_SYSTEM_LIBS),true)
override SERVER_LIBS+= -lsqlite3
else
override CXXFLAGS+= -DSQLITE_THREADSAFE=1
override INCLUDES+= -Imod/sqlite3

#linux-only libs
ifneq (,$(findstring linux,$(PLATFORM)))
override SERVER_LIBS+= -ldl -lpthread
endif
override SERVER_OBJS+= mod/sqlite3/sqlite3-standalone.o
endif

override SERVER_OBJS+= mod/sqlite3-standalone.o
endif
#end of sqlite3

#mysql
ifeq ($(USE_MYSQL),true)

ifeq ($(MYSQL_USE_SYSTEM_LIBS),true)
override INCLUDES += -I/usr/include/mysql
override SERVER_LIBS += -lmysqlclient
else
override CXXFLAGS+= -DUSE_MYSQL -DDBUG_OFF
override INCLUDES += -Imod/mysql/include -Imod/mysql/extlib/regex -Imod/mysql/mysys -Imod/mysql/vio

ifeq (,$(findstring MINGW,$(PLATFORM)))
override SERVER_LIBS += -lpthread -lm
else
override CXXFLAGS += -DNO_OLDNAMES
override SERVER_LIBS += -lws2_32 
endif
MYSQL_STRINGS_DIR = strings
MYSQL_STRINGS_OBJS = bchange-standalone.o bcmp-standalone.o bfill-standalone.o bmove512-standalone.o bmove_upp-standalone.o \
				ctype-bin-standalone.o ctype-extra-standalone.o ctype-mb-standalone.o ctype-utf8-standalone.o ctype-latin1-standalone.o ctype-standalone.o ctype-simple-standalone.o \
				decimal-standalone.o dtoa-standalone.o int2str-standalone.o \
				is_prefix-standalone.o llstr-standalone.o longlong2str-standalone.o my_strtoll10-standalone.o my_vsnprintf-standalone.o r_strinstr-standalone.o \
				str2int-standalone.o str_alloc-standalone.o strcend-standalone.o strend-standalone.o strfill-standalone.o strmake-standalone.o strmov-standalone.o strnmov-standalone.o \
				strtol-standalone.o strtoll-standalone.o strtoul-standalone.o strtoull-standalone.o strxmov-standalone.o strxnmov-standalone.o xml-standalone.o \
				my_strchr-standalone.o strcont-standalone.o strinstr-standalone.o strnlen-standalone.o strappend-standalone.o
MYSQL_REGEX_DIR = extlib/regex
MYSQL_REGEX_OBJS = debug-standalone.o regcomp-standalone.o regerror-standalone.o regexec-standalone.o regfree-standalone.o reginit-standalone.o split-standalone.o
MYSQL_MYSYS_DIR = mysys
MYSQL_MYSYS_OBJS = array-standalone.o charset-def-standalone.o charset-standalone.o checksum-standalone.o default-standalone.o default_modify-standalone.o \
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
			my_atomic-standalone.o my_getncpus-standalone.o my_rnd-standalone.o \
			my_uuid-standalone.o wqueue-standalone.o  my_port-standalone.o
MYSQL_VIO_DIR = vio
MYSQL_VIO_OBJS = vio-standalone.o viosocket-standalone.o
MYSQL_LIBMYSQL_DIR = libmysql
MYSQL_LIBMYSQL_OBJS = client-standalone.o errmsg-standalone.o get_password-standalone.o libmysql-standalone.o my_time-standalone.o net_serv-standalone.o pack-standalone.o password-standalone.o

override SERVER_OBJS += $(foreach v,$(MYSQL_STRINGS_OBJS),mod/mysql/$(MYSQL_STRINGS_DIR)/$(v))
override SERVER_OBJS += $(foreach v,$(MYSQL_REGEX_OBJS),mod/mysql/$(MYSQL_REGEX_DIR)/$(v))
override SERVER_OBJS += $(foreach v,$(MYSQL_MYSYS_OBJS),mod/mysql/$(MYSQL_MYSYS_DIR)/$(v))
override SERVER_OBJS += $(foreach v,$(MYSQL_VIO_OBJS),mod/mysql/$(MYSQL_VIO_DIR)/$(v))
override SERVER_OBJS += $(foreach v,$(MYSQL_LIBMYSQL_OBJS),mod/mysql/$(MYSQL_LIBMYSQL_DIR)/$(v))
endif
override SERVER_OBJS += mod/mysql-standalone.o
endif
#end of mysql

default: release

release: release-flags 
release-flags: override CXXFLAGS+=-O2 -s
release-flags: build

debug: debug-flags 
debug-flags: override CXXFLAGS+=-O0 -g -DDEBUG -DDEBUG_SQL
debug-flags: build

build: server

enet/libenet.a:
	$(MAKE)	-C enet

libenet: enet/libenet.a

clean-enet:
	$(MAKE) -C enet clean

mod/libmaxminddb/Makefile:
	cd mod/libmaxminddb; autoreconf -f -i; ./configure --disable-shared --disable-binaries --disable-tests --host=$(PLATFORM)

mod/libmaxminddb/src/.libs/libmaxminddb.a: mod/libmaxminddb/Makefile
	$(MAKE)	-C mod/libmaxminddb

libmaxminddb: mod/libmaxminddb/src/.libs/libmaxminddb.a

clean-maxminddb: mod/libmaxminddb/Makefile
	$(MAKE) -C mod/libmaxminddb clean

mod/libGeoIP/Makefile:
	cd mod/libGeoIP; autoreconf -f -i; ./configure --disable-shared --host=$(PLATFORM)

mod/libGeoIP/libGeoIP/.libs/libGeoIP.a: mod/libGeoIP/Makefile
	$(MAKE)	-C mod/libGeoIP

libgeoip: mod/libGeoIP/libGeoIP/.libs/libGeoIP.a

clean-libgeoip: mod/libGeoIP/Makefile
	$(MAKE)	-C mod/libGeoIP/ clean

clean: clean-libgeoip clean-maxminddb clean-enet
	-$(RM) $(SERVER_OBJS) 

%-standalone.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst -standalone.o,.cpp,$@) 

%-standalone.o: %.c
	$(CC) $(CXXFLAGS) -c -o $@ $(subst -standalone.o,.c,$@)	

res/remod.res:
	$(WINDRES) -I res -i res/mingw.rc -J rc -o res/remod.res -O coff

servericon: res/remod.res

$(SERVER_OBJS): CXXFLAGS += $(SERVER_INCLUDES)

ifneq (,$(findstring MINGW,$(PLATFORM)))
server:	libenet libgeoip libmaxminddb servericon $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o ../$(OUTPUT_FILE) res/remod.res $(SERVER_OBJS) $(SERVER_LIBS)
else
server:	libenet libgeoip libmaxminddb $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o ../$(OUTPUT_FILE) $(SERVER_OBJS) $(SERVER_LIBS)
endif
