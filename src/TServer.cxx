#include "TServer.h"
#include "TNote.h"
#include "TSystem.h"
#include "TJob.h"
#include "TROOT.h"
#include <iostream>


TServer::TServer(TSocket *s) : TFileHandler(s->GetDescriptor(), kRead)
{
   fS = s;
   fPid = getpid();
}


void TServer::HandleInput(TMessage  *&msg)
{
   TString myId = "S" + std::to_string(fPid);
   if (msg->What() == kMESS_ANY) {
      TNote *n = (TNote *)msg->ReadObjectAny(TNote::Class());
      if (n->code == TNote::kMessage) {
         //general message
         Send(TNote::kMessage, "ok");
         //ignore it
      } else if (n->code == TNote::kError) {
         //general error
         Send(TNote::kMessage, "ko");
         //ignore it
      } else if (n->code == TNote::kExecClass) {
         //execute TJob::Process
         if (n->obj->InheritsFrom(TJob::Class())) {
            TJob *j = (TJob *) n->obj;
            TObject *res = j->Process();
            Send(TNote::kClassResult, myId, res);
         } else {
            Send(TNote::kError, "could not execute job. no job received");
            std::cerr << myId << ": could not execute job. no job received\n";
         }
         gSystem->Exit(0);
      } else if (n->code == TNote::kExecMacro) {
         //execute macro
         TString macro = n->str;
         //FIXME how do I check for errors in macro execution?
         TObject *res = (TObject *)gROOT->ProcessLine(macro + "()");
         Send(TNote::kMacroResult, myId, res);
         gSystem->Exit(0);
      } else if (n->code == TNote::kShutdownOrder) {
         //shutdown order
         Send(TNote::kShutdownNotice);
         gSystem->Exit(0);
      } else {
         Send(TNote::kError, myId + ": unknown code received. code=" + to_string(n->code));
      }
      delete n;
   } else {
      Send(TNote::kError, myId + ": unexpected message received. type=" + std::to_string(msg->What()));
      std::cerr << myId << ": unexpected message received. msg type: " << msg->What() << std::endl;
   }
}


void TServer::Send(TNote::ECode code, const TString &str, TObject *o) const
{
   TNote *n = new TNote;
   n->code = code;
   n->str = str;
   n->obj = o;
   TMessage msg; //TODO try with msg(kMESS_OBJECT) just to see what happens
   msg.WriteObject(n);
   fS->Send(msg);
   delete n;
}


Bool_t TServer::Notify()
{
   TMessage *msg;
   int nBytes = fS->Recv(msg);
   if (nBytes && msg) {
      HandleInput(msg);
      delete msg;
   } else {
      std::cerr << "S" << fPid << ": bad message received. shutting down\n";
      Send(TNote::kShutdownNotice); //send shutdown notice (even if nobody is listening)
      gSystem->Exit(0);
   }

   return kTRUE;
}
