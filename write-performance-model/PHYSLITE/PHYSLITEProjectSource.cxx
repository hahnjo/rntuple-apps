namespace std {}
using namespace std;
#include "PHYSLITEProjectHeaders.h"

#include "PHYSLITELinkDef.h"

#include "PHYSLITEProjectDict.cxx"

struct DeleteObjectFunctor {
   template <typename T>
   void operator()(const T *ptr) const {
      delete ptr;
   }
   template <typename T, typename Q>
   void operator()(const std::pair<T,Q> &) const {
      // Do nothing
   }
   template <typename T, typename Q>
   void operator()(const std::pair<T,Q*> &ptr) const {
      delete ptr.second;
   }
   template <typename T, typename Q>
   void operator()(const std::pair<T*,Q> &ptr) const {
      delete ptr.first;
   }
   template <typename T, typename Q>
   void operator()(const std::pair<T*,Q*> &ptr) const {
      delete ptr.first;
      delete ptr.second;
   }
};

#ifndef EventStreamInfo_p3_cxx
#define EventStreamInfo_p3_cxx
EventStreamInfo_p3::EventStreamInfo_p3() {
}
EventStreamInfo_p3 &EventStreamInfo_p3::operator=(const EventStreamInfo_p3 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_numberOfEvents = (const_cast<EventStreamInfo_p3 &>( rhs ).m_numberOfEvents);
   m_runNumbers = (const_cast<EventStreamInfo_p3 &>( rhs ).m_runNumbers);
   m_lumiBlockNumbers = (const_cast<EventStreamInfo_p3 &>( rhs ).m_lumiBlockNumbers);
   m_processingTags = (const_cast<EventStreamInfo_p3 &>( rhs ).m_processingTags);
   m_itemList = (const_cast<EventStreamInfo_p3 &>( rhs ).m_itemList);
   m_eventTypes = (const_cast<EventStreamInfo_p3 &>( rhs ).m_eventTypes);
   EventStreamInfo_p3 &modrhs = const_cast<EventStreamInfo_p3 &>( rhs );
   modrhs.m_runNumbers.clear();
   modrhs.m_lumiBlockNumbers.clear();
   modrhs.m_processingTags.clear();
   modrhs.m_itemList.clear();
   modrhs.m_eventTypes.clear();
   return *this;
}
EventStreamInfo_p3::EventStreamInfo_p3(const EventStreamInfo_p3 & rhs)
   : m_numberOfEvents(const_cast<EventStreamInfo_p3 &>( rhs ).m_numberOfEvents)
   , m_runNumbers(const_cast<EventStreamInfo_p3 &>( rhs ).m_runNumbers)
   , m_lumiBlockNumbers(const_cast<EventStreamInfo_p3 &>( rhs ).m_lumiBlockNumbers)
   , m_processingTags(const_cast<EventStreamInfo_p3 &>( rhs ).m_processingTags)
   , m_itemList(const_cast<EventStreamInfo_p3 &>( rhs ).m_itemList)
   , m_eventTypes(const_cast<EventStreamInfo_p3 &>( rhs ).m_eventTypes)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   EventStreamInfo_p3 &modrhs = const_cast<EventStreamInfo_p3 &>( rhs );
   modrhs.m_runNumbers.clear();
   modrhs.m_lumiBlockNumbers.clear();
   modrhs.m_processingTags.clear();
   modrhs.m_itemList.clear();
   modrhs.m_eventTypes.clear();
}
EventStreamInfo_p3::~EventStreamInfo_p3() {
}
#endif // EventStreamInfo_p3_cxx

#ifndef IOVMetaDataContainer_p1_cxx
#define IOVMetaDataContainer_p1_cxx
IOVMetaDataContainer_p1::IOVMetaDataContainer_p1() {
}
IOVMetaDataContainer_p1 &IOVMetaDataContainer_p1::operator=(const IOVMetaDataContainer_p1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_folderName = (const_cast<IOVMetaDataContainer_p1 &>( rhs ).m_folderName);
   m_folderDescription = (const_cast<IOVMetaDataContainer_p1 &>( rhs ).m_folderDescription);
   m_payload = (const_cast<IOVMetaDataContainer_p1 &>( rhs ).m_payload);
   IOVMetaDataContainer_p1 &modrhs = const_cast<IOVMetaDataContainer_p1 &>( rhs );
   modrhs.m_folderName.clear();
   modrhs.m_folderDescription.clear();
   return *this;
}
IOVMetaDataContainer_p1::IOVMetaDataContainer_p1(const IOVMetaDataContainer_p1 & rhs)
   : m_folderName(const_cast<IOVMetaDataContainer_p1 &>( rhs ).m_folderName)
   , m_folderDescription(const_cast<IOVMetaDataContainer_p1 &>( rhs ).m_folderDescription)
   , m_payload(const_cast<IOVMetaDataContainer_p1 &>( rhs ).m_payload)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   IOVMetaDataContainer_p1 &modrhs = const_cast<IOVMetaDataContainer_p1 &>( rhs );
   modrhs.m_folderName.clear();
   modrhs.m_folderDescription.clear();
}
IOVMetaDataContainer_p1::~IOVMetaDataContainer_p1() {
}
#endif // IOVMetaDataContainer_p1_cxx

#ifndef xAOD__FileMetaDataAuxInfo_v1_cxx
#define xAOD__FileMetaDataAuxInfo_v1_cxx
xAOD::FileMetaDataAuxInfo_v1::FileMetaDataAuxInfo_v1() {
}
xAOD::FileMetaDataAuxInfo_v1 &xAOD::FileMetaDataAuxInfo_v1::operator=(const FileMetaDataAuxInfo_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::AuxInfoBase::operator=(const_cast<FileMetaDataAuxInfo_v1 &>( rhs ));
   productionRelease = (const_cast<FileMetaDataAuxInfo_v1 &>( rhs ).productionRelease);
   dataType = (const_cast<FileMetaDataAuxInfo_v1 &>( rhs ).dataType);
   runNumbers = (const_cast<FileMetaDataAuxInfo_v1 &>( rhs ).runNumbers);
   lumiBlocks = (const_cast<FileMetaDataAuxInfo_v1 &>( rhs ).lumiBlocks);
   FileMetaDataAuxInfo_v1 &modrhs = const_cast<FileMetaDataAuxInfo_v1 &>( rhs );
   modrhs.productionRelease.clear();
   modrhs.dataType.clear();
   modrhs.runNumbers.clear();
   modrhs.lumiBlocks.clear();
   return *this;
}
xAOD::FileMetaDataAuxInfo_v1::FileMetaDataAuxInfo_v1(const FileMetaDataAuxInfo_v1 & rhs)
   : xAOD::AuxInfoBase(const_cast<FileMetaDataAuxInfo_v1 &>( rhs ))
   , productionRelease(const_cast<FileMetaDataAuxInfo_v1 &>( rhs ).productionRelease)
   , dataType(const_cast<FileMetaDataAuxInfo_v1 &>( rhs ).dataType)
   , runNumbers(const_cast<FileMetaDataAuxInfo_v1 &>( rhs ).runNumbers)
   , lumiBlocks(const_cast<FileMetaDataAuxInfo_v1 &>( rhs ).lumiBlocks)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   FileMetaDataAuxInfo_v1 &modrhs = const_cast<FileMetaDataAuxInfo_v1 &>( rhs );
   modrhs.productionRelease.clear();
   modrhs.dataType.clear();
   modrhs.runNumbers.clear();
   modrhs.lumiBlocks.clear();
}
xAOD::FileMetaDataAuxInfo_v1::~FileMetaDataAuxInfo_v1() {
}
#endif // xAOD__FileMetaDataAuxInfo_v1_cxx

#ifndef xAOD__AuxInfoBase_cxx
#define xAOD__AuxInfoBase_cxx
xAOD::AuxInfoBase::AuxInfoBase() {
}
xAOD::AuxInfoBase &xAOD::AuxInfoBase::operator=(const AuxInfoBase & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::IAuxStore::operator=(const_cast<AuxInfoBase &>( rhs ));
   SG::IAuxStoreIO::operator=(const_cast<AuxInfoBase &>( rhs ));
   SG::IAuxStoreHolder::operator=(const_cast<AuxInfoBase &>( rhs ));
   ILockable::operator=(const_cast<AuxInfoBase &>( rhs ));
   return *this;
}
xAOD::AuxInfoBase::AuxInfoBase(const AuxInfoBase & rhs)
   : SG::IAuxStore(const_cast<AuxInfoBase &>( rhs ))
   , SG::IAuxStoreIO(const_cast<AuxInfoBase &>( rhs ))
   , SG::IAuxStoreHolder(const_cast<AuxInfoBase &>( rhs ))
   , ILockable(const_cast<AuxInfoBase &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::AuxInfoBase::~AuxInfoBase() {
}
#endif // xAOD__AuxInfoBase_cxx

#ifndef SG__IAuxStore_cxx
#define SG__IAuxStore_cxx
SG::IAuxStore::IAuxStore() {
}
SG::IAuxStore &SG::IAuxStore::operator=(const IAuxStore & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::IConstAuxStore::operator=(const_cast<IAuxStore &>( rhs ));
   return *this;
}
SG::IAuxStore::IAuxStore(const IAuxStore & rhs)
   : SG::IConstAuxStore(const_cast<IAuxStore &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
SG::IAuxStore::~IAuxStore() {
}
#endif // SG__IAuxStore_cxx

#ifndef SG__IConstAuxStore_cxx
#define SG__IConstAuxStore_cxx
SG::IConstAuxStore::IConstAuxStore() {
}
SG::IConstAuxStore &SG::IConstAuxStore::operator=(const IConstAuxStore & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   return *this;
}
SG::IConstAuxStore::IConstAuxStore(const IConstAuxStore & rhs)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
SG::IConstAuxStore::~IConstAuxStore() {
}
#endif // SG__IConstAuxStore_cxx

#ifndef SG__IAuxStoreIO_cxx
#define SG__IAuxStoreIO_cxx
SG::IAuxStoreIO::IAuxStoreIO() {
}
SG::IAuxStoreIO &SG::IAuxStoreIO::operator=(const IAuxStoreIO & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   return *this;
}
SG::IAuxStoreIO::IAuxStoreIO(const IAuxStoreIO & rhs)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
SG::IAuxStoreIO::~IAuxStoreIO() {
}
#endif // SG__IAuxStoreIO_cxx

#ifndef SG__IAuxStoreHolder_cxx
#define SG__IAuxStoreHolder_cxx
SG::IAuxStoreHolder::IAuxStoreHolder() {
}
SG::IAuxStoreHolder &SG::IAuxStoreHolder::operator=(const IAuxStoreHolder & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   return *this;
}
SG::IAuxStoreHolder::IAuxStoreHolder(const IAuxStoreHolder & rhs)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
SG::IAuxStoreHolder::~IAuxStoreHolder() {
}
#endif // SG__IAuxStoreHolder_cxx

#ifndef ILockable_cxx
#define ILockable_cxx
ILockable::ILockable() {
}
ILockable &ILockable::operator=(const ILockable & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   return *this;
}
ILockable::ILockable(const ILockable & rhs)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ILockable::~ILockable() {
}
#endif // ILockable_cxx

#ifndef xAOD__TruthMetaDataAuxContainer_v1_cxx
#define xAOD__TruthMetaDataAuxContainer_v1_cxx
xAOD::TruthMetaDataAuxContainer_v1::TruthMetaDataAuxContainer_v1() {
}
xAOD::TruthMetaDataAuxContainer_v1 &xAOD::TruthMetaDataAuxContainer_v1::operator=(const TruthMetaDataAuxContainer_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::AuxContainerBase::operator=(const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ));
   weightNames = (const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).weightNames);
   mcChannelNumber = (const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).mcChannelNumber);
   lhefGenerator = (const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).lhefGenerator);
   generators = (const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).generators);
   evgenProcess = (const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).evgenProcess);
   evgenTune = (const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).evgenTune);
   hardPDF = (const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).hardPDF);
   softPDF = (const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).softPDF);
   TruthMetaDataAuxContainer_v1 &modrhs = const_cast<TruthMetaDataAuxContainer_v1 &>( rhs );
   modrhs.weightNames.clear();
   modrhs.mcChannelNumber.clear();
   modrhs.lhefGenerator.clear();
   modrhs.generators.clear();
   modrhs.evgenProcess.clear();
   modrhs.evgenTune.clear();
   modrhs.hardPDF.clear();
   modrhs.softPDF.clear();
   return *this;
}
xAOD::TruthMetaDataAuxContainer_v1::TruthMetaDataAuxContainer_v1(const TruthMetaDataAuxContainer_v1 & rhs)
   : xAOD::AuxContainerBase(const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ))
   , weightNames(const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).weightNames)
   , mcChannelNumber(const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).mcChannelNumber)
   , lhefGenerator(const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).lhefGenerator)
   , generators(const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).generators)
   , evgenProcess(const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).evgenProcess)
   , evgenTune(const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).evgenTune)
   , hardPDF(const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).hardPDF)
   , softPDF(const_cast<TruthMetaDataAuxContainer_v1 &>( rhs ).softPDF)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   TruthMetaDataAuxContainer_v1 &modrhs = const_cast<TruthMetaDataAuxContainer_v1 &>( rhs );
   modrhs.weightNames.clear();
   modrhs.mcChannelNumber.clear();
   modrhs.lhefGenerator.clear();
   modrhs.generators.clear();
   modrhs.evgenProcess.clear();
   modrhs.evgenTune.clear();
   modrhs.hardPDF.clear();
   modrhs.softPDF.clear();
}
xAOD::TruthMetaDataAuxContainer_v1::~TruthMetaDataAuxContainer_v1() {
}
#endif // xAOD__TruthMetaDataAuxContainer_v1_cxx

