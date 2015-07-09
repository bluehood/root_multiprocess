#ifndef ROOT_TMultiProcess
#define ROOT_TMultiProcess

#include "TMessage.h"
#include "TString.h"
#include "TMonitor.h"
#include "TNote.h"
#include <vector>
#include <unistd.h> //pid_t

class TMultiProcess {
   ClassDef(TMultiProcess, 0);
public:
   TMultiProcess();
   ~TMultiProcess();

   void Fork(unsigned n_forks = 0);
   void Broadcast(TNote::ECode code, const TString &str = "", TObject *o = nullptr);
   void Send(TNote::ECode code, const TString &str = "", TObject *o = nullptr, TSocket * = nullptr) const;
   void Collect();
   void CollectOne(TSocket * = nullptr);
   inline void SetResList(TList *l) { fResList = l; }
   inline TList *GetResList() const { return fResList; }
   inline bool GetIsParent() const { return fIsParent; }
   void ReapServers();

protected:
   virtual void HandleInput(TMessage *&msg, TSocket *sender);

private:
   bool fIsParent;
   unsigned fPortN;
   std::vector<pid_t> fServerPids;
   TMonitor fMon;
   TList *fResList;
};

#endif
