CXX=g++
LD=ld

CXXFLAGS = -s -O2 -DSTANDALONE
LIBS =  -lz
INCLUDES = -I../../shared -I../../engine -I../../fpsgame -I../../enet/include

OBJS= mkents-standalone.o \
	   ../../fpsgame/entities-standalone.o \
	   ../../shared/stream-standalone.o

default: all

all: $(OBJS) link

%-standalone.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $(subst -standalone.o,.cpp,$@) 

clean:
	rm $(OBJS)

link:
	$(CXX)  $(OBJS) $(LIBS) -o mkents
	strip mkents
