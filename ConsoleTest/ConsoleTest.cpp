#include "Framework/Common/Log.h"
#include "Framework/Common/ThreadManager.h"
#include "Framework/Common/Global.h"
#include <map>



using namespace std;
using namespace Engine;
using std::cout;
using std::endl;

namespace Engine
{
	LogManager* g_pLogManager = new LogManager();

}




int main(int argc,char** argv)
{	
	LoadConfigFile("H:/Project_VS2019/NutEngine/Engine/config.ini");
	g_pLogManager->Initialize();

	g_pLogManager->Finalize();
	return 0;
}

