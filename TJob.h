#ifndef ROOT_TJob
#define ROOT_TJob

#include "TObject.h"
#include "TList.h"

class TJob : public TObject {
   ClassDef(TJob, 1);
public:
   virtual TObject *Process() = 0;
   virtual TList *Merge(TList *);
};

#endif
