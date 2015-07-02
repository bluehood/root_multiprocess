void setup() {
	gROOT->ProcessLine(".L TJob.cxx+");
	gROOT->ProcessLine(".L myJob.cxx+");
	gROOT->ProcessLine(".L TNote.cxx+");
	gROOT->ProcessLine(".L TClientServer.cxx+");
	gROOT->ProcessLine(".L myClientServer.cxx+");
}
