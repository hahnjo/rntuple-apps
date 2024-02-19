ROOT_CONFIG = $(shell which root-config)
ifeq ($(ROOT_CONFIG),)
$(error Could not find root-config)
endif
ROOT_CFLAGS = $(shell $(ROOT_CONFIG) --cflags)
ROOT_LIBS = $(shell $(ROOT_CONFIG) --libs)

OPTS = -O2 -g

skim: skim.cxx
	$(CXX) $(OPTS) $< -o $@ $(ROOT_CFLAGS) $(ROOT_LIBS) -lROOTNTuple
