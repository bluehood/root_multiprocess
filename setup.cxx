void setup()
{
   gROOT->ProcessLine(".L TJob.cxx+");
   gROOT->ProcessLine(".L TNote.cxx+");
   gROOT->ProcessLine(".L TServer.cxx+");
   gROOT->ProcessLine(".L TMultiProcess.cxx+");
   gROOT->ProcessLine(".L TPool.cxx+");
}
