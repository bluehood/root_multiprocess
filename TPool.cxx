#include "TPool.h"
#include "TNote.h"
#include "TClass.h"
#include "TMethodCall.h"
#include <iostream>

TList *TPool::Map(TString macro)
{
   Fork();
   if (!GetIsParent())
      return nullptr;
   Broadcast(TNote::kExecMacro, macro);
   Collect();
   Merge();
   ReapServers();
   return GetResList();
}


void TPool::HandleInput(TMessage *&msg, TSocket *sender)
{
   TMultiProcess::HandleInput(msg, sender);
}


//TODO handle the case of ResList as list of lists
void TPool::Merge()
{
   TList *l = GetResList();
   if (l->GetSize() == 0)
      return;

   TList *retl = new TList;

   TClass *c = l->First()->IsA();
   if (c->GetMerge()) {
      TObject *merged = l->First();
      l->RemoveFirst();
      TMethodCall callEnv;
      if (merged->IsA())
         callEnv.InitWithPrototype(merged->IsA(), "Merge", "TCollection*");
      if (callEnv.IsValid()) {
         callEnv.SetParam((Long_t) l);
         callEnv.Execute(merged);
      }
      l->Delete();
      delete l;
      retl->Add(merged);
      SetResList(retl);
   }
}
