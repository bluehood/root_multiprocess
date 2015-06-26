#include "TClientServer.h"
#include "TSystem.h" //gSystem
#include "TRandom.h" //gRandom
#include "TServerSocket.h"
#include "TH1F.h"
#include "TCollection.h" //TIter
#include <cstdlib> //exit
#include <unistd.h> // close
#include <iostream>


//***** TClientServerHandler *****//
TClientServerHandler::TClientServerHandler(TClientServer *s, Int_t fd) : TFileHandler(fd, 1) {
	fClientServer = s;
}

Bool_t TClientServerHandler::Notify() {
   fClientServer->HandleInput();
   return kTRUE;
}

Bool_t TClientServerHandler::ReadNotify() {
	return Notify();
}


//***** TAnswer *****//
TAnswer::TAnswer() {
	retval = 0;
	msg = "";
	obj = 0;
}


//**** TClientSever ****//
TClientServer::TClientServer() {
	fSocketN = 9089;
	fIsParent = true;
}

void TClientServer::Fork() {
	fSocketN++;
	int pid = fork();

	if(pid) {
	//PARENT/CLIENT
		TServerSocket ss(fSocketN,kTRUE);
		//wait for child to connect
		fS = ss.Accept();
		fHandler = new TClientServerHandler(this, fS->GetDescriptor());	
		gSystem->AddFileHandler(fHandler);
	}
	else {
	//CHILD/SERVER
		fIsParent = false;
		//remove stdin from handlers and close it
		TIter next(gSystem->GetListOfFileHandlers());
		TFileHandler *h;
		while((h = (TFileHandler*)next())) {
			if(h->GetFd() == 0) {
				gSystem->RemoveFileHandler(h);
				break;
			}
		}
		close(0);
		//connect to parent
		unsigned n_fail = 0;
		do {
			gSystem->Sleep(10);
			fS = new TSocket("localhost",fSocketN);
			if(!fS) {
				++n_fail;
				if(n_fail>2) {
					std::cerr << "S: could not connect to parent. giving up." << std::endl;
					exit(1);
				}
			}
		} while(!fS);
		fHandler = new TClientServerHandler(this, fS->GetDescriptor());	
		gSystem->AddFileHandler(fHandler);
	}
}

void TClientServer::HandleInput() {
	std::cerr << (fIsParent?"C: ":"S: ") << "handle input on port " << fSocketN << std::endl;
	TMessage *m;
	if (fS->Recv(m) <= 0 || !m) {
		std::cerr << (fIsParent?"C: ":"S: ") << "error receiving message on port " << fSocketN << std::endl;
		return;
	}
	else {
		std::cerr << (fIsParent?"C: ":"S: ") << "processing message on port " << fSocketN << std::endl;
		switch(m->What()) {
			case kMESS_STRING:
			{
				char s[64];
				m->ReadString(s,64);
				std::cout << (fIsParent?"C: ":"S: ") << "message received: '" << s << "'" << std::endl;
				break;
			}
			case kMESS_ANY:
			{
				TAnswer *ans = (TAnswer*)m->ReadObjectAny(TAnswer::Class());	
				std::cout << (fIsParent?"C: ":"S: ") << "answer received: '" << ans->msg << "'" << std::endl;
				if(ans->obj)
					ans->obj->Draw();
				else
					std::cerr << "asra" << std::endl;
				break;
			}
			default:
			{
				std::cerr << (fIsParent?"C: ":"S: ") << "unexpected message received. msg type: " << m->What() << std::endl;
				break;
			}
		}
		delete m;
	}
	if(!fIsParent) {
		//create histogram and fill it
		TH1F* h = new TH1F("h1","h",100,-3,3);
		h->FillRandom("gaus",10000);
		//build TAnswer and send it back to client
		TAnswer *ans = new TAnswer;
		ans->retval = 0;
		ans->msg = "tutto ok";
		ans->obj = h;
		TMessage m;
		m.WriteObject(ans);
		Send(m);
	
	}
}

void TClientServer::Send(TString msg) const {
	fS->Send(msg);
}

void TClientServer::Send(TMessage& m) const {
	fS->Send(m);
}
