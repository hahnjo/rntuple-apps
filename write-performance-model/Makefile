ROOT_CONFIG = $(shell which root-config)
ifeq ($(ROOT_CONFIG),)
$(error Could not find root-config)
endif
ROOT_CFLAGS = $(shell $(ROOT_CONFIG) --cflags)
ROOT_LIBS = $(shell $(ROOT_CONFIG) --libs)

OPTS = -O2 -g

all: analyze benchmark cms-Muon libMuon.so serialize skim-PHYSLITE

%: %.cxx
	$(CXX) $(OPTS) $< -o $@ $(ROOT_CFLAGS) $(ROOT_LIBS) -lROOTNTuple

%.cxx: %.hxx %_LinkDef.h
	rootcling -f $@ $^

libMuon.so: Muon.cxx
	$(CXX) $(OPTS) -shared -fPIC $< -o $@ $(ROOT_CFLAGS) $(ROOT_LIBS)
