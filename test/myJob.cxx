#include "myJob.h"
#include "TH1F.h"

TObject* myJob::Process() {
	TH1F* h = new TH1F("h1","h",100,-3,3);
	h->FillRandom("gaus",10000);
	return h;
}

/*
TObject* myJob::Merge(const TList&) {
	TObject* o = nullptr;
	return o;
}*/
