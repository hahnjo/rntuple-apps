namespace std {}
using namespace std;
#include "FairRootProjectHeaders.h"

#include "FairRootLinkDef.h"

#include "FairRootProjectDict.cxx"

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

#ifndef FairFileHeader_cxx
#define FairFileHeader_cxx
FairFileHeader::FairFileHeader() {
   fTaskList = 0;
   fFileList = 0;
}
FairFileHeader &FairFileHeader::operator=(const FairFileHeader & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   TNamed::operator=(const_cast<FairFileHeader &>( rhs ));
   fRunId = (const_cast<FairFileHeader &>( rhs ).fRunId);
   fTaskList = (const_cast<FairFileHeader &>( rhs ).fTaskList);
   fFileList = (const_cast<FairFileHeader &>( rhs ).fFileList);
   FairFileHeader &modrhs = const_cast<FairFileHeader &>( rhs );
   modrhs.fTaskList = 0;
   modrhs.fFileList = 0;
   return *this;
}
FairFileHeader::FairFileHeader(const FairFileHeader & rhs)
   : TNamed(const_cast<FairFileHeader &>( rhs ))
   , fRunId(const_cast<FairFileHeader &>( rhs ).fRunId)
   , fTaskList(const_cast<FairFileHeader &>( rhs ).fTaskList)
   , fFileList(const_cast<FairFileHeader &>( rhs ).fFileList)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   FairFileHeader &modrhs = const_cast<FairFileHeader &>( rhs );
   modrhs.fTaskList = 0;
   modrhs.fFileList = 0;
}
FairFileHeader::~FairFileHeader() {
   delete fTaskList;   fTaskList = 0;
   delete fFileList;   fFileList = 0;
}
#endif // FairFileHeader_cxx

#ifndef FairMCTrack_cxx
#define FairMCTrack_cxx
FairMCTrack::FairMCTrack() {
}
FairMCTrack &FairMCTrack::operator=(const FairMCTrack & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   TObject::operator=(const_cast<FairMCTrack &>( rhs ));
   fPdgCode = (const_cast<FairMCTrack &>( rhs ).fPdgCode);
   fMotherId = (const_cast<FairMCTrack &>( rhs ).fMotherId);
   fPx = (const_cast<FairMCTrack &>( rhs ).fPx);
   fPy = (const_cast<FairMCTrack &>( rhs ).fPy);
   fPz = (const_cast<FairMCTrack &>( rhs ).fPz);
   fStartX = (const_cast<FairMCTrack &>( rhs ).fStartX);
   fStartY = (const_cast<FairMCTrack &>( rhs ).fStartY);
   fStartZ = (const_cast<FairMCTrack &>( rhs ).fStartZ);
   fStartT = (const_cast<FairMCTrack &>( rhs ).fStartT);
   fNPoints = (const_cast<FairMCTrack &>( rhs ).fNPoints);
   return *this;
}
FairMCTrack::FairMCTrack(const FairMCTrack & rhs)
   : TObject(const_cast<FairMCTrack &>( rhs ))
   , fPdgCode(const_cast<FairMCTrack &>( rhs ).fPdgCode)
   , fMotherId(const_cast<FairMCTrack &>( rhs ).fMotherId)
   , fPx(const_cast<FairMCTrack &>( rhs ).fPx)
   , fPy(const_cast<FairMCTrack &>( rhs ).fPy)
   , fPz(const_cast<FairMCTrack &>( rhs ).fPz)
   , fStartX(const_cast<FairMCTrack &>( rhs ).fStartX)
   , fStartY(const_cast<FairMCTrack &>( rhs ).fStartY)
   , fStartZ(const_cast<FairMCTrack &>( rhs ).fStartZ)
   , fStartT(const_cast<FairMCTrack &>( rhs ).fStartT)
   , fNPoints(const_cast<FairMCTrack &>( rhs ).fNPoints)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
FairMCTrack::~FairMCTrack() {
}
#endif // FairMCTrack_cxx

#ifndef FairTutPropPoint_cxx
#define FairTutPropPoint_cxx
FairTutPropPoint::FairTutPropPoint() {
}
FairTutPropPoint &FairTutPropPoint::operator=(const FairTutPropPoint & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   FairMCPoint::operator=(const_cast<FairTutPropPoint &>( rhs ));
   return *this;
}
FairTutPropPoint::FairTutPropPoint(const FairTutPropPoint & rhs)
   : FairMCPoint(const_cast<FairTutPropPoint &>( rhs ))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
