#include "__Header/Shell_Clients.h"
#include "__Header/Client_terminal.h"
#include "__Header/Client_socket.h"
using namespace std;

int main() {

	CRF_Shell shell;
	Client_Terminal debug_terminal(&shell);
	shell.Activate_loop();


	//char next_instruction[1000];
	//while (true)
	//{
	//	fgets(&next_instruction[0], sizeof(next_instruction), stdin);
	//	string command(next_instruction);
	//	Command comm(command);
	//	comm.Print();
	//}


	system("pause");
	return 0;
}
