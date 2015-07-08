#include "TPool.h"
#include "TNote.h"
#include <iostream>

TList* TPool::Map(TString macro) {
   Fork();
   Broadcast(TNote::kExecMacro,macro);
   Collect();
   SetResList(Merge());
   std::cerr << GetResList() << std::endl;
   ReapServers();
   return GetResList();
}


void TPool::HandleInput(TMessage *&msg, TSocket *sender) {
   TMultiProcess::HandleInput(msg,sender);
}


TList* TPool::Merge() {
   return GetResList();
}