#ifndef xAOD__AuxContainerBase_cxx
#define xAOD__AuxContainerBase_cxx
xAOD::AuxContainerBase::AuxContainerBase() {
}
xAOD::AuxContainerBase &xAOD::AuxContainerBase::operator=(const AuxContainerBase & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::IAuxStore::operator=(const_cast<AuxContainerBase &>( rhs ));
   SG::IAuxStoreIO::operator=(const_cast<AuxContainerBase &>( rhs ));
   SG::IAuxStoreHolder::operator=(const_cast<AuxContainerBase &>( rhs ));
   ILockable::operator=(const_cast<AuxContainerBase &>( rhs ));
   return *this;
}
xAOD::AuxContainerBase::AuxContainerBase(const AuxContainerBase & rhs)
   : SG::IAuxStore(const_cast<AuxContainerBase &>( rhs ))
   , SG::IAuxStoreIO(const_cast<AuxContainerBase &>( rhs ))
   , SG::IAuxStoreHolder(const_cast<AuxContainerBase &>( rhs ))
   , ILockable(const_cast<AuxContainerBase &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::AuxContainerBase::~AuxContainerBase() {
}
#endif // xAOD__AuxContainerBase_cxx

#ifndef xAOD__CutBookkeeperAuxContainer_v1_cxx
#define xAOD__CutBookkeeperAuxContainer_v1_cxx
xAOD::CutBookkeeperAuxContainer_v1::CutBookkeeperAuxContainer_v1() {
}
xAOD::CutBookkeeperAuxContainer_v1 &xAOD::CutBookkeeperAuxContainer_v1::operator=(const CutBookkeeperAuxContainer_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::AuxContainerBase::operator=(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ));
   nameIdentifier = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).nameIdentifier);
   uniqueIdentifier = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).uniqueIdentifier);
   name = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).name);
   description = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).description);
   cutLogic = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).cutLogic);
   isTopFilter = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).isTopFilter);
   cycle = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).cycle);
   inputStream = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).inputStream);
   outputStreams = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).outputStreams);
   parentLink = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).parentLink);
   childrenLinks = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).childrenLinks);
   othersLinks = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).othersLinks);
   siblingsLinks = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).siblingsLinks);
   nAcceptedEvents = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).nAcceptedEvents);
   sumOfEventWeights = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).sumOfEventWeights);
   sumOfEventWeightsSquared = (const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).sumOfEventWeightsSquared);
   CutBookkeeperAuxContainer_v1 &modrhs = const_cast<CutBookkeeperAuxContainer_v1 &>( rhs );
   modrhs.nameIdentifier.clear();
   modrhs.uniqueIdentifier.clear();
   modrhs.name.clear();
   modrhs.description.clear();
   modrhs.cutLogic.clear();
   modrhs.isTopFilter.clear();
   modrhs.cycle.clear();
   modrhs.inputStream.clear();
   modrhs.outputStreams.clear();
   modrhs.parentLink.clear();
   modrhs.childrenLinks.clear();
   modrhs.othersLinks.clear();
   modrhs.siblingsLinks.clear();
   modrhs.nAcceptedEvents.clear();
   modrhs.sumOfEventWeights.clear();
   modrhs.sumOfEventWeightsSquared.clear();
   return *this;
}
xAOD::CutBookkeeperAuxContainer_v1::CutBookkeeperAuxContainer_v1(const CutBookkeeperAuxContainer_v1 & rhs)
   : xAOD::AuxContainerBase(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ))
   , nameIdentifier(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).nameIdentifier)
   , uniqueIdentifier(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).uniqueIdentifier)
   , name(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).name)
   , description(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).description)
   , cutLogic(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).cutLogic)
   , isTopFilter(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).isTopFilter)
   , cycle(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).cycle)
   , inputStream(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).inputStream)
   , outputStreams(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).outputStreams)
   , parentLink(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).parentLink)
   , childrenLinks(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).childrenLinks)
   , othersLinks(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).othersLinks)
   , siblingsLinks(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).siblingsLinks)
   , nAcceptedEvents(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).nAcceptedEvents)
   , sumOfEventWeights(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).sumOfEventWeights)
   , sumOfEventWeightsSquared(const_cast<CutBookkeeperAuxContainer_v1 &>( rhs ).sumOfEventWeightsSquared)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   CutBookkeeperAuxContainer_v1 &modrhs = const_cast<CutBookkeeperAuxContainer_v1 &>( rhs );
   modrhs.nameIdentifier.clear();
   modrhs.uniqueIdentifier.clear();
   modrhs.name.clear();
   modrhs.description.clear();
   modrhs.cutLogic.clear();
   modrhs.isTopFilter.clear();
   modrhs.cycle.clear();
   modrhs.inputStream.clear();
   modrhs.outputStreams.clear();
   modrhs.parentLink.clear();
   modrhs.childrenLinks.clear();
   modrhs.othersLinks.clear();
   modrhs.siblingsLinks.clear();
   modrhs.nAcceptedEvents.clear();
   modrhs.sumOfEventWeights.clear();
   modrhs.sumOfEventWeightsSquared.clear();
}
xAOD::CutBookkeeperAuxContainer_v1::~CutBookkeeperAuxContainer_v1() {
}
#endif // xAOD__CutBookkeeperAuxContainer_v1_cxx

#ifndef ElementLink_xAOD__CutBookkeeperContainer_v1__cxx
#define ElementLink_xAOD__CutBookkeeperContainer_v1__cxx
ElementLink<xAOD::CutBookkeeperContainer_v1>::ElementLink() {
}
ElementLink<xAOD::CutBookkeeperContainer_v1> &ElementLink<xAOD::CutBookkeeperContainer_v1>::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<xAOD::CutBookkeeperContainer_v1>::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<xAOD::CutBookkeeperContainer_v1>::~ElementLink() {
}
#endif // ElementLink_xAOD__CutBookkeeperContainer_v1__cxx

#ifndef ElementLinkBase_cxx
#define ElementLinkBase_cxx
ElementLinkBase::ElementLinkBase() {
}
ElementLinkBase &ElementLinkBase::operator=(const ElementLinkBase & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_persKey = (const_cast<ElementLinkBase &>( rhs ).m_persKey);
   m_persIndex = (const_cast<ElementLinkBase &>( rhs ).m_persIndex);
   return *this;
}
ElementLinkBase::ElementLinkBase(const ElementLinkBase & rhs)
   : m_persKey(const_cast<ElementLinkBase &>( rhs ).m_persKey)
   , m_persIndex(const_cast<ElementLinkBase &>( rhs ).m_persIndex)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLinkBase::~ElementLinkBase() {
}
#endif // ElementLinkBase_cxx

