#ifndef ROOT_TServer
#define ROOT_TServer

#include "TSysEvtHandler.h"
#include "TMessage.h"
#include "TMonitor.h"
#include "TSocket.h"
#include "TNote.h"
#include <unistd.h> //pid_t

class TServer : public TFileHandler {
   ClassDef(TServer,0);
   public:
   TServer(TSocket *s);
   void HandleInput(TMessage *&);
   void Send(TNote::ECode code, const TString &str = "", TObject* o = nullptr) const;
   
   Bool_t   Notify();
   Bool_t   ReadNotify() { return Notify(); }

   private:
   TSocket *fS;
   pid_t fPid;
};

#endif
