#include "TClientServer.h"
#include "TSystem.h" //gSystem
#include "TROOT.h" //gROOT
#include "TServerSocket.h"
#include "TSocket.h"
#include "TCollection.h" //TIter
#include <unistd.h> // close
#include <iostream>


//***** TNote *****//
TNote::TNote() {
	code = 0;
	str = "";
	obj = nullptr;
}


//**** TClientSever ****//
TClientServer::TClientServer() {
	fIsParent = true;
	fPortN = 9090;
	fServerN = 0;
	fJob = nullptr;
	fMacroPath = "";
}


TClientServer::TClientServer(TJob *j) : TClientServer() {
	SetJob(j);
}


TClientServer::TClientServer(const TString& macroPath) : TClientServer() {
	SetMacroPath(macroPath);
}


TClientServer::~TClientServer() {
	fMon.RemoveAll(); //FIXME memleak: this calls TList::Delete, and this only deletes objects if it has ownership
}


void TClientServer::Fork(unsigned n_forks) {
	int pid;
	for(unsigned i=0; i<n_forks; ++i) {
		fServerN++;
		pid = fork();
		if(!pid)
			break;
	}

	if(pid) {
	//PARENT/CLIENT
		TServerSocket ss(fPortN,kTRUE);
		//wait for child to connect
		for(unsigned i=0; i<n_forks; ++i) {
			TSocket* s = ss.Accept();
			fMon.Add(s);
		}
	}
	else {
	//CHILD/SERVER
		gSystem->Sleep(100); //FIXME there must be a better way to synchronize connections
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
		TSocket *s;
		do {
			gSystem->Sleep(10);
			s = new TSocket("localhost",fPortN);
			if(!s)
				++n_fail;
		} while(!s && n_fail<3);
		if(!s) {
			std::cerr << "[E] S" << fServerN << ": could not connect to parent, giving up\n";
			gSystem->Exit(1);
		}
		fMon.Add(s);
		//enter loop
		Run();
	}
}


void TClientServer::Broadcast(unsigned code, TString str) const {
	//set parameters for macro execution message
	if(code == 3) {
		if(str != "")
			std::cerr << "[W][C] code is \"execute macro\" but string is not empty. String will be ignored\n";
		str = fMacroPath;
		if(str.First('.') > 0)
			str.Remove(str.First('.'));
	}

	//send message to all sockets
	TIter next(fMon.GetListOfActives()); //FIXME memleak: does TIter's destructor delete the list too?
	TSocket *s;
	while((s = (TSocket*)next()))
		Send(code, str, s);
}


void TClientServer::CollectOne(TSocket *s) {
	if(!s)
		s = fMon.Select();
	TMessage *msg;
	int nBytes = s->Recv(msg);
	if(nBytes == 0 || !msg) {
		std::cerr << "[I][C] lost connection to a server\n";
		fMon.Remove(s);
		fServerN--;
	}
	else {
		ClientHandleInput(msg, s);
	}
	delete msg;
}


void TClientServer::CollectAll() {
	TIter next(fMon.GetListOfActives()); //FIXME memleak: does TIter's destructor delete the list too?
	TSocket *s;
	while((s = (TSocket*)next()))
		CollectOne(s);
}


void TClientServer::SetJob(TJob* j) {
	if(j)
		fJob = j;
	else
		std::cerr << "[E]C: could not set job. invalid address\n";
}


void TClientServer::SetMacroPath(const TString& path) {
	gROOT->ProcessLine(".L "+path+"+");
	//TODO what if I couldn't load it
	fMacroPath = path;
}


void TClientServer::Run() { //FIXME interrupt events (ctrl-C) interrupt this too
	while(true) {
		TSocket *s = fMon.Select();
		TMessage *msg;
		int nBytes = s->Recv(msg);
		if(nBytes) {
			ServerHandleInput(msg);
			delete msg;
		}
		else {
			Send(3); //send shutdown notice (even if nobody is listening)
			gSystem->Exit(0);
		}
	}
}


void TClientServer::Send(unsigned code, const TString& str, TSocket *s) const {
	if(!s)
		s = (TSocket*)fMon.GetListOfActives()->First();
	TNote *n = new TNote;
	n->code = code;
	n->str = str;
	TMessage msg; //TODO try with msg(kMESS_OBJECT) just to see what happens
	msg.WriteObject(n);
	s->Send(msg);
	//TODO try a `delete n` here
}


void TClientServer::Send(const TMessage& msg, TSocket *s) const {
	if(!s)
		s = (TSocket*)fMon.GetListOfActives()->First();
	s->Send(msg);
}


void TClientServer::ClientHandleInput(TMessage *& msg, TSocket* s) {
	if(msg->What() == kMESS_ANY) {
		TNote *n = (TNote*)msg->ReadObjectAny(TNote::Class());
		if(n->code == 0) {
			std::cerr << "[I][C] message received: '" << n->str << "\n";
		}
		else if(n->code == 1) {
			std::cerr << "[E][C] error message received: '" << n->str << "\n";
		}
		else if(n->code == 2) {
			std::cerr << "[I][C] job result received from " << n->str << "\n";
			if(n->obj)
				ResList.Add(n->obj);
		}
		else if(n->code == 3) {
			std::cerr << "[I][C] shutdown notice received\n";
			fMon.Remove(s);
			fServerN--;
		}
		else {
			std::cerr << "[W][C] unknown type of message received. code=" << n->code << "\n";
		}
	}
	else {
		std::cerr << "[W][C] unexpected message received. type=" << msg->What() << "\n";
	}
}


void TClientServer::ServerHandleInput(TMessage *& msg) {
	TString head = "S"+std::to_string(fServerN)+": ";
	if(msg->What() == kMESS_ANY) {
		TNote *n = (TNote*)msg->ReadObjectAny(TNote::Class());
		if(n->code == 0) {
		//general message
			//ignore it
		}
		else if(n->code == 1) {
		//general error
			//ignore it
		}
		else if(n->code == 2) {
		//execute fJob->Process
			if(fJob) {
				TObject *res = fJob->Process(); //FIXME memleak?
				TNote *ans = new TNote;
				ans->code = 2;
				ans->str = "S"+std::to_string(fServerN);
				if(res)
					ans->obj = res;
				TMessage m; //TODO try with m(kMESS_OBJECT)
				m.WriteObject(ans);
				Send(m);
			}
			else {
				Send(1,"could not execute job: job not set");
				std::cerr << head << "could not execute job: job not set\n";
			}
		}
		else if(n->code == 3) {
		//execute macro
		//FIXME memleak: when shoud this stuff be deleted?
			TObject *o = (TObject*)gROOT->ProcessLine(n->str+"()");
			TNote *ans = new TNote;
			ans->code = 2;
			ans->str = "S"+std::to_string(fServerN);
			if(o)
				ans->obj = o;
			TMessage m;
			m.WriteObject(ans);
			Send(m);
		}
		else if(n->code == 4) {
		//shutdown order
			Send(3);
			gSystem->Exit(0);
		}
		else {
			Send(1,head+"unknown code received. code="+std::to_string(n->code));
		}
	}
	else {
		Send(1,head+"unexpected message received. type="+std::to_string(msg->What()));
		std::cerr << head << "unexpected message received. msg type: " << msg->What() << std::endl;
	}
}