#ifndef xAOD__TriggerMenuAuxContainer_v1_cxx
#define xAOD__TriggerMenuAuxContainer_v1_cxx
xAOD::TriggerMenuAuxContainer_v1::TriggerMenuAuxContainer_v1() {
}
xAOD::TriggerMenuAuxContainer_v1 &xAOD::TriggerMenuAuxContainer_v1::operator=(const TriggerMenuAuxContainer_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::AuxContainerBase::operator=(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ));
   smk = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).smk);
   l1psk = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).l1psk);
   hltpsk = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).hltpsk);
   itemCtpIds = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).itemCtpIds);
   itemNames = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).itemNames);
   itemPrescales = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).itemPrescales);
   chainIds = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainIds);
   chainNames = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainNames);
   chainParentNames = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainParentNames);
   chainPrescales = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainPrescales);
   chainRerunPrescales = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainRerunPrescales);
   chainPassthroughPrescales = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainPassthroughPrescales);
   chainSignatureCounters = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainSignatureCounters);
   chainSignatureLogics = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainSignatureLogics);
   chainSignatureOutputTEs = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainSignatureOutputTEs);
   chainSignatureLabels = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainSignatureLabels);
   sequenceInputTEs = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).sequenceInputTEs);
   sequenceOutputTEs = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).sequenceOutputTEs);
   sequenceAlgorithms = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).sequenceAlgorithms);
   bunchGroupBunches = (const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).bunchGroupBunches);
   TriggerMenuAuxContainer_v1 &modrhs = const_cast<TriggerMenuAuxContainer_v1 &>( rhs );
   modrhs.smk.clear();
   modrhs.l1psk.clear();
   modrhs.hltpsk.clear();
   modrhs.itemCtpIds.clear();
   modrhs.itemNames.clear();
   modrhs.itemPrescales.clear();
   modrhs.chainIds.clear();
   modrhs.chainNames.clear();
   modrhs.chainParentNames.clear();
   modrhs.chainPrescales.clear();
   modrhs.chainRerunPrescales.clear();
   modrhs.chainPassthroughPrescales.clear();
   modrhs.chainSignatureCounters.clear();
   modrhs.chainSignatureLogics.clear();
   modrhs.chainSignatureOutputTEs.clear();
   modrhs.chainSignatureLabels.clear();
   modrhs.sequenceInputTEs.clear();
   modrhs.sequenceOutputTEs.clear();
   modrhs.sequenceAlgorithms.clear();
   modrhs.bunchGroupBunches.clear();
   return *this;
}
xAOD::TriggerMenuAuxContainer_v1::TriggerMenuAuxContainer_v1(const TriggerMenuAuxContainer_v1 & rhs)
   : xAOD::AuxContainerBase(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ))
   , smk(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).smk)
   , l1psk(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).l1psk)
   , hltpsk(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).hltpsk)
   , itemCtpIds(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).itemCtpIds)
   , itemNames(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).itemNames)
   , itemPrescales(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).itemPrescales)
   , chainIds(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainIds)
   , chainNames(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainNames)
   , chainParentNames(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainParentNames)
   , chainPrescales(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainPrescales)
   , chainRerunPrescales(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainRerunPrescales)
   , chainPassthroughPrescales(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainPassthroughPrescales)
   , chainSignatureCounters(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainSignatureCounters)
   , chainSignatureLogics(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainSignatureLogics)
   , chainSignatureOutputTEs(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainSignatureOutputTEs)
   , chainSignatureLabels(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).chainSignatureLabels)
   , sequenceInputTEs(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).sequenceInputTEs)
   , sequenceOutputTEs(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).sequenceOutputTEs)
   , sequenceAlgorithms(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).sequenceAlgorithms)
   , bunchGroupBunches(const_cast<TriggerMenuAuxContainer_v1 &>( rhs ).bunchGroupBunches)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   TriggerMenuAuxContainer_v1 &modrhs = const_cast<TriggerMenuAuxContainer_v1 &>( rhs );
   modrhs.smk.clear();
   modrhs.l1psk.clear();
   modrhs.hltpsk.clear();
   modrhs.itemCtpIds.clear();
   modrhs.itemNames.clear();
   modrhs.itemPrescales.clear();
   modrhs.chainIds.clear();
   modrhs.chainNames.clear();
   modrhs.chainParentNames.clear();
   modrhs.chainPrescales.clear();
   modrhs.chainRerunPrescales.clear();
   modrhs.chainPassthroughPrescales.clear();
   modrhs.chainSignatureCounters.clear();
   modrhs.chainSignatureLogics.clear();
   modrhs.chainSignatureOutputTEs.clear();
   modrhs.chainSignatureLabels.clear();
   modrhs.sequenceInputTEs.clear();
   modrhs.sequenceOutputTEs.clear();
   modrhs.sequenceAlgorithms.clear();
   modrhs.bunchGroupBunches.clear();
}
xAOD::TriggerMenuAuxContainer_v1::~TriggerMenuAuxContainer_v1() {
}
#endif // xAOD__TriggerMenuAuxContainer_v1_cxx

#ifndef xAOD__LumiBlockRangeAuxContainer_v1_cxx
#define xAOD__LumiBlockRangeAuxContainer_v1_cxx
xAOD::LumiBlockRangeAuxContainer_v1::LumiBlockRangeAuxContainer_v1() {
}
xAOD::LumiBlockRangeAuxContainer_v1 &xAOD::LumiBlockRangeAuxContainer_v1::operator=(const LumiBlockRangeAuxContainer_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::AuxContainerBase::operator=(const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ));
   startRunNumber = (const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ).startRunNumber);
   startLumiBlockNumber = (const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ).startLumiBlockNumber);
   stopRunNumber = (const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ).stopRunNumber);
   stopLumiBlockNumber = (const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ).stopLumiBlockNumber);
   eventsExpected = (const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ).eventsExpected);
   eventsSeen = (const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ).eventsSeen);
   LumiBlockRangeAuxContainer_v1 &modrhs = const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs );
   modrhs.startRunNumber.clear();
   modrhs.startLumiBlockNumber.clear();
   modrhs.stopRunNumber.clear();
   modrhs.stopLumiBlockNumber.clear();
   modrhs.eventsExpected.clear();
   modrhs.eventsSeen.clear();
   return *this;
}
xAOD::LumiBlockRangeAuxContainer_v1::LumiBlockRangeAuxContainer_v1(const LumiBlockRangeAuxContainer_v1 & rhs)
   : xAOD::AuxContainerBase(const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ))
   , startRunNumber(const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ).startRunNumber)
   , startLumiBlockNumber(const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ).startLumiBlockNumber)
   , stopRunNumber(const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ).stopRunNumber)
   , stopLumiBlockNumber(const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ).stopLumiBlockNumber)
   , eventsExpected(const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ).eventsExpected)
   , eventsSeen(const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs ).eventsSeen)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   LumiBlockRangeAuxContainer_v1 &modrhs = const_cast<LumiBlockRangeAuxContainer_v1 &>( rhs );
   modrhs.startRunNumber.clear();
   modrhs.startLumiBlockNumber.clear();
   modrhs.stopRunNumber.clear();
   modrhs.stopLumiBlockNumber.clear();
   modrhs.eventsExpected.clear();
   modrhs.eventsSeen.clear();
}
xAOD::LumiBlockRangeAuxContainer_v1::~LumiBlockRangeAuxContainer_v1() {
}
#endif // xAOD__LumiBlockRangeAuxContainer_v1_cxx

#ifndef EventType_p3_cxx
#define EventType_p3_cxx
EventType_p3::EventType_p3() {
}
EventType_p3 &EventType_p3::operator=(const EventType_p3 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_bit_mask = (const_cast<EventType_p3 &>( rhs ).m_bit_mask);
   m_user_type = (const_cast<EventType_p3 &>( rhs ).m_user_type);
   m_mc_event_weights = (const_cast<EventType_p3 &>( rhs ).m_mc_event_weights);
   m_mc_channel_number = (const_cast<EventType_p3 &>( rhs ).m_mc_channel_number);
   m_mc_event_number = (const_cast<EventType_p3 &>( rhs ).m_mc_event_number);
   EventType_p3 &modrhs = const_cast<EventType_p3 &>( rhs );
   modrhs.m_bit_mask.clear();
   modrhs.m_user_type.clear();
   modrhs.m_mc_event_weights.clear();
   return *this;
}
EventType_p3::EventType_p3(const EventType_p3 & rhs)
   : m_bit_mask(const_cast<EventType_p3 &>( rhs ).m_bit_mask)
   , m_user_type(const_cast<EventType_p3 &>( rhs ).m_user_type)
   , m_mc_event_weights(const_cast<EventType_p3 &>( rhs ).m_mc_event_weights)
   , m_mc_channel_number(const_cast<EventType_p3 &>( rhs ).m_mc_channel_number)
   , m_mc_event_number(const_cast<EventType_p3 &>( rhs ).m_mc_event_number)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   EventType_p3 &modrhs = const_cast<EventType_p3 &>( rhs );
   modrhs.m_bit_mask.clear();
   modrhs.m_user_type.clear();
   modrhs.m_mc_event_weights.clear();
}
EventType_p3::~EventType_p3() {
}
#endif // EventType_p3_cxx

