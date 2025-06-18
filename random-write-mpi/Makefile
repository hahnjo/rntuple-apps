MPI_CXX = $(shell which mpic++)
ifeq ($(MPI_CXX),)
$(error Could not find mpic++)
endif

ROOT_CONFIG = $(shell which root-config)
ifeq ($(ROOT_CONFIG),)
$(error Could not find root-config)
endif
ROOT_CFLAGS = $(shell $(ROOT_CONFIG) --cflags)
ROOT_LIBS = $(shell $(ROOT_CONFIG) --libs)

OPTS = -O2 -g

default: random-write-mpi random-write-mpi-multiple

%.o: %.cxx
	$(MPI_CXX) $(OPTS) -c $< -o $@ $(ROOT_CFLAGS)

random-write-mpi.o random-write-mpi-multiple.o RNTupleWriterMPI.o: RNTupleWriterMPI.hxx

random-write-mpi: random-write-mpi.o RNTupleWriterMPI.o
	$(MPI_CXX) $^ -o $@ -lROOTNTuple $(ROOT_LIBS)

random-write-mpi-multiple: random-write-mpi-multiple.o RNTupleWriterMPI.o
	$(MPI_CXX) $^ -o $@ -lROOTNTuple $(ROOT_LIBS)
