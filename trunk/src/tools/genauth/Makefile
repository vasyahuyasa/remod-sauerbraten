CXX=g++
LD=ld

CXXFLAGS = -s -O2 -DSTANDALONE
INCLUDES = -I../../shared -I../../engine -I../../fpsgame -I../../enet/include

OBJS= genauth-standalone.o \
	../../shared/crypto-standalone.o

default: all

all: $(OBJS) link

%-standalone.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $(subst -standalone.o,.cpp,$@) 

clean:
	rm $(OBJS)

link:
	$(CXX)  $(OBJS) $(LIBS) -o genauth
	strip genauth
