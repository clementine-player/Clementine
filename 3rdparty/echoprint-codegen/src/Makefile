UNAME := $(shell uname -s)
CXX=g++
CC=gcc
ARCH=`uname -m`
#OPTFLAGS=-g -O0
OPTFLAGS=-O3 -DBOOST_UBLAS_NDEBUG -DNDEBUG
CXXFLAGS=-Wall -I/usr/local/include/boost-1_35 `taglib-config --cflags` -fPIC $(OPTFLAGS)
CFLAGS=-Wall -fPIC $(OPTFLAGS)
LDFLAGS=`taglib-config --libs` -lz -lpthread $(OPTFLAGS)

MODULES_LIB = \
    AudioBufferInput.o \
    AudioStreamInput.o \
    Base64.o \
    Codegen.o \
    Fingerprint.o \
    MatrixUtility.o \
    SubbandAnalysis.o \
    Whitening.o
MODULES = $(MODULES_LIB) Metadata.o

main: $(MODULES) main.o
	$(CXX) $(MODULES) $(LDFLAGS) main.o -o ../echoprint-codegen 
	$(CXX) -shared -fPIC -o libcodegen.so $(MODULES_LIB) -lz
ifeq ($(UNAME),Darwin)
	libtool -dynamic -flat_namespace -install_name libcodegen.4.1.1.dylib -lSystem -compatibility_version 4.1 -macosx_version_min 10.6 \
	    -current_version 4.1.1 -o libcodegen.4.1.1.dylib -undefined suppress \
	    $(MODULES) -framework vecLib -framework Accelerate
endif

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cxx %.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f *.o ../echoprint-codegen
	rm -f *.so
ifeq ($(UNAME),Darwin)
	rm -f *.dylib
endif