#ifndef xAOD__FileMetaData_v1_cxx
#define xAOD__FileMetaData_v1_cxx
xAOD::FileMetaData_v1::FileMetaData_v1() {
}
xAOD::FileMetaData_v1 &xAOD::FileMetaData_v1::operator=(const FileMetaData_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<FileMetaData_v1 &>( rhs ));
   return *this;
}
xAOD::FileMetaData_v1::FileMetaData_v1(const FileMetaData_v1 & rhs)
   : SG::AuxElement(const_cast<FileMetaData_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::FileMetaData_v1::~FileMetaData_v1() {
}
#endif // xAOD__FileMetaData_v1_cxx

#ifndef SG__AuxElement_cxx
#define SG__AuxElement_cxx
SG::AuxElement::AuxElement() {
}
SG::AuxElement &SG::AuxElement::operator=(const AuxElement & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::IAuxElement::operator=(const_cast<AuxElement &>( rhs ));
   return *this;
}
SG::AuxElement::AuxElement(const AuxElement & rhs)
   : SG::IAuxElement(const_cast<AuxElement &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
SG::AuxElement::~AuxElement() {
}
#endif // SG__AuxElement_cxx

#ifndef SG__IAuxElement_cxx
#define SG__IAuxElement_cxx
SG::IAuxElement::IAuxElement() {
}
SG::IAuxElement &SG::IAuxElement::operator=(const IAuxElement & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   return *this;
}
SG::IAuxElement::IAuxElement(const IAuxElement & rhs)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
SG::IAuxElement::~IAuxElement() {
}
#endif // SG__IAuxElement_cxx

#ifndef xAOD__EventFormat_v1_cxx
#define xAOD__EventFormat_v1_cxx
xAOD::EventFormat_v1::EventFormat_v1() {
}
xAOD::EventFormat_v1 &xAOD::EventFormat_v1::operator=(const EventFormat_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_branchNames = (const_cast<EventFormat_v1 &>( rhs ).m_branchNames);
   m_classNames = (const_cast<EventFormat_v1 &>( rhs ).m_classNames);
   m_parentNames = (const_cast<EventFormat_v1 &>( rhs ).m_parentNames);
   m_branchHashes = (const_cast<EventFormat_v1 &>( rhs ).m_branchHashes);
   EventFormat_v1 &modrhs = const_cast<EventFormat_v1 &>( rhs );
   modrhs.m_branchNames.clear();
   modrhs.m_classNames.clear();
   modrhs.m_parentNames.clear();
   modrhs.m_branchHashes.clear();
   return *this;
}
xAOD::EventFormat_v1::EventFormat_v1(const EventFormat_v1 & rhs)
   : m_branchNames(const_cast<EventFormat_v1 &>( rhs ).m_branchNames)
   , m_classNames(const_cast<EventFormat_v1 &>( rhs ).m_classNames)
   , m_parentNames(const_cast<EventFormat_v1 &>( rhs ).m_parentNames)
   , m_branchHashes(const_cast<EventFormat_v1 &>( rhs ).m_branchHashes)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   EventFormat_v1 &modrhs = const_cast<EventFormat_v1 &>( rhs );
   modrhs.m_branchNames.clear();
   modrhs.m_classNames.clear();
   modrhs.m_parentNames.clear();
   modrhs.m_branchHashes.clear();
}
xAOD::EventFormat_v1::~EventFormat_v1() {
}
#endif // xAOD__EventFormat_v1_cxx

#ifndef xAOD__TriggerMenu_v1_cxx
#define xAOD__TriggerMenu_v1_cxx
xAOD::TriggerMenu_v1::TriggerMenu_v1() {
}
xAOD::TriggerMenu_v1 &xAOD::TriggerMenu_v1::operator=(const TriggerMenu_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<TriggerMenu_v1 &>( rhs ));
   return *this;
}
xAOD::TriggerMenu_v1::TriggerMenu_v1(const TriggerMenu_v1 & rhs)
   : SG::AuxElement(const_cast<TriggerMenu_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::TriggerMenu_v1::~TriggerMenu_v1() {
}
#endif // xAOD__TriggerMenu_v1_cxx

#ifndef xAOD__LumiBlockRange_v1_cxx
#define xAOD__LumiBlockRange_v1_cxx
xAOD::LumiBlockRange_v1::LumiBlockRange_v1() {
}
xAOD::LumiBlockRange_v1 &xAOD::LumiBlockRange_v1::operator=(const LumiBlockRange_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<LumiBlockRange_v1 &>( rhs ));
   return *this;
}
xAOD::LumiBlockRange_v1::LumiBlockRange_v1(const LumiBlockRange_v1 & rhs)
   : SG::AuxElement(const_cast<LumiBlockRange_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::LumiBlockRange_v1::~LumiBlockRange_v1() {
}
#endif // xAOD__LumiBlockRange_v1_cxx

#ifndef xAOD__TruthMetaData_v1_cxx
#define xAOD__TruthMetaData_v1_cxx
xAOD::TruthMetaData_v1::TruthMetaData_v1() {
}
xAOD::TruthMetaData_v1 &xAOD::TruthMetaData_v1::operator=(const TruthMetaData_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<TruthMetaData_v1 &>( rhs ));
   return *this;
}
xAOD::TruthMetaData_v1::TruthMetaData_v1(const TruthMetaData_v1 & rhs)
   : SG::AuxElement(const_cast<TruthMetaData_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::TruthMetaData_v1::~TruthMetaData_v1() {
}
#endif // xAOD__TruthMetaData_v1_cxx

#ifndef xAOD__CutBookkeeper_v1_cxx
#define xAOD__CutBookkeeper_v1_cxx
xAOD::CutBookkeeper_v1::CutBookkeeper_v1() {
}
xAOD::CutBookkeeper_v1 &xAOD::CutBookkeeper_v1::operator=(const CutBookkeeper_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<CutBookkeeper_v1 &>( rhs ));
   return *this;
}
xAOD::CutBookkeeper_v1::CutBookkeeper_v1(const CutBookkeeper_v1 & rhs)
   : SG::AuxElement(const_cast<CutBookkeeper_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::CutBookkeeper_v1::~CutBookkeeper_v1() {
}
#endif // xAOD__CutBookkeeper_v1_cxx

#ifndef IOVPayloadContainer_p1__IOVRange_p1_cxx
#define IOVPayloadContainer_p1__IOVRange_p1_cxx
IOVPayloadContainer_p1::IOVRange_p1::IOVRange_p1() {
}
IOVPayloadContainer_p1::IOVRange_p1 &IOVPayloadContainer_p1::IOVRange_p1::operator=(const IOVRange_p1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_start = (const_cast<IOVRange_p1 &>( rhs ).m_start);
   m_stop = (const_cast<IOVRange_p1 &>( rhs ).m_stop);
   return *this;
}
IOVPayloadContainer_p1::IOVRange_p1::IOVRange_p1(const IOVRange_p1 & rhs)
   : m_start(const_cast<IOVRange_p1 &>( rhs ).m_start)
   , m_stop(const_cast<IOVRange_p1 &>( rhs ).m_stop)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
IOVPayloadContainer_p1::IOVRange_p1::~IOVRange_p1() {
}
#endif // IOVPayloadContainer_p1__IOVRange_p1_cxx

#ifndef IOVPayloadContainer_p1__CondAttrListEntry_p1_cxx
#define IOVPayloadContainer_p1__CondAttrListEntry_p1_cxx
IOVPayloadContainer_p1::CondAttrListEntry_p1::CondAttrListEntry_p1() {
}
IOVPayloadContainer_p1::CondAttrListEntry_p1 &IOVPayloadContainer_p1::CondAttrListEntry_p1::operator=(const CondAttrListEntry_p1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_channelNumber = (const_cast<CondAttrListEntry_p1 &>( rhs ).m_channelNumber);
   m_firstIndex = (const_cast<CondAttrListEntry_p1 &>( rhs ).m_firstIndex);
   m_lastIndex = (const_cast<CondAttrListEntry_p1 &>( rhs ).m_lastIndex);
   m_range = (const_cast<CondAttrListEntry_p1 &>( rhs ).m_range);
   m_name = (const_cast<CondAttrListEntry_p1 &>( rhs ).m_name);
   CondAttrListEntry_p1 &modrhs = const_cast<CondAttrListEntry_p1 &>( rhs );
   modrhs.m_name.clear();
   return *this;
}
IOVPayloadContainer_p1::CondAttrListEntry_p1::CondAttrListEntry_p1(const CondAttrListEntry_p1 & rhs)
   : m_channelNumber(const_cast<CondAttrListEntry_p1 &>( rhs ).m_channelNumber)
   , m_firstIndex(const_cast<CondAttrListEntry_p1 &>( rhs ).m_firstIndex)
   , m_lastIndex(const_cast<CondAttrListEntry_p1 &>( rhs ).m_lastIndex)
   , m_range(const_cast<CondAttrListEntry_p1 &>( rhs ).m_range)
   , m_name(const_cast<CondAttrListEntry_p1 &>( rhs ).m_name)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   CondAttrListEntry_p1 &modrhs = const_cast<CondAttrListEntry_p1 &>( rhs );
   modrhs.m_name.clear();
}
IOVPayloadContainer_p1::CondAttrListEntry_p1::~CondAttrListEntry_p1() {
}
#endif // IOVPayloadContainer_p1__CondAttrListEntry_p1_cxx

#ifndef IOVPayloadContainer_p1__CondAttrListCollection_p1_cxx
#define IOVPayloadContainer_p1__CondAttrListCollection_p1_cxx
IOVPayloadContainer_p1::CondAttrListCollection_p1::CondAttrListCollection_p1() {
}
IOVPayloadContainer_p1::CondAttrListCollection_p1 &IOVPayloadContainer_p1::CondAttrListCollection_p1::operator=(const CondAttrListCollection_p1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_attrLists = (const_cast<CondAttrListCollection_p1 &>( rhs ).m_attrLists);
   m_start = (const_cast<CondAttrListCollection_p1 &>( rhs ).m_start);
   m_stop = (const_cast<CondAttrListCollection_p1 &>( rhs ).m_stop);
   m_hasRunLumiBlockTime = (const_cast<CondAttrListCollection_p1 &>( rhs ).m_hasRunLumiBlockTime);
   CondAttrListCollection_p1 &modrhs = const_cast<CondAttrListCollection_p1 &>( rhs );
   modrhs.m_attrLists.clear();
   return *this;
}
IOVPayloadContainer_p1::CondAttrListCollection_p1::CondAttrListCollection_p1(const CondAttrListCollection_p1 & rhs)
   : m_attrLists(const_cast<CondAttrListCollection_p1 &>( rhs ).m_attrLists)
   , m_start(const_cast<CondAttrListCollection_p1 &>( rhs ).m_start)
   , m_stop(const_cast<CondAttrListCollection_p1 &>( rhs ).m_stop)
   , m_hasRunLumiBlockTime(const_cast<CondAttrListCollection_p1 &>( rhs ).m_hasRunLumiBlockTime)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   CondAttrListCollection_p1 &modrhs = const_cast<CondAttrListCollection_p1 &>( rhs );
   modrhs.m_attrLists.clear();
}
IOVPayloadContainer_p1::CondAttrListCollection_p1::~CondAttrListCollection_p1() {
}
#endif // IOVPayloadContainer_p1__CondAttrListCollection_p1_cxx

#ifndef IOVPayloadContainer_p1_cxx
#define IOVPayloadContainer_p1_cxx
IOVPayloadContainer_p1::IOVPayloadContainer_p1() {
}
IOVPayloadContainer_p1 &IOVPayloadContainer_p1::operator=(const IOVPayloadContainer_p1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_payloadVec = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_payloadVec);
   m_attrIndexes = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_attrIndexes);
   m_bool = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_bool);
   m_char = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_char);
   m_unsignedChar = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_unsignedChar);
   m_short = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_short);
   m_unsignedShort = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_unsignedShort);
   m_int = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_int);
   m_unsignedInt = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_unsignedInt);
   m_long = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_long);
   m_unsignedLong = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_unsignedLong);
   m_longLong = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_longLong);
   m_unsignedLongLong = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_unsignedLongLong);
   m_float = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_float);
   m_double = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_double);
   m_string = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_string);
   m_date = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_date);
   m_timeStamp = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_timeStamp);
   m_attrName = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_attrName);
   m_attrType = (const_cast<IOVPayloadContainer_p1 &>( rhs ).m_attrType);
   IOVPayloadContainer_p1 &modrhs = const_cast<IOVPayloadContainer_p1 &>( rhs );
   modrhs.m_payloadVec.clear();
   modrhs.m_attrIndexes.clear();
   modrhs.m_bool.clear();
   modrhs.m_char.clear();
   modrhs.m_unsignedChar.clear();
   modrhs.m_short.clear();
   modrhs.m_unsignedShort.clear();
   modrhs.m_int.clear();
   modrhs.m_unsignedInt.clear();
   modrhs.m_long.clear();
   modrhs.m_unsignedLong.clear();
   modrhs.m_longLong.clear();
   modrhs.m_unsignedLongLong.clear();
   modrhs.m_float.clear();
   modrhs.m_double.clear();
   modrhs.m_string.clear();
   modrhs.m_date.clear();
   modrhs.m_timeStamp.clear();
   modrhs.m_attrName.clear();
   modrhs.m_attrType.clear();
   return *this;
}
IOVPayloadContainer_p1::IOVPayloadContainer_p1(const IOVPayloadContainer_p1 & rhs)
   : m_payloadVec(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_payloadVec)
   , m_attrIndexes(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_attrIndexes)
   , m_bool(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_bool)
   , m_char(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_char)
   , m_unsignedChar(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_unsignedChar)
   , m_short(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_short)
   , m_unsignedShort(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_unsignedShort)
   , m_int(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_int)
   , m_unsignedInt(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_unsignedInt)
   , m_long(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_long)
   , m_unsignedLong(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_unsignedLong)
   , m_longLong(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_longLong)
   , m_unsignedLongLong(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_unsignedLongLong)
   , m_float(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_float)
   , m_double(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_double)
   , m_string(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_string)
   , m_date(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_date)
   , m_timeStamp(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_timeStamp)
   , m_attrName(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_attrName)
   , m_attrType(const_cast<IOVPayloadContainer_p1 &>( rhs ).m_attrType)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   IOVPayloadContainer_p1 &modrhs = const_cast<IOVPayloadContainer_p1 &>( rhs );
   modrhs.m_payloadVec.clear();
   modrhs.m_attrIndexes.clear();
   modrhs.m_bool.clear();
   modrhs.m_char.clear();
   modrhs.m_unsignedChar.clear();
   modrhs.m_short.clear();
   modrhs.m_unsignedShort.clear();
   modrhs.m_int.clear();
   modrhs.m_unsignedInt.clear();
   modrhs.m_long.clear();
   modrhs.m_unsignedLong.clear();
   modrhs.m_longLong.clear();
   modrhs.m_unsignedLongLong.clear();
   modrhs.m_float.clear();
   modrhs.m_double.clear();
   modrhs.m_string.clear();
   modrhs.m_date.clear();
   modrhs.m_timeStamp.clear();
   modrhs.m_attrName.clear();
   modrhs.m_attrType.clear();
}
IOVPayloadContainer_p1::~IOVPayloadContainer_p1() {
}
#endif // IOVPayloadContainer_p1_cxx

