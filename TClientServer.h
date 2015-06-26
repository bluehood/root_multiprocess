#include "TSocket.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include <string>

class TClientServer {
	public:
	TClientServer();
	~TClientServer();
	void Fork();
	void Send(std::string) const;
	std::string Recv();

	private:
	// to be used by child processes
	void Run();

	unsigned HandleInput(TMessage*);

	TString fSocketPath;
	bool fIsParent;
	TMonitor fMon;
	TSocket *fS;
};
