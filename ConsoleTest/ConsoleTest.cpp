#include <iostream>
#include <filesystem>


using namespace std;
namespace fs = std::filesystem;
using std::cout;
using std::endl;




int main(int argc,char** argv)
{	
    int result = 0;
	fs::path src("H:/Project_VS2019/NutEngine/Engine/Asset/diffuse.png");
	fs::path dst("H:/Project_VS2019/NutEngine/Engine/Asset/Img/diffuse.png");
	
	auto ret = fs::exists(dst.parent_path());
	fs::copy_file(src,dst);
	return 0;
}