#ifndef AttrListIndexes_cxx
#define AttrListIndexes_cxx
AttrListIndexes::AttrListIndexes() {
}
AttrListIndexes &AttrListIndexes::operator=(const AttrListIndexes & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_typeIndex = (const_cast<AttrListIndexes &>( rhs ).m_typeIndex);
   m_objIndex = (const_cast<AttrListIndexes &>( rhs ).m_objIndex);
   return *this;
}
AttrListIndexes::AttrListIndexes(const AttrListIndexes & rhs)
   : m_typeIndex(const_cast<AttrListIndexes &>( rhs ).m_typeIndex)
   , m_objIndex(const_cast<AttrListIndexes &>( rhs ).m_objIndex)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
AttrListIndexes::~AttrListIndexes() {
}
#endif // AttrListIndexes_cxx

#ifndef DataHeader_p6__FullElement_cxx
#define DataHeader_p6__FullElement_cxx
DataHeader_p6::FullElement::FullElement() {
}
DataHeader_p6::FullElement &DataHeader_p6::FullElement::operator=(const FullElement & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   oid2 = (const_cast<FullElement &>( rhs ).oid2);
   dbIdx = (const_cast<FullElement &>( rhs ).dbIdx);
   objIdx = (const_cast<FullElement &>( rhs ).objIdx);
   return *this;
}
DataHeader_p6::FullElement::FullElement(const FullElement & rhs)
   : oid2(const_cast<FullElement &>( rhs ).oid2)
   , dbIdx(const_cast<FullElement &>( rhs ).dbIdx)
   , objIdx(const_cast<FullElement &>( rhs ).objIdx)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
DataHeader_p6::FullElement::~FullElement() {
}
#endif // DataHeader_p6__FullElement_cxx

#ifndef DataHeader_p6_cxx
#define DataHeader_p6_cxx
DataHeader_p6::DataHeader_p6() {
}
DataHeader_p6 &DataHeader_p6::operator=(const DataHeader_p6 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_commonDbIndex = (const_cast<DataHeader_p6 &>( rhs ).m_commonDbIndex);
   m_commonOID2 = (const_cast<DataHeader_p6 &>( rhs ).m_commonOID2);
   m_shortElements = (const_cast<DataHeader_p6 &>( rhs ).m_shortElements);
   m_fullElements = (const_cast<DataHeader_p6 &>( rhs ).m_fullElements);
   m_provenanceSize = (const_cast<DataHeader_p6 &>( rhs ).m_provenanceSize);
   m_dhFormToken = (const_cast<DataHeader_p6 &>( rhs ).m_dhFormToken);
   DataHeader_p6 &modrhs = const_cast<DataHeader_p6 &>( rhs );
   modrhs.m_shortElements.clear();
   modrhs.m_fullElements.clear();
   modrhs.m_dhFormToken.clear();
   return *this;
}
DataHeader_p6::DataHeader_p6(const DataHeader_p6 & rhs)
   : m_commonDbIndex(const_cast<DataHeader_p6 &>( rhs ).m_commonDbIndex)
   , m_commonOID2(const_cast<DataHeader_p6 &>( rhs ).m_commonOID2)
   , m_shortElements(const_cast<DataHeader_p6 &>( rhs ).m_shortElements)
   , m_fullElements(const_cast<DataHeader_p6 &>( rhs ).m_fullElements)
   , m_provenanceSize(const_cast<DataHeader_p6 &>( rhs ).m_provenanceSize)
   , m_dhFormToken(const_cast<DataHeader_p6 &>( rhs ).m_dhFormToken)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   DataHeader_p6 &modrhs = const_cast<DataHeader_p6 &>( rhs );
   modrhs.m_shortElements.clear();
   modrhs.m_fullElements.clear();
   modrhs.m_dhFormToken.clear();
}
DataHeader_p6::~DataHeader_p6() {
}
#endif // DataHeader_p6_cxx

#ifndef DataHeaderForm_p6__ObjRecord_cxx
#define DataHeaderForm_p6__ObjRecord_cxx
DataHeaderForm_p6::ObjRecord::ObjRecord() {
}
DataHeaderForm_p6::ObjRecord &DataHeaderForm_p6::ObjRecord::operator=(const ObjRecord & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   guid = (const_cast<ObjRecord &>( rhs ).guid);
   cont = (const_cast<ObjRecord &>( rhs ).cont);
   key = (const_cast<ObjRecord &>( rhs ).key);
   clid = (const_cast<ObjRecord &>( rhs ).clid);
   oid1 = (const_cast<ObjRecord &>( rhs ).oid1);
   ObjRecord &modrhs = const_cast<ObjRecord &>( rhs );
   modrhs.cont.clear();
   modrhs.key.clear();
   return *this;
}
DataHeaderForm_p6::ObjRecord::ObjRecord(const ObjRecord & rhs)
   : guid(const_cast<ObjRecord &>( rhs ).guid)
   , cont(const_cast<ObjRecord &>( rhs ).cont)
   , key(const_cast<ObjRecord &>( rhs ).key)
   , clid(const_cast<ObjRecord &>( rhs ).clid)
   , oid1(const_cast<ObjRecord &>( rhs ).oid1)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ObjRecord &modrhs = const_cast<ObjRecord &>( rhs );
   modrhs.cont.clear();
   modrhs.key.clear();
}
DataHeaderForm_p6::ObjRecord::~ObjRecord() {
}
#endif // DataHeaderForm_p6__ObjRecord_cxx

#ifndef DataHeaderForm_p6__DbRecord_cxx
#define DataHeaderForm_p6__DbRecord_cxx
DataHeaderForm_p6::DbRecord::DbRecord() {
}
DataHeaderForm_p6::DbRecord &DataHeaderForm_p6::DbRecord::operator=(const DbRecord & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   fid = (const_cast<DbRecord &>( rhs ).fid);
   tech = (const_cast<DbRecord &>( rhs ).tech);
   return *this;
}
DataHeaderForm_p6::DbRecord::DbRecord(const DbRecord & rhs)
   : fid(const_cast<DbRecord &>( rhs ).fid)
   , tech(const_cast<DbRecord &>( rhs ).tech)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
DataHeaderForm_p6::DbRecord::~DbRecord() {
}
#endif // DataHeaderForm_p6__DbRecord_cxx

#ifndef DataHeaderForm_p6_cxx
#define DataHeaderForm_p6_cxx
DataHeaderForm_p6::DataHeaderForm_p6() {
}
DataHeaderForm_p6 &DataHeaderForm_p6::operator=(const DataHeaderForm_p6 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_dbRecords = (const_cast<DataHeaderForm_p6 &>( rhs ).m_dbRecords);
   m_objRecords = (const_cast<DataHeaderForm_p6 &>( rhs ).m_objRecords);
   m_objAlias = (const_cast<DataHeaderForm_p6 &>( rhs ).m_objAlias);
   m_objSymLinks = (const_cast<DataHeaderForm_p6 &>( rhs ).m_objSymLinks);
   m_objHashes = (const_cast<DataHeaderForm_p6 &>( rhs ).m_objHashes);
   m_version = (const_cast<DataHeaderForm_p6 &>( rhs ).m_version);
   m_processTag = (const_cast<DataHeaderForm_p6 &>( rhs ).m_processTag);
   DataHeaderForm_p6 &modrhs = const_cast<DataHeaderForm_p6 &>( rhs );
   modrhs.m_dbRecords.clear();
   modrhs.m_objRecords.clear();
   modrhs.m_objAlias.clear();
   modrhs.m_objSymLinks.clear();
   modrhs.m_objHashes.clear();
   modrhs.m_processTag.clear();
   return *this;
}
DataHeaderForm_p6::DataHeaderForm_p6(const DataHeaderForm_p6 & rhs)
   : m_dbRecords(const_cast<DataHeaderForm_p6 &>( rhs ).m_dbRecords)
   , m_objRecords(const_cast<DataHeaderForm_p6 &>( rhs ).m_objRecords)
   , m_objAlias(const_cast<DataHeaderForm_p6 &>( rhs ).m_objAlias)
   , m_objSymLinks(const_cast<DataHeaderForm_p6 &>( rhs ).m_objSymLinks)
   , m_objHashes(const_cast<DataHeaderForm_p6 &>( rhs ).m_objHashes)
   , m_version(const_cast<DataHeaderForm_p6 &>( rhs ).m_version)
   , m_processTag(const_cast<DataHeaderForm_p6 &>( rhs ).m_processTag)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   DataHeaderForm_p6 &modrhs = const_cast<DataHeaderForm_p6 &>( rhs );
   modrhs.m_dbRecords.clear();
   modrhs.m_objRecords.clear();
   modrhs.m_objAlias.clear();
   modrhs.m_objSymLinks.clear();
   modrhs.m_objHashes.clear();
   modrhs.m_processTag.clear();
}
DataHeaderForm_p6::~DataHeaderForm_p6() {
}
#endif // DataHeaderForm_p6_cxx

#ifndef Guid_cxx
#define Guid_cxx
Guid::Guid() {
}
Guid &Guid::operator=(const Guid & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_data1 = (const_cast<Guid &>( rhs ).m_data1);
   m_data2 = (const_cast<Guid &>( rhs ).m_data2);
   m_data3 = (const_cast<Guid &>( rhs ).m_data3);
   for (Int_t i=0;i<8;i++) m_data4[i] = rhs.m_data4[i];
   return *this;
}
Guid::Guid(const Guid & rhs)
   : m_data1(const_cast<Guid &>( rhs ).m_data1)
   , m_data2(const_cast<Guid &>( rhs ).m_data2)
   , m_data3(const_cast<Guid &>( rhs ).m_data3)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   for (Int_t i=0;i<8;i++) m_data4[i] = rhs.m_data4[i];
}
Guid::~Guid() {
}
#endif // Guid_cxx

