#ifndef ROOT_TServer
#define ROOT_TServer

#include "TMessage.h"
#include "TMonitor.h"
#include "TSocket.h"
#include "TNote.h"
#include <unistd.h> //pid_t

class TServer {
   ClassDef(TServer,1);
   public:
   TServer(TSocket *s);
   void Run();
   void HandleInput(TMessage *&);
   void Send(TNote::ECode code, const TString &str = "", TObject* o = nullptr) const;

   private:
   TMonitor fMon;
   pid_t fPid;
};

#endif
