#include "../CRF/Header/Graphical_model.h"
#include "__Header/CRF_Shell.h"
using namespace std;

#ifdef _DEBUG
#pragma comment (lib, "../x64/Debug/CRF.lib")
#else
#pragma comment (lib, "../x64/Release/CRF.lib")
#endif // DEBUG
using namespace Segugio;


int main() {

	Shell_Debugger Shell_dbg;
	Shell_dbg.do_staff();

	system("pause");
	return 0;
}