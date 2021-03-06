#ifndef ROOT_TPool
#define ROOT_TPool

#include "TMultiProcess.h"
#include "TString.h"

class TPool : private TMultiProcess {
   ClassDef(TPool, 0);
public:
   TList *Map(TString funcName);

private:
   void HandleInput(TMessage *&msg, TSocket *sender);
   void Merge();
};

#endif
