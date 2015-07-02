#include "TH1F.h"

TObject *myMacro()
{
   TH1F *h = new TH1F("h1", "h", 100, -3, 3);
   h->FillRandom("gaus", 1000);
   return h;
}
