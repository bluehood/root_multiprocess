#include "TClientServer.h"
#include "TSystem.h"
#include <cstdlib> //exit
#include <iostream>

TClientServer::TClientServer() {
	fIsParent = true;
	fSocketPath = "/tmp/plsocket"; //TODO we should use tempnam for this
}

TClientServer::~TClientServer() {
}

void TClientServer::Fork() {
	TServerSocket ss(9090,kTRUE);
	int pid = fork();
	if(pid) { //parent
		// wait for child to connect
		TSocket *s = ss.Accept();
		fMon.Add(s);
	}
	else { //child
		fIsParent = false;
		// connect to parent
		unsigned n_fail = 0;
		do {
			gSystem->Sleep(10);
			fS = new TSocket("localhost",9090);
			if(!fS && n_fail > 2) {
				std::cerr << "S: could not connect to parent. giving up." << std::endl;
				exit(1);
			}
			else {
				++n_fail;
			}
		} while(!fS);
		fMon.Add(fS);
		Run();
	}
}

void TClientServer::Run() {
	while(true) {
		fS = fMon.Select(); //FIXME this returns a "valid" socket even if client has exited
		if(fS->IsValid()) {
			TMessage *m;
			fS->Recv(m); // if client has exited, though, m==nullptr
			unsigned r = HandleInput(m);
			delete m;
			TMessage toSend(kMESS_STRING);
			toSend.WriteStdString("message processed");
			fS->Send(toSend);
			gSystem->Sleep(600000);
		}
		else {
			fMon.Remove(fS);
			exit(0);
		}
	}
}

unsigned TClientServer::HandleInput(TMessage* m) {
	//std::cerr << "m: " << m << std::endl;
	switch(m->What()) {
		case kMESS_STRING:
		{
			char s[64];
			m->ReadString(s,64);
			std::cout << "S: message received from client: '" << s << "'" << std::endl;
			return 0;
		}
		default:
		{
			std::cerr << "S: unexpected message received from client" << std::endl;
			return 1;
		}
	}
}

void TClientServer::Send(std::string msg) const {
	TIter first(fMon.GetListOfActives());	
	if(TSocket *s = (TSocket*)first())
		s->Send(msg.c_str());
}

std::string TClientServer::Recv() {
	TSocket *s = fMon.Select();
	TMessage *m;
	s->Recv(m);
	if(m->What() == kMESS_STRING) {
		std::string msg;
		m->ReadStdString(msg);
		return msg;
	}
	delete m;
}
