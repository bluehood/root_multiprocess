#ifndef ROOT_TClientServer
#define ROOT_TClientServer

#include "TSocket.h"
#include "TMessage.h"
#include "TString.h"
#include "TSysEvtHandler.h" // TFileHandler

class TClientServer;

class TClientServerHandler : public TFileHandler {
	ClassDef(TClientServerHandler, 1);
	public:
	TClientServerHandler(TClientServer *s, Int_t fd);
   Bool_t Notify();
   Bool_t ReadNotify();
	private:
	TClientServer *fClientServer;
};


struct TAnswer : public TObject {
	ClassDef(TAnswer, 1);
	TAnswer();
	unsigned retval;
	TString msg;
	TObject *obj;
};


class TClientServer {
	ClassDef(TClientServer, 1);
	public:
	TClientServer();
	void Fork();
	void Send(TString) const;
	void Send(TMessage&) const;
	void HandleInput();

	private:
	bool fIsParent;
	TSocket *fS;
	unsigned fSocketN;
	TFileHandler *fHandler;
};

#endif
