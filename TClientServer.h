#ifndef ROOT_TClientServer
#define ROOT_TClientServer

#include "TMessage.h"
#include "TString.h"
#include "TMonitor.h"
#include "TJob.h"

struct TNote : public TObject {
	ClassDef(TNote, 1);
	TNote();
	unsigned code;
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
	void Broadcast(unsigned code, TString msg = "") const;
	void CollectOne(TSocket* = nullptr);
	void CollectAll();
	void SetJob(TJob*);
	void SetMacroPath(const TString&);
	inline TString GetMacroPath() const { return fMacroPath; }
	//void Reset() TODO kill all servers, delete list of results, clean fMonitor, reset fServerN, remove fJob
	
	TList ResList;

	private:
	void Run();
	void Send(unsigned code, const TString& msg = "", TSocket * = nullptr) const;
	void Send(const TMessage&, TSocket * = nullptr) const;
	void ClientHandleInput(TMessage*&, TSocket*);
	void ServerHandleInput(TMessage*&);
	
	bool fIsParent;
	unsigned fPortN;
	unsigned fServerN;
	TMonitor fMon;
	TJob *fJob;
	TString fMacroPath;

};

#endif
