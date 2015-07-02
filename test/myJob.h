#ifndef myjob
#define myjob

#include "../TJob.h"

class myJob : public TJob {
   ClassDef(myJob,1);
   public:
   TObject *Process();
   //TObject* Merge(const TList&);
};

#endif
