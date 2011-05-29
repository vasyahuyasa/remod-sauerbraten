########### CONFIGURATION ############

#Use geoip module?  true|false 
USE_GEOIP=true

#Use database?  none|mysql|postgres|sqlite
#NOT IMPLEMENTED YET
#USE_DATABASE=mysql

#Use irc bot? true|false
USE_IRCBOT=true

######################################

CXX= gcc
CXXFLAGS= -O3 -fomit-frame-pointer -Wall -fsigned-char 
 
override CXXFLAGS+= -g -DDEBUG # uncomment for debugging

PLATFORM= $(shell uname -s)
PLATFORM_PREFIX= native

INCLUDES= -Ishared -Iengine -Ifpsgame -Ienet/include -Imod 

#geoip
ifeq ($(USE_GEOIP),true)
override CXXFLAGS+= -DGEOIPDATADIR -DGEOIP
override INCLUDES+= -IlibGeoIP
endif

#irc
ifeq ($(USE_IRCBOT),true)
override CXXFLAGS+=  -DIRC
endif

#database
#ifeq ($(USE_DATABASE),mysql)
#override INCLUDES+= -Iinclude/db/mysql
#override CXXFLAGS+= -DDBMYSQL -DDATABASE -lmysqlclient
#endif
#ifeq ($(USE_DATABASE),postgres)
#override INCLUDES+= -Ipostgres
#override CXXFLAGS+= -DDBPOSTGRES -DDATABASE
#endif
#ifeq ($(USE_DATABASE),sqlite)
#override INCLUDES+= -Ipostgres
#override CXXFLAGS+= -DDBSQLITE -DDATABASE
#endif


STRIP=
ifeq (,$(findstring -g,$(CXXFLAGS)))
ifeq (,$(findstring -pg,$(CXXFLAGS)))
  STRIP=strip
endif
endif

MV=mv

ifneq (,$(findstring MINGW,$(PLATFORM)))
WINDRES= windres
endif

ifneq (,$(findstring MINGW,$(PLATFORM)))
SERVER_INCLUDES+= -DSTANDALONE $(INCLUDES) -Iinclude
SERVER_LIBS= -Llib -lzdll -lenet -lws2_32 -lwinmm 
else
SERVER_INCLUDES+= -DSTANDALONE $(INCLUDES)
SERVER_LIBS= -Lenet/.libs -L/usr/local/lib -lenet -lz -lstdc++
endif
SERVER_OBJS= \
	shared/crypto-standalone.o \
	shared/stream-standalone.o \
	shared/tools-standalone.o \
	engine/command-standalone.o \
	engine/server-standalone.o \
	fpsgame/server-standalone.o \
	mod/commandev-standalone.o \
	mod/commandhandler-standalone.o \
	mod/geoipmod-standalone.o \
	mod/irc-standalone.o \
	mod/rconmod-standalone.o \
	mod/serverctrl-standalone.o	\
	mod/remod-standalone.o \
	mod/remodex-standalone.o \
	libGeoIP/GeoIP-standalone.o

ifeq ($(PLATFORM),SunOS)
SERVER_LIBS+= -lsocket -lnsl
endif

default: all

all: server

enet/Makefile:
	cd enet; chmod +x configure; ./configure --enable-shared=no 
--enable-static=yes
	
libenet: enet/Makefile
	$(MAKE)	-C enet/ all

clean-enet: enet/Makefile
	$(MAKE) -C enet/ clean

clean:
	-$(RM) $(CLIENT_PCH) $(CLIENT_OBJS) $(SERVER_OBJS) $(MASTER_OBJS) sauer_client sauer_server sauer_master

%.h.gch: %.h
	$(CXX) $(CXXFLAGS) -o $@.tmp $(subst .h.gch,.h,$@)
	$(MV) $@.tmp $@

%-standalone.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst -standalone.o,.cpp,$@) 

%-standalone.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst -standalone.o,.c,$@)	

$(CLIENT_OBJS): CXXFLAGS += $(CLIENT_INCLUDES)
$(filter shared/%,$(CLIENT_OBJS)): $(filter shared/%,$(CLIENT_PCH))
$(filter engine/%,$(CLIENT_OBJS)): $(filter engine/%,$(CLIENT_PCH))
$(filter fpsgame/%,$(CLIENT_OBJS)): $(filter fpsgame/%,$(CLIENT_PCH))

$(SERVER_OBJS): CXXFLAGS += $(SERVER_INCLUDES)
$(filter-out $(SERVER_OBJS),$(MASTER_OBJS)): CXXFLAGS += $(SERVER_INCLUDES)

ifneq (,$(findstring MINGW,$(PLATFORM)))
vcpp/%.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst .o,.c,$@) 

server: $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o ../bin/sauer_server.exe $(SERVER_OBJS) $(SERVER_LIBS)

install: all
else
server:	libenet $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o sauer_server $(SERVER_OBJS) $(SERVER_LIBS)  
	
install: all
	cp sauer_server	../bin_unix/$(PLATFORM_PREFIX)_server
ifneq (,$(STRIP))
	$(STRIP) ../bin_unix/$(PLATFORM_PREFIX)_server
endif
endif
