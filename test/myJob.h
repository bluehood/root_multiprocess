#ifndef myjob
#define myjob

#include "../TJob.h"

class myJob : public TJob {
	TObject* Process();
	//TObject* Merge(const TList&);
};

#endif
