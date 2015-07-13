#ifndef ROOT_TLocalServer
#define ROOT_TLocalServer

#include "TServer.h"

class TLocalServer : public TServer {
   //ClassDef(TLocalServer, 0);
   public:
   TLocalServer(TSocket *s);
};

#endif
