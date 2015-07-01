#ifndef ROOT_TClientServer
#define ROOT_TClientServer

#include "TMessage.h"
#include "TString.h"
#include "TMonitor.h"
#include "TJob.h"
#include <string>

//I need an enum class so I can use simple names enclosed in a scope
//instead of inventing exotic variable names
enum class Code : unsigned {
	//general
	message,
	error,
	//client codes
	execClass,
	execMacro,
	shutdownOrder,
	//server codes
	classResult,
	macroResult,
	shutdownNotice
};

std::string to_string(Code);


struct TNote : public TObject {
	ClassDef(TNote, 1);
	TNote();
	Code code;
	TString str;
	TObject *obj;
};


class TClientServer {
	ClassDef(TClientServer, 1);
	public:
	TClientServer();
	TClientServer(TJob *);
	TClientServer(const TString& macroPath);
	~TClientServer();
	void Fork(unsigned n_forks);
	void Broadcast(Code code, TString msg = "");
	void Collect();
	void SetJob(TJob*);
	void SetMacroPath(const TString&);
	inline TString GetMacroPath() const { return fMacroPath; }
	//void Reset() TODO kill all servers, delete list of results, clean fMonitor, reset fServerN, remove fJob
	
	TList ResList;

	private:
	void Run();
	void Send(Code code, const TString& msg = "", TSocket * = nullptr) const;
	void Send(const TMessage&, TSocket * = nullptr) const;
	void CollectOne(TSocket* = nullptr);
	void ClientHandleInput(TMessage*&, TSocket*);
	void ServerHandleInput(TMessage*&);
	
	bool fIsParent;
	unsigned fPortN;
	unsigned fActiveServerN;
	unsigned fTotServerN;
	TMonitor fMon;
	TJob *fJob;
	TString fMacroPath;

};

#endif
