#include "TMultiProcess.h"
#include "TSystem.h" //gSystem
#include "TROOT.h" //gROOT
#include "TServerSocket.h"
#include "TSocket.h"
#include "TServer.h"
#include "TCollection.h" //TIter
#include <unistd.h> // close
#include <iostream>


TMultiProcess::TMultiProcess()
{
   fResList = nullptr;
   fIsParent = true;
   fPortN = 9090;
   fActiveServerN = 0;
   fTotServerN = 0;

   SysInfo_t si;
   if (gSystem->GetSysInfo(&si) == 0)
      fNWorkers = si.fCpus;
   else
      fNWorkers = 2;
}


TMultiProcess::~TMultiProcess()
{
   Broadcast(TNote::kShutdownOrder);
   fMon.RemoveAll(); //FIXME memleak? this calls TList::Delete, and this only deletes objects if it has ownership
   fResList->Clear();
   delete fResList;
}


void TMultiProcess::Fork(unsigned n_forks)
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
      if(!fResList)
         fResList = new TList;
   } else {
      //CHILD/SERVER
      gSystem->Sleep(100); //FIXME there must be a better way to synchronize connections
      fIsParent = false;

      //remove stdin from eventloop and close it
      TIter next(gSystem->GetListOfFileHandlers());
      TFileHandler *h;
      while ((h = (TFileHandler *)next())) {
         if (h->GetFd() == 0) {
            gSystem->RemoveFileHandler(h);
            break;
         }
      }
      close(0);

      //disable graphics
      gROOT->SetBatch();
      gXDisplay = 0;

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
      //instatiate server and enter loop
      TServer serv(s, fTotServerN);
      serv.Run();
      //we should never reach this point: TServer is in charge of closing the session
   }
}


void TMultiProcess::Broadcast(TNote::ECode code, const TString &str, TObject* o)
{
   fMon.ActivateAll();

   //send message to all sockets
   TIter next(fMon.GetListOfActives()); //FIXME memleak: does TIter's destructor delete the list too?
   TSocket *s;
   while ((s = (TSocket *)next()))
      Send(code, str, o, s);
   fMon.DeActivateAll();
}


void TMultiProcess::Send(TNote::ECode code, const TString &str, TObject* o, TSocket *s) const
{
   if (!s)
      s = (TSocket *)fMon.GetListOfActives()->First();
   TNote *n = new TNote;
   n->code = code;
   n->str = str;
   n->obj = o;
   TMessage msg; //TODO try with msg(kMESS_OBJECT) just to see what happens
   msg.WriteObject(n);
   s->Send(msg);
   //TODO try a `delete n` here
}


void TMultiProcess::Collect()
{
   fMon.ActivateAll();
   TIter next(fMon.GetListOfActives()); //FIXME memleak: does TIter's destructor delete the list too?
   TSocket *s;
   while ((s = (TSocket *)next()))
      CollectOne(s);
   fMon.DeActivateAll();
}


// N.B. this method assumes the TSocket is active in fMon
void TMultiProcess::CollectOne(TSocket *s)
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
      HandleInput(msg, s);
   }
   delete msg;
}


void TMultiProcess::HandleInput(TMessage  *&msg, TSocket *s)
{
   if (msg->What() == kMESS_ANY) {
      TNote *n = (TNote *)msg->ReadObjectAny(TNote::Class());
      if (n->code == TNote::kMessage) {
         std::cerr << "[I][C] message received: '" << n->str << "'\n";
      } else if (n->code == TNote::kError) {
         std::cerr << "[E][C] error message received: '" << n->str << "'\n";
      } else if (n->code == TNote::kClassResult || n->code == TNote::kMacroResult) {
         std::cerr << "[I][C] job result received from " << n->str << "\n";
         if (n->obj)
            fResList->Add(n->obj);
      } else if (n->code == TNote::kShutdownNotice) {
         std::cerr << "[I][C] shutdown notice received\n";
         fMon.Remove(s);
         fActiveServerN--;
         //TODO wait for child process to return? I don't have the pid though
      } else {
         std::cerr << "[W][C] unknown type of message received. code=" << to_string(n->code) << "\n";
      }
   } else {
      std::cerr << "[W][C] unexpected message received. type=" << msg->What() << "\n";
   }
}
