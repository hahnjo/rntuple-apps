//////////////////////////////////////////////////////////
//   This class has been generated by TFile::MakeProject
//     (Wed Jul 23 15:04:55 2025 by ROOT version 6.36.02)
//      from the StreamerInfo in file FairRoot.rntuple
//////////////////////////////////////////////////////////


#ifndef FairMultiLinkedData_Interface_h
#define FairMultiLinkedData_Interface_h
class FairMultiLinkedData_Interface;

#include "Rtypes.h"
#include "TObject.h"
#include "FairMultiLinkedData.h"

class FairMultiLinkedData_Interface : public TObject {

public:
// Nested classes declaration.

public:
// Data Members.
   std::unique_ptr<FairMultiLinkedData> fLink;       //

   FairMultiLinkedData_Interface();
   FairMultiLinkedData_Interface(FairMultiLinkedData_Interface && ) = default;
   FairMultiLinkedData_Interface &operator=(const FairMultiLinkedData_Interface & );
   FairMultiLinkedData_Interface(const FairMultiLinkedData_Interface & );
   virtual ~FairMultiLinkedData_Interface();

   ClassDef(FairMultiLinkedData_Interface,7); // Generated by MakeProject.
};
#endif
