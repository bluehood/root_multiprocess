#include "TClientServer.h"
#include "TSystem.h" //gSystem
#include "TROOT.h" //gROOT
#include "TServerSocket.h"
#include "TSocket.h"
#include "TCollection.h" //TIter
#include <unistd.h> // close
#include <iostream>


TClientServer::TClientServer()
{
   fResList = new TList;
   fIsParent = true;
   fPortN = 9090;
   fActiveServerN = 0;
   fTotServerN = 0;
   fJob = nullptr;
   fMacroPath = "";

   SysInfo_t si;
   if (gSystem->GetSysInfo(&si) == 0)
      fNWorkers = si.fCpus;
   else
      fNWorkers = 2;
}


TClientServer::~TClientServer()
{
   Broadcast(TNote::kShutdownOrder);
   fMon.RemoveAll(); //FIXME memleak? this calls TList::Delete, and this only deletes objects if it has ownership
}


void TClientServer::Fork(unsigned n_forks)
{
   if (!n_forks)
      n_forks = fNWorkers;
   int pid;
   for (unsigned i = 0; i < n_forks; ++i) {
      fActiveServerN++;
      fTotServerN++;
      pid = fork();
      if (!pid)
         break;
   }

   if (pid) {
      //PARENT/CLIENT
      TServerSocket ss(fPortN, kTRUE);
      //wait for child to connect
      for (unsigned i = 0; i < n_forks; ++i) {
         TSocket *s = ss.Accept();
         fMon.Add(s);
         fMon.DeActivate(s);
      }
   } else {
      //CHILD/SERVER
      gSystem->Sleep(100); //FIXME there must be a better way to synchronize connections
      fIsParent = false;

      //remove stdin from handlers and close it
      TIter next(gSystem->GetListOfFileHandlers());
      TFileHandler *h;
      while ((h = (TFileHandler *)next())) {
         if (h->GetFd() == 0) {
            gSystem->RemoveFileHandler(h);
            break;
         }
      }
      close(0);

      //connect to parent
      unsigned n_fail = 0;
      TSocket *s;
      do {
         gSystem->Sleep(10);
         s = new TSocket("localhost", fPortN);
         if (!s)
            ++n_fail;
      } while (!s && n_fail < 3);
      if (!s) {
         std::cerr << "[E] S" << fTotServerN << ": could not connect to parent, giving up\n";
         gSystem->Exit(1);
      }
      fMon.Add(s);
      //enter loop
      Run();
   }
}


void TClientServer::Broadcast(TNote::ECode code, const TString &str)
{
   fMon.ActivateAll();

   //send message to all sockets
   TIter next(fMon.GetListOfActives()); //FIXME memleak: does TIter's destructor delete the list too?
   TSocket *s;
   while ((s = (TSocket *)next()))
      Send(code, str, s);
   fMon.DeActivateAll();
}


void TClientServer::Collect()
{
   fMon.ActivateAll();
   TIter next(fMon.GetListOfActives()); //FIXME memleak: does TIter's destructor delete the list too?
   TSocket *s;
   while ((s = (TSocket *)next()))
      CollectOne(s);
   fMon.DeActivateAll();
}


void TClientServer::SetMacroPath(const TString &path)
{
   gROOT->ProcessLine(".L " + path + "+");
   //TODO what if I couldn't load it
   fMacroPath = path;
}


void TClientServer::Run()   //FIXME interrupt events (ctrl-C) interrupt this too
{
   while (true) {
      TSocket *s = fMon.Select();
      TMessage *msg;
      int nBytes = s->Recv(msg);
      if (nBytes) {
         ServerHandleInput(msg);
         delete msg;
      } else {
         Send(TNote::kShutdownNotice); //send shutdown notice (even if nobody is listening)
         gSystem->Exit(0);
      }
   }
}


void TClientServer::Send(TNote::ECode code, const TString &str, TSocket *s) const
{
   if (!s)
      s = (TSocket *)fMon.GetListOfActives()->First();
   TNote *n = new TNote;
   n->code = code;
   n->str = str;
   TMessage msg; //TODO try with msg(kMESS_OBJECT) just to see what happens
   msg.WriteObject(n);
   s->Send(msg);
   //TODO try a `delete n` here
}


void TClientServer::Send(const TMessage &msg, TSocket *s) const
{
   if (!s)
      s = (TSocket *)fMon.GetListOfActives()->First();
   s->Send(msg);
}


// this method assumes the socket is active
void TClientServer::CollectOne(TSocket *s)
{
   if (!s)
      s = fMon.Select();
   TMessage *msg;
   int nBytes = s->Recv(msg);
   if (nBytes == 0 || !msg) {
      std::cerr << "[I][C] lost connection to a server\n";
      fMon.Remove(s);
      fActiveServerN--;
   } else {
      ClientHandleInput(msg, s);
   }
   delete msg;
}


void TClientServer::ClientHandleInput(TMessage  *&msg, TSocket *s)
{
   if (msg->What() == kMESS_ANY) {
      TNote *n = (TNote *)msg->ReadObjectAny(TNote::Class());
      if (n->code == TNote::kMessage) {
         std::cerr << "[I][C] message received: '" << n->str << "\n";
      } else if (n->code == TNote::kError) {
         std::cerr << "[E][C] error message received: '" << n->str << "\n";
      } else if (n->code == TNote::kClassResult || n->code == TNote::kMacroResult) {
         std::cerr << "[I][C] job result received from " << n->str << "\n";
         if (n->obj)
            fResList->Add(n->obj);
      } else if (n->code == TNote::kShutdownNotice) {
         std::cerr << "[I][C] shutdown notice received\n";
         fMon.Remove(s);
         fActiveServerN--;
      } else {
         std::cerr << "[W][C] unknown type of message received. code=" << to_string(n->code) << "\n";
      }
   } else {
      std::cerr << "[W][C] unexpected message received. type=" << msg->What() << "\n";
   }
}


void TClientServer::ServerHandleInput(TMessage  *&msg)
{
   TString head = "S" + std::to_string(fTotServerN) + ": ";
   if (msg->What() == kMESS_ANY) {
      TNote *n = (TNote *)msg->ReadObjectAny(TNote::Class());
      if (n->code == TNote::kMessage) {
         //general message
         //ignore it
      } else if (n->code == TNote::kError) {
         //general error
         //ignore it
      } else if (n->code == TNote::kExecClass) {
         //execute fJob->Process
         if (fJob) {
            TObject *res = fJob->Process(); //FIXME memleak?
            TNote *ans = new TNote;
            ans->code = TNote::kClassResult;
            ans->str = "S" + std::to_string(fTotServerN);
            if (res)
               ans->obj = res;
            TMessage m; //TODO try with m(kMESS_OBJECT)
            m.WriteObject(ans);
            Send(m);
         } else {
            Send(TNote::kError, "could not execute job: job not set");
            std::cerr << head << "could not execute job: job not set\n";
         }
      } else if (n->code == TNote::kExecMacro) {
         //execute macro
         TString macro = fMacroPath;
         macro.Remove(macro.First('.'));
         //FIXME memleak: when should o and a be deleted?
         TObject *o = (TObject *)gROOT->ProcessLine(macro + "()");
         TNote *ans = new TNote;
         ans->code = TNote::kMacroResult;
         ans->str = "S" + std::to_string(fTotServerN);
         if (o)
            ans->obj = o;
         TMessage m;
         m.WriteObject(ans);
         Send(m);
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
