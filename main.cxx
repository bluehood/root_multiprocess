#include "TClientServer.h"
#include "TSystem.h"
#include "TSocket.h"
#include <iostream>

int main() {
	TClientServer cs;
	cs.Fork(); // child does not return
	gSystem->Sleep(600000);
}
