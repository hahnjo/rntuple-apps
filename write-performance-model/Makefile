ROOT_CONFIG = $(shell which root-config)
ifeq ($(ROOT_CONFIG),)
$(error Could not find root-config)
endif
ROOT_CFLAGS = $(shell $(ROOT_CONFIG) --cflags)
ROOT_LIBS = $(shell $(ROOT_CONFIG) --libs)

OPTS = -O2 -g

all: analyze benchmark cms-Muon libAoS-SoA.so libMuon.so libPHYSLITE.so measure-AoS-SoA serialize skim-PHYSLITE

%: %.cxx
	$(CXX) $(OPTS) $< -o $@ $(ROOT_CFLAGS) $(ROOT_LIBS) -lROOTNTuple

%.cxx: %.hxx %_LinkDef.h
	rootcling -f $@ $^

PHYSLITEProjectDict.cxx: PHYSLITE/PHYSLITEProjectHeaders.h PHYSLITE/PHYSLITELinkDef.h
	rootcling -f $@ $^

lib%.so: %.cxx
	$(CXX) $(OPTS) -shared -fPIC $< -o $@ $(ROOT_CFLAGS) $(ROOT_LIBS)

libPHYSLITE.so: PHYSLITE/PHYSLITEProjectSource.cxx PHYSLITEProjectDict.cxx
	$(CXX) $(OPTS) -shared -fPIC $< -o $@ $(ROOT_CFLAGS) -I. $(ROOT_LIBS)
