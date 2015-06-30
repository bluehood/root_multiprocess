void setup() {
	gROOT->ProcessLine(".L TJob.h+");
	gROOT->ProcessLine(".L myJob.cxx+");
	gROOT->ProcessLine(".L TClientServer.cxx+");
}
