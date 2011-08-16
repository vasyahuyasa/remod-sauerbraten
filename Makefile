########### CONFIGURATION ############

#Use geoip module?  true|false 
USE_GEOIP=true

#Use irc bot? true|false
USE_IRCBOT=true

#Sqlite3 support? true|false
USE_SQLITE3=true

######################################

### Tools
CXX=gcc
MV=mv
STRIP=
ifeq (,$(findstring -g,$(CXXFLAGS)))
ifeq (,$(findstring -pg,$(CXXFLAGS)))
  STRIP=strip
endif
endif

ifneq (,$(findstring MINGW,$(PLATFORM)))
WINDRES= windres
endif

### Folders, libraries, includes
ifneq (,$(findstring MINGW,$(PLATFORM)))
SERVER_INCLUDES+= -DSTANDALONE $(INCLUDES) -Iinclude
SERVER_LIBS= -Llib -lzdll -lenet -lws2_32 -lwinmm 
else
SERVER_INCLUDES+= -DSTANDALONE $(INCLUDES)
SERVER_LIBS= -Lenet/.libs -L/usr/local/lib -lenet -lz -lstdc++
endif

ifeq ($(PLATFORM),SunOS)
SERVER_LIBS+= -lsocket -lnsl
endif

CXXFLAGS= -O0 -fomit-frame-pointer -Wall -fsigned-char -DSTANDALONE
override CXXFLAGS+= -g -DDEBUG # uncomment for debugging

PLATFORM= $(shell uname -s)
PLATFORM_PREFIX= native

INCLUDES= -Ishared -Iengine -Ifpsgame -Ienet/include -Imod 

SERVER_OBJS= \
	shared/crypto-standalone.o \
	shared/stream-standalone.o \
	shared/tools-standalone.o \
	engine/command-standalone.o \
	engine/server-standalone.o \
	fpsgame/server-standalone.o \
	mod/commandev-standalone.o \
	mod/commandhandler-standalone.o \
	mod/rconmod-standalone.o \
	mod/serverctrl-standalone.o \
	mod/remod-standalone.o

### Options checks

#geoip
ifeq ($(USE_GEOIP),true)
override CXXFLAGS+= -DGEOIPDATADIR -DGEOIP
override INCLUDES+= -Imod/libGeoIP
override SERVER_OBJS+= mod/geoipmod-standalone.o mod/libGeoIP/GeoIP-standalone.o
endif

#irc
ifeq ($(USE_IRCBOT),true)
override CXXFLAGS+= -DIRC
override SERVER_OBJS+= mod/irc-standalone.o
endif

#sqlite3
ifeq ($(USE_SQLITE3),true)
override CXXFLAGS+= -DSQLITE3
override INCLUDES+= -Imod/sqlite3

#linux-only libs
ifeq ($(PLATFORM),Linux)
override SERVER_LIBS+= -ldl -lpthread
endif

override SERVER_OBJS+= mod/sqlite3/sqlite3-standalone.o mod/sqlite3-standalone.o
endif

default: all

all: revision server

revision:
SVNVERSION= $(shell svnversion -cn . 2>/dev/null | sed -e "s/.*://" -e "s/\([0-9]*\).*/\1/" | grep "[0-9]") 
ifneq "$(SVNVERSION)" " "
override CXXFLAGS+= -DREMOD_VERSION="\"SVN build rev: $(SVNVERSION)\""
endif

enet/Makefile:
	cd enet; chmod +x configure; ./configure --enable-shared=no --enable-static=yes
	
libenet: enet/Makefile
	$(MAKE)	-C enet/ all

clean-enet: enet/Makefile
	$(MAKE) -C enet/ clean

clean:
	-$(RM) $(CLIENT_PCH) $(CLIENT_OBJS) $(SERVER_OBJS) $(MASTER_OBJS) sauer_client sauer_server

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
