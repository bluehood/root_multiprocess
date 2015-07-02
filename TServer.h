#ifndef ROOT_TServer
#define ROOT_TServer

#include "TMessage.h"
#include "TMonitor.h"
#include "TSocket.h"
#include "TNote.h"

class TServer {
   ClassDef(TServer,1);
   public:
   TServer(TSocket *s, unsigned serverN);
   void Run();
   void HandleInput(TMessage *&);
   void Send(TNote::ECode code, const TString &str = "", TObject* o = nullptr) const;

   private:
   TMonitor fMon;
   unsigned fServerN;
};

#endif
