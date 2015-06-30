#ifndef ROOT_TJob
#define ROOT_TJob

#include "TObject.h"
#include "TList.h"

class TJob {
	ClassDef(TJob,1);
	public:
	virtual TObject* Process()=0;
	//virtual TObject* Merge(const TList&)=0;
};

#endif
