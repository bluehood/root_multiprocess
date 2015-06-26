#include "TClientServer.h"
#include "TSystem.h"
#include "TSocket.h"
#include <iostream>

int main() {
	TClientServer cs;
	cs.Fork(); // child does not return
	cs.Send("Hello child");
	std::cout << "C: message received from server: '" << cs.Recv() << "'" << std::endl;
	std::cout << "start waiting..." << std::endl;
	gSystem->Sleep(600000);
	std::cout << "stop waiting..." << std::endl;
}
