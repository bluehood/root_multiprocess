#ifndef _myClientServer_
#define _myClientServer_

#include "../TClientServer.h"
#include "../TJob.h"
#include "TString.h"

class myClientServer : public TClientServer {
	public:
	void ProcessJob(TJob*);
	void ProcessMacro(const TString& path);
};

#endif
