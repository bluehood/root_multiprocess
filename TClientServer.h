#ifndef ROOT_TClientServer
#define ROOT_TClientServer

#include "TMessage.h"
#include "TString.h"
#include "TMonitor.h"
#include "TJob.h"

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
	void Broadcast(unsigned code, TString msg = "");
	void Collect();
	void SetJob(TJob*);
	void SetMacroPath(const TString&);
	inline TString GetMacroPath() const { return fMacroPath; }
	//void Reset() TODO kill all servers, delete list of results, clean fMonitor, reset fServerN, remove fJob
	
	TList ResList;

	private:
	void Run();
	void Send(unsigned code, const TString& msg = "", TSocket * = nullptr) const;
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
