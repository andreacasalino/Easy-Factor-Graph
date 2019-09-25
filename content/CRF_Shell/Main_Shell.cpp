#include "__Header/Shell_Clients.h"
#include "__Header/Client_terminal.h"
#include "__Header/Client_socket.h"
using namespace std;
using namespace Segugio;


// When a second input is specified, additional remote client (socket) are launched.
// The syntax of the second argument must be C$a$address_1$p$port_1$a$address_2$p$port_2$a$address_2$p$port_2 
// use address = "_" for localhost
int main(int argc, char *argv[]) {

	CRF_Shell shell;
	list<CRF_Shell::Client*> Clients;

	Clients.push_back(new Client_Terminal(&shell));
	if (argc > 1) {
		//launch remote clients
		Command C(argv[1]);

		if (C.Get_name() != '-') {
			//read the addresses
			auto addresses = C.Get_values('a');
			auto			  ports = C.Get_values('p');

			if ((addresses != NULL) && (ports != NULL)) {
				if (addresses->size() == ports->size()) {
					auto it_p = ports->begin();
					for (auto it_a = addresses->begin(); it_a != addresses->end(); it_a++) {
						if (it_a->compare("_") == 0)
							Clients.push_back(new Client_Socket(&shell, "", *it_p));
						else
							Clients.push_back(new Client_Socket(&shell, *it_a, *it_p));
						it_p++;
					}
				}
			}
		}
		else system("ECHO invalid argv syntax: no remote clients were launched");
	}

	shell.Activate_loop();

	for (auto it = Clients.begin(); it != Clients.end(); it++)
		delete *it;

	return 0;
}