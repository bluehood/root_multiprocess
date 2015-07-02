#ifndef ROOT_TMultiProcess
#define ROOT_TMultiProcess

#include "TMessage.h"
#include "TString.h"
#include "TMonitor.h"
#include "TJob.h"
#include "TNote.h"

class TMultiProcess {
   ClassDef(TMultiProcess, 1);
public:
   TMultiProcess();
   ~TMultiProcess();

   void Fork(unsigned n_forks = 0);
   void Broadcast(TNote::ECode code, const TString &str = "", TObject *o = nullptr);
   void Send(TNote::ECode code, const TString &str = "", TObject* o = nullptr, TSocket * = nullptr) const;
   void Collect();
   void CollectOne(TSocket * = nullptr);
   inline void SetResList(TList *l) { delete fResList; fResList = l; }
   inline TList* GetResList() const { return fResList; }

private:
   void HandleInput(TMessage *&msg, TSocket *sender);

   bool fIsParent;
   unsigned fPortN;
   unsigned fActiveServerN;
   unsigned fTotServerN;
   unsigned fNWorkers;
   TMonitor fMon;
   TList *fResList;
};

#endif
