#include "TServer.h"
#include "TNote.h"
#include "TSystem.h"
#include "TJob.h"
#include "TROOT.h"
#include <iostream>

TServer::TServer(TSocket *s, unsigned serverN) {
   fMon.Add(s);
   fServerN = serverN;
}


void TServer::Run() {
   while (true) {
      TSocket *s = fMon.Select();
      TMessage *msg;
      int nBytes = s->Recv(msg);
      if (nBytes && msg) {
         HandleInput(msg);
         delete msg;
      } else {
         std::cerr << "S" << fServerN << ": bad message received. shutting down\n";
         Send(TNote::kShutdownNotice); //send shutdown notice (even if nobody is listening)
         gSystem->Exit(0);
      }
   }
}


void TServer::HandleInput(TMessage  *&msg)
{
   TString head = "S" + std::to_string(fServerN) + ": ";
   if (msg->What() == kMESS_ANY) {
      TNote *n = (TNote *)msg->ReadObjectAny(TNote::Class());
      if (n->code == TNote::kMessage) {
         //general message
         Send(TNote::kMessage,"ok");
         //ignore it
      } else if (n->code == TNote::kError) {
         //general error
         Send(TNote::kMessage,"ko");
         //ignore it
      } else if (n->code == TNote::kExecClass) {
         //execute TJob::Process
         if(n->obj->InheritsFrom(TJob::Class())) {
            TJob* j = (TJob*) n->obj;
            TObject *res = j->Process();
            Send(TNote::kClassResult,"",res);
         } else {
            Send(TNote::kError, "could not execute job. no job received");
            std::cerr << head << "could not execute job. no job received\n";
         }
      } else if (n->code == TNote::kExecMacro) {
      /*
         //execute macro
         TString macro = n->str;
         macro.Remove(0,a.Last('/')+1);
         macro.Remove(macro.First('.'));
         //FIXME memleak: when should o and a be deleted?
         //FIXME how do I check for errors in macro execution?
         TObject *o = (TObject *)gROOT->ProcessLine(macro + "()");
      */
         Send(TNote::kError,"not implemented");
      } else if (n->code == TNote::kShutdownOrder) {
         //shutdown order
         Send(TNote::kShutdownNotice);
         gSystem->Exit(0);
      } else {
         Send(TNote::kError, head + "unknown code received. code=" + to_string(n->code));
      }
   } else {
      Send(TNote::kError, head + "unexpected message received. type=" + std::to_string(msg->What()));
      std::cerr << head << "unexpected message received. msg type: " << msg->What() << std::endl;
   }
}


void TServer::Send(TNote::ECode code, const TString &str, TObject* o) const
{
   TSocket *s = (TSocket *)fMon.GetListOfActives()->First();
   TNote *n = new TNote;
   n->code = code;
   n->str = str;
   n->obj = o;
   TMessage msg; //TODO try with msg(kMESS_OBJECT) just to see what happens
   msg.WriteObject(n);
   s->Send(msg);
   //TODO try a `delete n` here
}
