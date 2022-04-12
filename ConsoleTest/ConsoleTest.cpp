// ConsoleTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <utility>
#include <DirectXMath.h>

#include "Framework/Math/NutMath.hpp"
//#include "fbxsdk.h"



using std::cout;
using std::endl;
using namespace DirectX;
using namespace Engine;
//using fbxsdk::FbxCast;



void func()
{
	//const char* file_name = "H:/Project_VS2019/NutEngine/Engine/Asset/box.fbx";
	// 
	// 
	//	// Initialize the SDK manager. This object handles memory management.
	//FbxManager* lSdkManager = FbxManager::Create();
	//// Create the IO settings object.
	//FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	//lSdkManager->SetIOSettings(ios);

	//// Create an importer using the SDK manager.
	//FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");
	//// Use the first argument as the filename for the importer.
	//if (!lImporter->Initialize(file_name, -1, lSdkManager->GetIOSettings()))
	//{
	//	printf("Call to FbxImporter::Initialize() failed.\n");
	//	printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
	//	exit(-1);
	//}
	//// Create a new scene so that it can be populated by the imported file.
	//FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");
	//lImporter->Import(lScene);
	//FbxNode* lRootNode = lScene->GetRootNode();
	//if (lRootNode) 
	//{
	//	for (int i = 0; i < lRootNode->GetChildCount(); i++) {
	//		auto* child = lRootNode->GetChild(i);
	//		//cout << "root's child :" << child->GetName() << endl;
	//		//int mat_count = child->GetMaterialCount();
	//		//if(mat_count == 0) {
	//		//	cout << child->GetName() << "haven't material" << endl; 
	//		//}
	//		//else {

	//		//}
	//		fbxsdk::FbxMesh* mesh = fbxsdk::FbxCast<fbxsdk::FbxMesh>(child->GetNodeAttribute());
	//		if (mesh != nullptr)
	//		{
	//			if(!mesh->IsTriangleMesh())
	//			{
	//				fbxsdk::FbxGeometryConverter convert(lSdkManager);
	//				mesh = fbxsdk::FbxCast<fbxsdk::FbxMesh>(convert.Triangulate(mesh, true));
	//				cout << mesh->GetName() << "have " << mesh->GetControlPointsCount() << "vertex" << endl;
	//				cout << mesh->GetName() << "have " << mesh->GetPolygonCount() << " face" << endl;
	//				cout << mesh->GetName() << "have " << mesh->GetPolygonVertexCount() << " index" << endl;
	//			}		
	//		}
	//		else
	//		{
	//			cout << "Cast Failed" << endl;
	//		}
	//		cout << "--------------------------------------" << endl;
	//		cout << endl;
	//	}		
	//}
	//// Import the contents of the file into the scene.
	//lImporter->Import(lScene);
	//// The file is imported, so get rid of the importer.
	//lImporter->Destroy();
	//ios->Destroy();
	//lSdkManager->Destroy();
}



int main(int argc,char** argv)
{	

	return 0;
}
