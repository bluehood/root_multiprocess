#ifndef ROOT_TPool
#define ROOT_TPool

#include "TMultiProcess.h"
#include "TString.h"

class TPool : private TMultiProcess {
   ClassDef(TPool,1);
   public:
   TList* Map(TString macro);

   private:
   void HandleInput(TMessage *&msg, TSocket *sender);
   void Merge();
};

#endif
