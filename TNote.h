#ifndef ROOT_TNote
#define ROOT_TNote

#include "TString.h"
#include "TObject.h"

struct TNote : public TObject {
   ClassDef(TNote, 1);

   enum ECode : unsigned {
      //general
      kMessage,
      kError,
      //client codes
      kExecClass,
      kExecMacro,
      kShutdownOrder,
      //server codes
      kClassResult,
      kMacroResult,
      kShutdownNotice
   };

   TNote();
   ECode code;
   TString str;
   TObject *obj;
};

std::string to_string(TNote::ECode);

#endif
