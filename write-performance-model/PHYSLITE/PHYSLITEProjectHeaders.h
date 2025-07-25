#include "EventStreamInfo_p3.h"
#include "IOVMetaDataContainer_p1.h"
#include "xAOD__FileMetaDataAuxInfo_v1.h"
#include "xAOD__AuxInfoBase.h"
#include "SG__IAuxStore.h"
#include "SG__IConstAuxStore.h"
#include "SG__IAuxStoreIO.h"
#include "SG__IAuxStoreHolder.h"
#include "ILockable.h"
#include "xAOD__TruthMetaDataAuxContainer_v1.h"
#include "xAOD__AuxContainerBase.h"
#include "xAOD__CutBookkeeperAuxContainer_v1.h"
#include "ElementLink_xAOD__CutBookkeeperContainer_v1_.h"
#include "ElementLinkBase.h"
#include "xAOD__TriggerMenuAuxContainer_v1.h"
#include "xAOD__LumiBlockRangeAuxContainer_v1.h"
#include "EventType_p3.h"
#include "xAOD__FileMetaData_v1.h"
#include "SG__AuxElement.h"
#include "SG__IAuxElement.h"
#include "xAOD__EventFormat_v1.h"
#include "xAOD__TriggerMenu_v1.h"
#include "xAOD__LumiBlockRange_v1.h"
#include "xAOD__TruthMetaData_v1.h"
#include "xAOD__CutBookkeeper_v1.h"
#include "IOVPayloadContainer_p1.h"
#include "AttrListIndexes.h"
#include "DataHeader_p6.h"
#include "DataHeaderForm_p6.h"
#include "Guid.h"
#include "xAOD__EventInfo_v1.h"
#include "xAOD__TrigConfKeys_v1.h"
#include "xAOD__TrigDecisionAuxInfo_v1.h"
#include "xAOD__TrigDecision_v1.h"
#include "xAOD__MissingETAuxAssociationMap_v2.h"
#include "ElementLink_DataVector_xAOD__Jet_v1___.h"
#include "ElementLink_DataVector_xAOD__IParticle___.h"
#include "xAOD__MissingETAssociation_v1.h"
#include "xAOD__EventShape_v1.h"
#include "xAOD__Electron_v1.h"
#include "xAOD__Egamma_v1.h"
#include "xAOD__IParticle.h"
#include "xAOD__Vertex_v1.h"
#include "xAOD__MissingET_v1.h"
#include "xAOD__Photon_v1.h"
#include "xAOD__TauJet_v3.h"
#include "xAOD__Muon_v1.h"
#include "xAOD__TruthEvent_v1.h"
#include "xAOD__TruthEventBase_v1.h"
#include "xAOD__CaloCluster_v1.h"
#include "xAOD__TruthParticle_v1.h"
#include "xAOD__TruthVertex_v1.h"
#include "xAOD__Jet_v1.h"
#include "xAOD__BTagging_v1.h"
#include "xAOD__TrackParticle_v1.h"
#include "xAOD__JetAuxContainer_v1.h"
#include "xAOD__TauTrack_v1.h"
#include "xAOD__TrigComposite_v1.h"
#include "ElementLink_DataVector_xAOD__Vertex_v1___.h"
#include "ElementLink_DataVector_xAOD__TrackParticle_v1___.h"
#include "ElementLink_DataVector_xAOD__Egamma_v1___.h"
#include "ElementLink_DataVector_xAOD__TruthParticle_v1___.h"
#include "ElementLink_DataVector_xAOD__CaloCluster_v1___.h"
#include "ElementLink_DataVector_xAOD__BTagging_v1___.h"
#include "ElementLink_DataVector_xAOD__TauTrack_v1___.h"
#include "ElementLink_DataVector_xAOD__TruthVertex_v1___.h"
#include "ElementLink_DataVector_xAOD__MuonSegment_v1___.h"
#include "ElementLink_DataVector_xAOD__NeutralParticle_v1___.h"
#include "xAOD__CutBookkeeperContainer_v1.h"
#include "DataVector_xAOD__Jet_v1_.h"
#include "DataVector_xAOD__IParticle_.h"
#include "DataVector_xAOD__Vertex_v1_.h"
#include "DataVector_xAOD__TrackParticle_v1_.h"
#include "DataVector_xAOD__Egamma_v1_.h"
#include "DataVector_xAOD__TruthParticle_v1_.h"
#include "DataVector_xAOD__CaloCluster_v1_.h"
#include "DataVector_xAOD__BTagging_v1_.h"
#include "DataVector_xAOD__TauTrack_v1_.h"
#include "DataVector_xAOD__TruthVertex_v1_.h"
#include "xAOD__MuonSegment_v1.h"
#include "DataVector_xAOD__MuonSegment_v1_.h"
#include "xAOD__NeutralParticle_v1.h"
#include "DataVector_xAOD__NeutralParticle_v1_.h"
#include "PHYSLITEProjectInstances.h"