FairTutPropPoint::~FairTutPropPoint() {
}
#endif // FairTutPropPoint_cxx

#ifndef FairMCPoint_cxx
#define FairMCPoint_cxx
FairMCPoint::FairMCPoint() {
}
FairMCPoint &FairMCPoint::operator=(const FairMCPoint & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   FairMultiLinkedData_Interface::operator=(const_cast<FairMCPoint &>( rhs ));
   fTrackID = (const_cast<FairMCPoint &>( rhs ).fTrackID);
   fEventId = (const_cast<FairMCPoint &>( rhs ).fEventId);
   fPx = (const_cast<FairMCPoint &>( rhs ).fPx);
   fPy = (const_cast<FairMCPoint &>( rhs ).fPy);
   fPz = (const_cast<FairMCPoint &>( rhs ).fPz);
   fTime = (const_cast<FairMCPoint &>( rhs ).fTime);
   fLength = (const_cast<FairMCPoint &>( rhs ).fLength);
   fELoss = (const_cast<FairMCPoint &>( rhs ).fELoss);
   fDetectorID = (const_cast<FairMCPoint &>( rhs ).fDetectorID);
   fX = (const_cast<FairMCPoint &>( rhs ).fX);
   fY = (const_cast<FairMCPoint &>( rhs ).fY);
   fZ = (const_cast<FairMCPoint &>( rhs ).fZ);
   return *this;
}
FairMCPoint::FairMCPoint(const FairMCPoint & rhs)
   : FairMultiLinkedData_Interface(const_cast<FairMCPoint &>( rhs ))
   , fTrackID(const_cast<FairMCPoint &>( rhs ).fTrackID)
   , fEventId(const_cast<FairMCPoint &>( rhs ).fEventId)
   , fPx(const_cast<FairMCPoint &>( rhs ).fPx)
   , fPy(const_cast<FairMCPoint &>( rhs ).fPy)
   , fPz(const_cast<FairMCPoint &>( rhs ).fPz)
   , fTime(const_cast<FairMCPoint &>( rhs ).fTime)
   , fLength(const_cast<FairMCPoint &>( rhs ).fLength)
   , fELoss(const_cast<FairMCPoint &>( rhs ).fELoss)
   , fDetectorID(const_cast<FairMCPoint &>( rhs ).fDetectorID)
   , fX(const_cast<FairMCPoint &>( rhs ).fX)
   , fY(const_cast<FairMCPoint &>( rhs ).fY)
   , fZ(const_cast<FairMCPoint &>( rhs ).fZ)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
FairMCPoint::~FairMCPoint() {
}
#endif // FairMCPoint_cxx

#ifndef FairMultiLinkedData_Interface_cxx
#define FairMultiLinkedData_Interface_cxx
FairMultiLinkedData_Interface::FairMultiLinkedData_Interface() {
   fLink = 0;
}
FairMultiLinkedData_Interface &FairMultiLinkedData_Interface::operator=(const FairMultiLinkedData_Interface & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   TObject::operator=(const_cast<FairMultiLinkedData_Interface &>( rhs ));
   fLink = std::move((const_cast<FairMultiLinkedData_Interface &>( rhs ).fLink));
   return *this;
}
FairMultiLinkedData_Interface::FairMultiLinkedData_Interface(const FairMultiLinkedData_Interface & rhs)
   : TObject(const_cast<FairMultiLinkedData_Interface &>( rhs ))
   , fLink(std::move(const_cast<FairMultiLinkedData_Interface &>( rhs ).fLink))
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
FairMultiLinkedData_Interface::~FairMultiLinkedData_Interface() {
}
#endif // FairMultiLinkedData_Interface_cxx

#ifndef FairMultiLinkedData_cxx
#define FairMultiLinkedData_cxx
FairMultiLinkedData::FairMultiLinkedData() {
}
FairMultiLinkedData &FairMultiLinkedData::operator=(const FairMultiLinkedData & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   TObject::operator=(const_cast<FairMultiLinkedData &>( rhs ));
   fLinks = (const_cast<FairMultiLinkedData &>( rhs ).fLinks);
   fEntryNr = (const_cast<FairMultiLinkedData &>( rhs ).fEntryNr);
   fDefaultType = (const_cast<FairMultiLinkedData &>( rhs ).fDefaultType);
   FairMultiLinkedData &modrhs = const_cast<FairMultiLinkedData &>( rhs );
   modrhs.fLinks.clear();
   return *this;
}
FairMultiLinkedData::FairMultiLinkedData(const FairMultiLinkedData & rhs)
   : TObject(const_cast<FairMultiLinkedData &>( rhs ))
   , fLinks(const_cast<FairMultiLinkedData &>( rhs ).fLinks)
   , fEntryNr(const_cast<FairMultiLinkedData &>( rhs ).fEntryNr)
   , fDefaultType(const_cast<FairMultiLinkedData &>( rhs ).fDefaultType)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   FairMultiLinkedData &modrhs = const_cast<FairMultiLinkedData &>( rhs );
   modrhs.fLinks.clear();
}
FairMultiLinkedData::~FairMultiLinkedData() {
}
#endif // FairMultiLinkedData_cxx