#ifndef xAOD__EventInfo_v1_cxx
#define xAOD__EventInfo_v1_cxx
xAOD::EventInfo_v1::EventInfo_v1() {
}
xAOD::EventInfo_v1 &xAOD::EventInfo_v1::operator=(const EventInfo_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<EventInfo_v1 &>( rhs ));
   return *this;
}
xAOD::EventInfo_v1::EventInfo_v1(const EventInfo_v1 & rhs)
   : SG::AuxElement(const_cast<EventInfo_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::EventInfo_v1::~EventInfo_v1() {
}
#endif // xAOD__EventInfo_v1_cxx

#ifndef xAOD__TrigConfKeys_v1_cxx
#define xAOD__TrigConfKeys_v1_cxx
xAOD::TrigConfKeys_v1::TrigConfKeys_v1() {
}
xAOD::TrigConfKeys_v1 &xAOD::TrigConfKeys_v1::operator=(const TrigConfKeys_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   m_smk = (const_cast<TrigConfKeys_v1 &>( rhs ).m_smk);
   m_l1psk = (const_cast<TrigConfKeys_v1 &>( rhs ).m_l1psk);
   m_hltpsk = (const_cast<TrigConfKeys_v1 &>( rhs ).m_hltpsk);
   return *this;
}
xAOD::TrigConfKeys_v1::TrigConfKeys_v1(const TrigConfKeys_v1 & rhs)
   : m_smk(const_cast<TrigConfKeys_v1 &>( rhs ).m_smk)
   , m_l1psk(const_cast<TrigConfKeys_v1 &>( rhs ).m_l1psk)
   , m_hltpsk(const_cast<TrigConfKeys_v1 &>( rhs ).m_hltpsk)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::TrigConfKeys_v1::~TrigConfKeys_v1() {
}
#endif // xAOD__TrigConfKeys_v1_cxx

#ifndef xAOD__TrigDecisionAuxInfo_v1_cxx
#define xAOD__TrigDecisionAuxInfo_v1_cxx
xAOD::TrigDecisionAuxInfo_v1::TrigDecisionAuxInfo_v1() {
}
xAOD::TrigDecisionAuxInfo_v1 &xAOD::TrigDecisionAuxInfo_v1::operator=(const TrigDecisionAuxInfo_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::AuxInfoBase::operator=(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ));
   smk = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).smk);
   bgCode = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).bgCode);
   tav = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).tav);
   tap = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).tap);
   tbp = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).tbp);
   lvl2ErrorBits = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2ErrorBits);
   efErrorBits = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efErrorBits);
   lvl2Truncated = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2Truncated);
   efTruncated = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efTruncated);
   lvl2PassedPhysics = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2PassedPhysics);
   efPassedPhysics = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efPassedPhysics);
   lvl2PassedRaw = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2PassedRaw);
   efPassedRaw = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efPassedRaw);
   lvl2PassedThrough = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2PassedThrough);
   efPassedThrough = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efPassedThrough);
   lvl2Prescaled = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2Prescaled);
   efPrescaled = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efPrescaled);
   lvl2Resurrected = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2Resurrected);
   efResurrected = (const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efResurrected);
   TrigDecisionAuxInfo_v1 &modrhs = const_cast<TrigDecisionAuxInfo_v1 &>( rhs );
   modrhs.tav.clear();
   modrhs.tap.clear();
   modrhs.tbp.clear();
   modrhs.lvl2PassedPhysics.clear();
   modrhs.efPassedPhysics.clear();
   modrhs.lvl2PassedRaw.clear();
   modrhs.efPassedRaw.clear();
   modrhs.lvl2PassedThrough.clear();
   modrhs.efPassedThrough.clear();
   modrhs.lvl2Prescaled.clear();
   modrhs.efPrescaled.clear();
   modrhs.lvl2Resurrected.clear();
   modrhs.efResurrected.clear();
   return *this;
}
xAOD::TrigDecisionAuxInfo_v1::TrigDecisionAuxInfo_v1(const TrigDecisionAuxInfo_v1 & rhs)
   : xAOD::AuxInfoBase(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ))
   , smk(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).smk)
   , bgCode(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).bgCode)
   , tav(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).tav)
   , tap(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).tap)
   , tbp(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).tbp)
   , lvl2ErrorBits(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2ErrorBits)
   , efErrorBits(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efErrorBits)
   , lvl2Truncated(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2Truncated)
   , efTruncated(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efTruncated)
   , lvl2PassedPhysics(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2PassedPhysics)
   , efPassedPhysics(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efPassedPhysics)
   , lvl2PassedRaw(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2PassedRaw)
   , efPassedRaw(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efPassedRaw)
   , lvl2PassedThrough(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2PassedThrough)
   , efPassedThrough(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efPassedThrough)
   , lvl2Prescaled(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2Prescaled)
   , efPrescaled(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efPrescaled)
   , lvl2Resurrected(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).lvl2Resurrected)
   , efResurrected(const_cast<TrigDecisionAuxInfo_v1 &>( rhs ).efResurrected)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   TrigDecisionAuxInfo_v1 &modrhs = const_cast<TrigDecisionAuxInfo_v1 &>( rhs );
   modrhs.tav.clear();
   modrhs.tap.clear();
   modrhs.tbp.clear();
   modrhs.lvl2PassedPhysics.clear();
   modrhs.efPassedPhysics.clear();
   modrhs.lvl2PassedRaw.clear();
   modrhs.efPassedRaw.clear();
   modrhs.lvl2PassedThrough.clear();
   modrhs.efPassedThrough.clear();
   modrhs.lvl2Prescaled.clear();
   modrhs.efPrescaled.clear();
   modrhs.lvl2Resurrected.clear();
   modrhs.efResurrected.clear();
}
xAOD::TrigDecisionAuxInfo_v1::~TrigDecisionAuxInfo_v1() {
}
#endif // xAOD__TrigDecisionAuxInfo_v1_cxx

#ifndef xAOD__TrigDecision_v1_cxx
#define xAOD__TrigDecision_v1_cxx
xAOD::TrigDecision_v1::TrigDecision_v1() {
}
xAOD::TrigDecision_v1 &xAOD::TrigDecision_v1::operator=(const TrigDecision_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<TrigDecision_v1 &>( rhs ));
   return *this;
}
xAOD::TrigDecision_v1::TrigDecision_v1(const TrigDecision_v1 & rhs)
   : SG::AuxElement(const_cast<TrigDecision_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::TrigDecision_v1::~TrigDecision_v1() {
}
#endif // xAOD__TrigDecision_v1_cxx

#ifndef xAOD__MissingETAuxAssociationMap_v2_cxx
#define xAOD__MissingETAuxAssociationMap_v2_cxx
xAOD::MissingETAuxAssociationMap_v2::MissingETAuxAssociationMap_v2() {
}
xAOD::MissingETAuxAssociationMap_v2 &xAOD::MissingETAuxAssociationMap_v2::operator=(const MissingETAuxAssociationMap_v2 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::AuxContainerBase::operator=(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ));
   jetLink = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).jetLink);
   objectLinks = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).objectLinks);
   calpx = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).calpx);
   calpy = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).calpy);
   calpz = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).calpz);
   cale = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).cale);
   calsumpt = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).calsumpt);
   calkey = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).calkey);
   trkpx = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).trkpx);
   trkpy = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).trkpy);
   trkpz = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).trkpz);
   trke = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).trke);
   trksumpt = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).trksumpt);
   trkkey = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).trkkey);
   jettrkpx = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).jettrkpx);
   jettrkpy = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).jettrkpy);
   jettrkpz = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).jettrkpz);
   jettrke = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).jettrke);
   jettrksumpt = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).jettrksumpt);
   overlapIndices = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).overlapIndices);
   overlapTypes = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).overlapTypes);
   isMisc = (const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).isMisc);
   MissingETAuxAssociationMap_v2 &modrhs = const_cast<MissingETAuxAssociationMap_v2 &>( rhs );
   modrhs.jetLink.clear();
   modrhs.objectLinks.clear();
   modrhs.calpx.clear();
   modrhs.calpy.clear();
   modrhs.calpz.clear();
   modrhs.cale.clear();
   modrhs.calsumpt.clear();
   modrhs.calkey.clear();
   modrhs.trkpx.clear();
   modrhs.trkpy.clear();
   modrhs.trkpz.clear();
   modrhs.trke.clear();
   modrhs.trksumpt.clear();
   modrhs.trkkey.clear();
   modrhs.jettrkpx.clear();
   modrhs.jettrkpy.clear();
   modrhs.jettrkpz.clear();
   modrhs.jettrke.clear();
   modrhs.jettrksumpt.clear();
   modrhs.overlapIndices.clear();
   modrhs.overlapTypes.clear();
   modrhs.isMisc.clear();
   return *this;
}
xAOD::MissingETAuxAssociationMap_v2::MissingETAuxAssociationMap_v2(const MissingETAuxAssociationMap_v2 & rhs)
   : xAOD::AuxContainerBase(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ))
   , jetLink(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).jetLink)
   , objectLinks(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).objectLinks)
   , calpx(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).calpx)
   , calpy(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).calpy)
   , calpz(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).calpz)
   , cale(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).cale)
   , calsumpt(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).calsumpt)
   , calkey(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).calkey)
   , trkpx(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).trkpx)
   , trkpy(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).trkpy)
   , trkpz(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).trkpz)
   , trke(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).trke)
   , trksumpt(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).trksumpt)
   , trkkey(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).trkkey)
   , jettrkpx(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).jettrkpx)
   , jettrkpy(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).jettrkpy)
   , jettrkpz(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).jettrkpz)
   , jettrke(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).jettrke)
   , jettrksumpt(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).jettrksumpt)
   , overlapIndices(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).overlapIndices)
   , overlapTypes(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).overlapTypes)
   , isMisc(const_cast<MissingETAuxAssociationMap_v2 &>( rhs ).isMisc)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   MissingETAuxAssociationMap_v2 &modrhs = const_cast<MissingETAuxAssociationMap_v2 &>( rhs );
   modrhs.jetLink.clear();
   modrhs.objectLinks.clear();
   modrhs.calpx.clear();
   modrhs.calpy.clear();
   modrhs.calpz.clear();
   modrhs.cale.clear();
   modrhs.calsumpt.clear();
   modrhs.calkey.clear();
   modrhs.trkpx.clear();
   modrhs.trkpy.clear();
   modrhs.trkpz.clear();
   modrhs.trke.clear();
   modrhs.trksumpt.clear();
   modrhs.trkkey.clear();
   modrhs.jettrkpx.clear();
   modrhs.jettrkpy.clear();
   modrhs.jettrkpz.clear();
   modrhs.jettrke.clear();
   modrhs.jettrksumpt.clear();
   modrhs.overlapIndices.clear();
   modrhs.overlapTypes.clear();
   modrhs.isMisc.clear();
}
xAOD::MissingETAuxAssociationMap_v2::~MissingETAuxAssociationMap_v2() {
}
#endif // xAOD__MissingETAuxAssociationMap_v2_cxx

#ifndef ElementLink_DataVector_xAOD__Jet_v1____cxx
#define ElementLink_DataVector_xAOD__Jet_v1____cxx
ElementLink<DataVector<xAOD::Jet_v1> >::ElementLink() {
}
ElementLink<DataVector<xAOD::Jet_v1> > &ElementLink<DataVector<xAOD::Jet_v1> >::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<DataVector<xAOD::Jet_v1> >::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<DataVector<xAOD::Jet_v1> >::~ElementLink() {
}
#endif // ElementLink_DataVector_xAOD__Jet_v1____cxx

