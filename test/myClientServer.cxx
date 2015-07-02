#include "myClientServer.h"

void myClientServer::ProcessJob(TJob *j)
{
   SetJob(j);
   Fork();
   Broadcast(TNote::kExecClass);
   Collect();
   Broadcast(TNote::kShutdownOrder);
   Collect();
   TList *newRes = GetJob()->Merge(GetResList());
   SetResList(newRes);
}


void myClientServer::ProcessMacro(const TString &path)
{
   SetMacroPath(path);
   Fork();
   Broadcast(TNote::kExecMacro);
   Collect();
   Broadcast(TNote::kShutdownOrder);
   Collect();
   //if Merge is defined
   //TList *newRes = Merge(fResList);
   //delete fResList;
   //fResList = newRes;
}