#ifndef FairLink_cxx
#define FairLink_cxx
FairLink::FairLink() {
}
FairLink &FairLink::operator=(const FairLink & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   fFile = (const_cast<FairLink &>( rhs ).fFile);
   fType = (const_cast<FairLink &>( rhs ).fType);
   fEntry = (const_cast<FairLink &>( rhs ).fEntry);
   fIndex = (const_cast<FairLink &>( rhs ).fIndex);
   fWeight = (const_cast<FairLink &>( rhs ).fWeight);
   return *this;
}
FairLink::FairLink(const FairLink & rhs)
   : fFile(const_cast<FairLink &>( rhs ).fFile)
   , fType(const_cast<FairLink &>( rhs ).fType)
   , fEntry(const_cast<FairLink &>( rhs ).fEntry)
   , fIndex(const_cast<FairLink &>( rhs ).fIndex)
   , fWeight(const_cast<FairLink &>( rhs ).fWeight)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
FairLink::~FairLink() {
}
#endif // FairLink_cxx

#ifndef FairMCEventHeader_cxx
#define FairMCEventHeader_cxx
FairMCEventHeader::FairMCEventHeader() {
}
FairMCEventHeader &FairMCEventHeader::operator=(const FairMCEventHeader & rhs)
{
   // This is NOT a copy operator=. This is actually a move operator= (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
   TObject::operator=(const_cast<FairMCEventHeader &>( rhs ));
   fRunId = (const_cast<FairMCEventHeader &>( rhs ).fRunId);
   fEventId = (const_cast<FairMCEventHeader &>( rhs ).fEventId);
   fX = (const_cast<FairMCEventHeader &>( rhs ).fX);
   fY = (const_cast<FairMCEventHeader &>( rhs ).fY);
   fZ = (const_cast<FairMCEventHeader &>( rhs ).fZ);
   fT = (const_cast<FairMCEventHeader &>( rhs ).fT);
   fB = (const_cast<FairMCEventHeader &>( rhs ).fB);
   fNPrim = (const_cast<FairMCEventHeader &>( rhs ).fNPrim);
   fIsSet = (const_cast<FairMCEventHeader &>( rhs ).fIsSet);
   fRotX = (const_cast<FairMCEventHeader &>( rhs ).fRotX);
   fRotY = (const_cast<FairMCEventHeader &>( rhs ).fRotY);
   fRotZ = (const_cast<FairMCEventHeader &>( rhs ).fRotZ);
   return *this;
}
FairMCEventHeader::FairMCEventHeader(const FairMCEventHeader & rhs)
   : TObject(const_cast<FairMCEventHeader &>( rhs ))
   , fRunId(const_cast<FairMCEventHeader &>( rhs ).fRunId)
   , fEventId(const_cast<FairMCEventHeader &>( rhs ).fEventId)
   , fX(const_cast<FairMCEventHeader &>( rhs ).fX)
   , fY(const_cast<FairMCEventHeader &>( rhs ).fY)
   , fZ(const_cast<FairMCEventHeader &>( rhs ).fZ)
   , fT(const_cast<FairMCEventHeader &>( rhs ).fT)
   , fB(const_cast<FairMCEventHeader &>( rhs ).fB)
   , fNPrim(const_cast<FairMCEventHeader &>( rhs ).fNPrim)
   , fIsSet(const_cast<FairMCEventHeader &>( rhs ).fIsSet)
   , fRotX(const_cast<FairMCEventHeader &>( rhs ).fRotX)
   , fRotY(const_cast<FairMCEventHeader &>( rhs ).fRotY)
   , fRotZ(const_cast<FairMCEventHeader &>( rhs ).fRotZ)
{
   // This is NOT a copy constructor. This is actually a move constructor (for stl container's sake).
   // Use at your own risk!
   (void)rhs; // avoid warning about unused parameter
}
FairMCEventHeader::~FairMCEventHeader() {
}
#endif // FairMCEventHeader_cxx

