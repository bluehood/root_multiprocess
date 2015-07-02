#include "TCollection.h"
#include "TClass.h"
#include "TJob.h"

//loop over the list of results
//if object has Merge, put merged version in output list
//otherwise put list of unmerged objects in output list
//TODO support for list of lists
/*TList* TJob::Merge(TList* l) {
   TList* retl = new TList;

   TClass *c = l->First()->IsA();
   if(c->GetMerge()) {
      TObject *merged = (TObject*)c->New();
      l->RemoveFirst();
      merged->Merge(l);
   }
   else {

   }

   return retl;
}
*/

TList *TJob::Merge(TList *l)
{
   TList *retl = new TList;
   TIter next(l);
   while (TObject *o = next()) {
      retl->Add(o);
   }
   return retl;
}
