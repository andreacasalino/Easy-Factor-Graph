#include "__Header/Shell_Clients.h"
#include "__Header/Client_terminal.h"
#include "__Header/Client_socket.h"
using namespace std;
using namespace Segugio;



int main() {

	CRF_Shell shell;
	Client_Terminal debug_terminal(&shell);
	shell.Activate_loop();

	system("pause");
	return 0;
}