#ifndef ElementLink_DataVector_xAOD__IParticle____cxx
#define ElementLink_DataVector_xAOD__IParticle____cxx
ElementLink<DataVector<xAOD::IParticle> >::ElementLink() {
}
ElementLink<DataVector<xAOD::IParticle> > &ElementLink<DataVector<xAOD::IParticle> >::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<DataVector<xAOD::IParticle> >::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<DataVector<xAOD::IParticle> >::~ElementLink() {
}
#endif // ElementLink_DataVector_xAOD__IParticle____cxx

#ifndef xAOD__MissingETAssociation_v1_cxx
#define xAOD__MissingETAssociation_v1_cxx
xAOD::MissingETAssociation_v1::MissingETAssociation_v1() {
}
xAOD::MissingETAssociation_v1 &xAOD::MissingETAssociation_v1::operator=(const MissingETAssociation_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<MissingETAssociation_v1 &>( rhs ));
   return *this;
}
xAOD::MissingETAssociation_v1::MissingETAssociation_v1(const MissingETAssociation_v1 & rhs)
   : SG::AuxElement(const_cast<MissingETAssociation_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::MissingETAssociation_v1::~MissingETAssociation_v1() {
}
#endif // xAOD__MissingETAssociation_v1_cxx

#ifndef xAOD__EventShape_v1_cxx
#define xAOD__EventShape_v1_cxx
xAOD::EventShape_v1::EventShape_v1() {
}
xAOD::EventShape_v1 &xAOD::EventShape_v1::operator=(const EventShape_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<EventShape_v1 &>( rhs ));
   return *this;
}
xAOD::EventShape_v1::EventShape_v1(const EventShape_v1 & rhs)
   : SG::AuxElement(const_cast<EventShape_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::EventShape_v1::~EventShape_v1() {
}
#endif // xAOD__EventShape_v1_cxx

#ifndef xAOD__Electron_v1_cxx
#define xAOD__Electron_v1_cxx
xAOD::Electron_v1::Electron_v1() {
}
xAOD::Electron_v1 &xAOD::Electron_v1::operator=(const Electron_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::Egamma_v1::operator=(const_cast<Electron_v1 &>( rhs ));
   return *this;
}
xAOD::Electron_v1::Electron_v1(const Electron_v1 & rhs)
   : xAOD::Egamma_v1(const_cast<Electron_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::Electron_v1::~Electron_v1() {
}
#endif // xAOD__Electron_v1_cxx

#ifndef xAOD__Egamma_v1_cxx
#define xAOD__Egamma_v1_cxx
xAOD::Egamma_v1::Egamma_v1() {
}
xAOD::Egamma_v1 &xAOD::Egamma_v1::operator=(const Egamma_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::IParticle::operator=(const_cast<Egamma_v1 &>( rhs ));
   return *this;
}
xAOD::Egamma_v1::Egamma_v1(const Egamma_v1 & rhs)
   : xAOD::IParticle(const_cast<Egamma_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::Egamma_v1::~Egamma_v1() {
}
#endif // xAOD__Egamma_v1_cxx

#ifndef xAOD__IParticle_cxx
#define xAOD__IParticle_cxx
xAOD::IParticle::IParticle() {
}
xAOD::IParticle &xAOD::IParticle::operator=(const IParticle & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<IParticle &>( rhs ));
   return *this;
}
xAOD::IParticle::IParticle(const IParticle & rhs)
   : SG::AuxElement(const_cast<IParticle &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::IParticle::~IParticle() {
}
#endif // xAOD__IParticle_cxx

#ifndef xAOD__Vertex_v1_cxx
#define xAOD__Vertex_v1_cxx
xAOD::Vertex_v1::Vertex_v1() {
}
xAOD::Vertex_v1 &xAOD::Vertex_v1::operator=(const Vertex_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<Vertex_v1 &>( rhs ));
   return *this;
}
xAOD::Vertex_v1::Vertex_v1(const Vertex_v1 & rhs)
   : SG::AuxElement(const_cast<Vertex_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::Vertex_v1::~Vertex_v1() {
}
#endif // xAOD__Vertex_v1_cxx

#ifndef xAOD__MissingET_v1_cxx
#define xAOD__MissingET_v1_cxx
xAOD::MissingET_v1::MissingET_v1() {
}
xAOD::MissingET_v1 &xAOD::MissingET_v1::operator=(const MissingET_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<MissingET_v1 &>( rhs ));
   return *this;
}
xAOD::MissingET_v1::MissingET_v1(const MissingET_v1 & rhs)
   : SG::AuxElement(const_cast<MissingET_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::MissingET_v1::~MissingET_v1() {
}
#endif // xAOD__MissingET_v1_cxx

#ifndef xAOD__Photon_v1_cxx
#define xAOD__Photon_v1_cxx
xAOD::Photon_v1::Photon_v1() {
}
xAOD::Photon_v1 &xAOD::Photon_v1::operator=(const Photon_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::Egamma_v1::operator=(const_cast<Photon_v1 &>( rhs ));
   return *this;
}
xAOD::Photon_v1::Photon_v1(const Photon_v1 & rhs)
   : xAOD::Egamma_v1(const_cast<Photon_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::Photon_v1::~Photon_v1() {
}
#endif // xAOD__Photon_v1_cxx

#ifndef xAOD__TauJet_v3_cxx
#define xAOD__TauJet_v3_cxx
xAOD::TauJet_v3::TauJet_v3() {
}
xAOD::TauJet_v3 &xAOD::TauJet_v3::operator=(const TauJet_v3 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::IParticle::operator=(const_cast<TauJet_v3 &>( rhs ));
   return *this;
}
xAOD::TauJet_v3::TauJet_v3(const TauJet_v3 & rhs)
   : xAOD::IParticle(const_cast<TauJet_v3 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::TauJet_v3::~TauJet_v3() {
}
#endif // xAOD__TauJet_v3_cxx

#ifndef xAOD__Muon_v1_cxx
#define xAOD__Muon_v1_cxx
xAOD::Muon_v1::Muon_v1() {
}
xAOD::Muon_v1 &xAOD::Muon_v1::operator=(const Muon_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::IParticle::operator=(const_cast<Muon_v1 &>( rhs ));
   return *this;
}
xAOD::Muon_v1::Muon_v1(const Muon_v1 & rhs)
   : xAOD::IParticle(const_cast<Muon_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::Muon_v1::~Muon_v1() {
}
#endif // xAOD__Muon_v1_cxx

#ifndef xAOD__TruthEvent_v1_cxx
#define xAOD__TruthEvent_v1_cxx
xAOD::TruthEvent_v1::TruthEvent_v1() {
}
xAOD::TruthEvent_v1 &xAOD::TruthEvent_v1::operator=(const TruthEvent_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::TruthEventBase_v1::operator=(const_cast<TruthEvent_v1 &>( rhs ));
   return *this;
}
xAOD::TruthEvent_v1::TruthEvent_v1(const TruthEvent_v1 & rhs)
   : xAOD::TruthEventBase_v1(const_cast<TruthEvent_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::TruthEvent_v1::~TruthEvent_v1() {
}
#endif // xAOD__TruthEvent_v1_cxx

#ifndef xAOD__TruthEventBase_v1_cxx
#define xAOD__TruthEventBase_v1_cxx
xAOD::TruthEventBase_v1::TruthEventBase_v1() {
}
xAOD::TruthEventBase_v1 &xAOD::TruthEventBase_v1::operator=(const TruthEventBase_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<TruthEventBase_v1 &>( rhs ));
   return *this;
}
xAOD::TruthEventBase_v1::TruthEventBase_v1(const TruthEventBase_v1 & rhs)
   : SG::AuxElement(const_cast<TruthEventBase_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::TruthEventBase_v1::~TruthEventBase_v1() {
}
#endif // xAOD__TruthEventBase_v1_cxx

#ifndef xAOD__CaloCluster_v1_cxx
#define xAOD__CaloCluster_v1_cxx
xAOD::CaloCluster_v1::CaloCluster_v1() {
}
xAOD::CaloCluster_v1 &xAOD::CaloCluster_v1::operator=(const CaloCluster_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::IParticle::operator=(const_cast<CaloCluster_v1 &>( rhs ));
   m_samplingPattern = (const_cast<CaloCluster_v1 &>( rhs ).m_samplingPattern);
   return *this;
}
xAOD::CaloCluster_v1::CaloCluster_v1(const CaloCluster_v1 & rhs)
   : xAOD::IParticle(const_cast<CaloCluster_v1 &>( rhs ))
   , m_samplingPattern(const_cast<CaloCluster_v1 &>( rhs ).m_samplingPattern)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::CaloCluster_v1::~CaloCluster_v1() {
}
#endif // xAOD__CaloCluster_v1_cxx

#ifndef xAOD__TruthParticle_v1_cxx
#define xAOD__TruthParticle_v1_cxx
xAOD::TruthParticle_v1::TruthParticle_v1() {
}
xAOD::TruthParticle_v1 &xAOD::TruthParticle_v1::operator=(const TruthParticle_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::IParticle::operator=(const_cast<TruthParticle_v1 &>( rhs ));
   return *this;
}
xAOD::TruthParticle_v1::TruthParticle_v1(const TruthParticle_v1 & rhs)
   : xAOD::IParticle(const_cast<TruthParticle_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::TruthParticle_v1::~TruthParticle_v1() {
}
#endif // xAOD__TruthParticle_v1_cxx

#ifndef xAOD__TruthVertex_v1_cxx
#define xAOD__TruthVertex_v1_cxx
xAOD::TruthVertex_v1::TruthVertex_v1() {
}
xAOD::TruthVertex_v1 &xAOD::TruthVertex_v1::operator=(const TruthVertex_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<TruthVertex_v1 &>( rhs ));
   return *this;
}
xAOD::TruthVertex_v1::TruthVertex_v1(const TruthVertex_v1 & rhs)
   : SG::AuxElement(const_cast<TruthVertex_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::TruthVertex_v1::~TruthVertex_v1() {
}
#endif // xAOD__TruthVertex_v1_cxx

#ifndef xAOD__Jet_v1_cxx
#define xAOD__Jet_v1_cxx
xAOD::Jet_v1::Jet_v1() {
}
xAOD::Jet_v1 &xAOD::Jet_v1::operator=(const Jet_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::IParticle::operator=(const_cast<Jet_v1 &>( rhs ));
   return *this;
}
xAOD::Jet_v1::Jet_v1(const Jet_v1 & rhs)
   : xAOD::IParticle(const_cast<Jet_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::Jet_v1::~Jet_v1() {
}
#endif // xAOD__Jet_v1_cxx

#ifndef xAOD__BTagging_v1_cxx
#define xAOD__BTagging_v1_cxx
xAOD::BTagging_v1::BTagging_v1() {
}
xAOD::BTagging_v1 &xAOD::BTagging_v1::operator=(const BTagging_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<BTagging_v1 &>( rhs ));
   return *this;
}
xAOD::BTagging_v1::BTagging_v1(const BTagging_v1 & rhs)
   : SG::AuxElement(const_cast<BTagging_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::BTagging_v1::~BTagging_v1() {
}
#endif // xAOD__BTagging_v1_cxx

#ifndef xAOD__TrackParticle_v1_cxx
#define xAOD__TrackParticle_v1_cxx
xAOD::TrackParticle_v1::TrackParticle_v1() {
}
xAOD::TrackParticle_v1 &xAOD::TrackParticle_v1::operator=(const TrackParticle_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::IParticle::operator=(const_cast<TrackParticle_v1 &>( rhs ));
   return *this;
}
xAOD::TrackParticle_v1::TrackParticle_v1(const TrackParticle_v1 & rhs)
   : xAOD::IParticle(const_cast<TrackParticle_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::TrackParticle_v1::~TrackParticle_v1() {
}
#endif // xAOD__TrackParticle_v1_cxx

#ifndef xAOD__JetAuxContainer_v1_cxx
#define xAOD__JetAuxContainer_v1_cxx
xAOD::JetAuxContainer_v1::JetAuxContainer_v1() {
}
xAOD::JetAuxContainer_v1 &xAOD::JetAuxContainer_v1::operator=(const JetAuxContainer_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::AuxContainerBase::operator=(const_cast<JetAuxContainer_v1 &>( rhs ));
   pt = (const_cast<JetAuxContainer_v1 &>( rhs ).pt);
   eta = (const_cast<JetAuxContainer_v1 &>( rhs ).eta);
   phi = (const_cast<JetAuxContainer_v1 &>( rhs ).phi);
   m = (const_cast<JetAuxContainer_v1 &>( rhs ).m);
   constituentLinks = (const_cast<JetAuxContainer_v1 &>( rhs ).constituentLinks);
   constituentWeights = (const_cast<JetAuxContainer_v1 &>( rhs ).constituentWeights);
   JetAuxContainer_v1 &modrhs = const_cast<JetAuxContainer_v1 &>( rhs );
   modrhs.pt.clear();
   modrhs.eta.clear();
   modrhs.phi.clear();
   modrhs.m.clear();
   modrhs.constituentLinks.clear();
   modrhs.constituentWeights.clear();
   return *this;
}
xAOD::JetAuxContainer_v1::JetAuxContainer_v1(const JetAuxContainer_v1 & rhs)
   : xAOD::AuxContainerBase(const_cast<JetAuxContainer_v1 &>( rhs ))
   , pt(const_cast<JetAuxContainer_v1 &>( rhs ).pt)
   , eta(const_cast<JetAuxContainer_v1 &>( rhs ).eta)
   , phi(const_cast<JetAuxContainer_v1 &>( rhs ).phi)
   , m(const_cast<JetAuxContainer_v1 &>( rhs ).m)
   , constituentLinks(const_cast<JetAuxContainer_v1 &>( rhs ).constituentLinks)
   , constituentWeights(const_cast<JetAuxContainer_v1 &>( rhs ).constituentWeights)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   JetAuxContainer_v1 &modrhs = const_cast<JetAuxContainer_v1 &>( rhs );
   modrhs.pt.clear();
   modrhs.eta.clear();
   modrhs.phi.clear();
   modrhs.m.clear();
   modrhs.constituentLinks.clear();
   modrhs.constituentWeights.clear();
}
xAOD::JetAuxContainer_v1::~JetAuxContainer_v1() {
}
#endif // xAOD__JetAuxContainer_v1_cxx

#ifndef xAOD__TauTrack_v1_cxx
#define xAOD__TauTrack_v1_cxx
xAOD::TauTrack_v1::TauTrack_v1() {
}
xAOD::TauTrack_v1 &xAOD::TauTrack_v1::operator=(const TauTrack_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   xAOD::IParticle::operator=(const_cast<TauTrack_v1 &>( rhs ));
   return *this;
}
xAOD::TauTrack_v1::TauTrack_v1(const TauTrack_v1 & rhs)
   : xAOD::IParticle(const_cast<TauTrack_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::TauTrack_v1::~TauTrack_v1() {
}
#endif // xAOD__TauTrack_v1_cxx

#ifndef xAOD__TrigComposite_v1_cxx
#define xAOD__TrigComposite_v1_cxx
xAOD::TrigComposite_v1::TrigComposite_v1() {
}
xAOD::TrigComposite_v1 &xAOD::TrigComposite_v1::operator=(const TrigComposite_v1 & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   SG::AuxElement::operator=(const_cast<TrigComposite_v1 &>( rhs ));
   return *this;
}
xAOD::TrigComposite_v1::TrigComposite_v1(const TrigComposite_v1 & rhs)
   : SG::AuxElement(const_cast<TrigComposite_v1 &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
xAOD::TrigComposite_v1::~TrigComposite_v1() {
}
#endif // xAOD__TrigComposite_v1_cxx

#ifndef ElementLink_DataVector_xAOD__Vertex_v1____cxx
#define ElementLink_DataVector_xAOD__Vertex_v1____cxx
ElementLink<DataVector<xAOD::Vertex_v1> >::ElementLink() {
}
ElementLink<DataVector<xAOD::Vertex_v1> > &ElementLink<DataVector<xAOD::Vertex_v1> >::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<DataVector<xAOD::Vertex_v1> >::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<DataVector<xAOD::Vertex_v1> >::~ElementLink() {
}
#endif // ElementLink_DataVector_xAOD__Vertex_v1____cxx

#ifndef ElementLink_DataVector_xAOD__TrackParticle_v1____cxx
#define ElementLink_DataVector_xAOD__TrackParticle_v1____cxx
ElementLink<DataVector<xAOD::TrackParticle_v1> >::ElementLink() {
}
ElementLink<DataVector<xAOD::TrackParticle_v1> > &ElementLink<DataVector<xAOD::TrackParticle_v1> >::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<DataVector<xAOD::TrackParticle_v1> >::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<DataVector<xAOD::TrackParticle_v1> >::~ElementLink() {
}
#endif // ElementLink_DataVector_xAOD__TrackParticle_v1____cxx

#ifndef ElementLink_DataVector_xAOD__Egamma_v1____cxx
#define ElementLink_DataVector_xAOD__Egamma_v1____cxx
ElementLink<DataVector<xAOD::Egamma_v1> >::ElementLink() {
}
ElementLink<DataVector<xAOD::Egamma_v1> > &ElementLink<DataVector<xAOD::Egamma_v1> >::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<DataVector<xAOD::Egamma_v1> >::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<DataVector<xAOD::Egamma_v1> >::~ElementLink() {
}
#endif // ElementLink_DataVector_xAOD__Egamma_v1____cxx

#ifndef ElementLink_DataVector_xAOD__TruthParticle_v1____cxx
#define ElementLink_DataVector_xAOD__TruthParticle_v1____cxx
ElementLink<DataVector<xAOD::TruthParticle_v1> >::ElementLink() {
}
ElementLink<DataVector<xAOD::TruthParticle_v1> > &ElementLink<DataVector<xAOD::TruthParticle_v1> >::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<DataVector<xAOD::TruthParticle_v1> >::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<DataVector<xAOD::TruthParticle_v1> >::~ElementLink() {
}
#endif // ElementLink_DataVector_xAOD__TruthParticle_v1____cxx

#ifndef ElementLink_DataVector_xAOD__CaloCluster_v1____cxx
#define ElementLink_DataVector_xAOD__CaloCluster_v1____cxx
ElementLink<DataVector<xAOD::CaloCluster_v1> >::ElementLink() {
}
ElementLink<DataVector<xAOD::CaloCluster_v1> > &ElementLink<DataVector<xAOD::CaloCluster_v1> >::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<DataVector<xAOD::CaloCluster_v1> >::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<DataVector<xAOD::CaloCluster_v1> >::~ElementLink() {
}
#endif // ElementLink_DataVector_xAOD__CaloCluster_v1____cxx

#ifndef ElementLink_DataVector_xAOD__BTagging_v1____cxx
#define ElementLink_DataVector_xAOD__BTagging_v1____cxx
ElementLink<DataVector<xAOD::BTagging_v1> >::ElementLink() {
}
ElementLink<DataVector<xAOD::BTagging_v1> > &ElementLink<DataVector<xAOD::BTagging_v1> >::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<DataVector<xAOD::BTagging_v1> >::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<DataVector<xAOD::BTagging_v1> >::~ElementLink() {
}
#endif // ElementLink_DataVector_xAOD__BTagging_v1____cxx

#ifndef ElementLink_DataVector_xAOD__TauTrack_v1____cxx
#define ElementLink_DataVector_xAOD__TauTrack_v1____cxx
ElementLink<DataVector<xAOD::TauTrack_v1> >::ElementLink() {
}
ElementLink<DataVector<xAOD::TauTrack_v1> > &ElementLink<DataVector<xAOD::TauTrack_v1> >::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<DataVector<xAOD::TauTrack_v1> >::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<DataVector<xAOD::TauTrack_v1> >::~ElementLink() {
}
#endif // ElementLink_DataVector_xAOD__TauTrack_v1____cxx

#ifndef ElementLink_DataVector_xAOD__TruthVertex_v1____cxx
#define ElementLink_DataVector_xAOD__TruthVertex_v1____cxx
ElementLink<DataVector<xAOD::TruthVertex_v1> >::ElementLink() {
}
ElementLink<DataVector<xAOD::TruthVertex_v1> > &ElementLink<DataVector<xAOD::TruthVertex_v1> >::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<DataVector<xAOD::TruthVertex_v1> >::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<DataVector<xAOD::TruthVertex_v1> >::~ElementLink() {
}
#endif // ElementLink_DataVector_xAOD__TruthVertex_v1____cxx

#ifndef ElementLink_DataVector_xAOD__MuonSegment_v1____cxx
#define ElementLink_DataVector_xAOD__MuonSegment_v1____cxx
ElementLink<DataVector<xAOD::MuonSegment_v1> >::ElementLink() {
}
ElementLink<DataVector<xAOD::MuonSegment_v1> > &ElementLink<DataVector<xAOD::MuonSegment_v1> >::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<DataVector<xAOD::MuonSegment_v1> >::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<DataVector<xAOD::MuonSegment_v1> >::~ElementLink() {
}
#endif // ElementLink_DataVector_xAOD__MuonSegment_v1____cxx

#ifndef ElementLink_DataVector_xAOD__NeutralParticle_v1____cxx
#define ElementLink_DataVector_xAOD__NeutralParticle_v1____cxx
ElementLink<DataVector<xAOD::NeutralParticle_v1> >::ElementLink() {
}
ElementLink<DataVector<xAOD::NeutralParticle_v1> > &ElementLink<DataVector<xAOD::NeutralParticle_v1> >::operator=(const ElementLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   ElementLinkBase::operator=(const_cast<ElementLink &>( rhs ));
   return *this;
}
ElementLink<DataVector<xAOD::NeutralParticle_v1> >::ElementLink(const ElementLink & rhs)
   : ElementLinkBase(const_cast<ElementLink &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
ElementLink<DataVector<xAOD::NeutralParticle_v1> >::~ElementLink() {
}
#endif // ElementLink_DataVector_xAOD__NeutralParticle_v1____cxx

