ROOT_CONFIG = $(shell which root-config)
ifeq ($(ROOT_CONFIG),)
$(error Could not find root-config)
endif
ROOT_CFLAGS = $(shell $(ROOT_CONFIG) --cflags)
ROOT_LIBS = $(shell $(ROOT_CONFIG) --libs)

OPTS = -O2 -g

default: random-write-zmq

%.o: %.cxx
	$(CXX) $(OPTS) -c $< -o $@ $(ROOT_CFLAGS)

random-write-zmq.o RNTupleWriterZeroMQ.o: RNTupleWriterZeroMQ.hxx

random-write-zmq: random-write-zmq.o RNTupleWriterZeroMQ.o
	$(CXX) $^ -o $@ -lROOTNTuple $(ROOT_LIBS)
