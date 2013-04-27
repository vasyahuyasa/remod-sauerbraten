CXX=g++
STRIP=strip
PROG=authgen

CXXFLAGS = -s -O2 -DSTANDALONE
INCLUDES = -I../../shared -I../../engine -I../../fpsgame -I../../enet/include
LIBS = 

OBJS= authgen-standalone.o \
	../../shared/crypto-standalone.o

default: all

win32-args:
override CXX = i486-mingw32-g++
override STRIP = i486-mingw32-strip
override PROG = authgen.exe
INCLUDES += -I../../include
override LIBS = -static

win32: win32-args all

all: $(OBJS) link

%-standalone.o: %.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) $(INCLUDES) -c -o $@ $(subst -standalone.o,.cpp,$@) 

clean:
	rm $(OBJS)

link:
	$(CXX)  $(OBJS) $(LIBS) -o $(PROG)
	$(STRIP) $(PROG)
