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

void BuildView(XMVECTOR pos, XMVECTOR at, XMVECTOR up)
{
	XMVECTOR x,y,z;
	z = XMVector3Normalize(pos - at);
	x = XMVector3Normalize(XMVector3Cross(up, z));
	y = XMVector3Cross(z, x);
	
	Matrix4x4f tmp = { {{
		{ x.m128_f32[0], y.m128_f32[0], z.m128_f32[0], 0.0f },
		{ x.m128_f32[1], y.m128_f32[1], z.m128_f32[1], 0.0f },
		{ x.m128_f32[2], y.m128_f32[2], z.m128_f32[2], 0.0f },
		{ (-XMVector3Dot(x, pos)).m128_f32[0], (-XMVector3Dot(y, pos)).m128_f32[0], (-XMVector3Dot(z, pos)).m128_f32[0], 1.0f }
	}} };
}

int main(int argc,char** argv)
{	
	const DirectX::XMVECTOR lightPositionX = DirectX::XMVectorSet(0.f, 3.0f, -10.0f, 1.0f);
	const DirectX::XMVECTOR lightTargetX = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	const DirectX::XMVECTOR lightUpX = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	Matrix4x4f view{};
	BuildViewLHMatrix(view, Vector3f{ 0.f, 3.0f, -10.0f }, Vector3f{ 0.0f, 0.0f, 0.0f }, Vector3f{ 0.0f, 1.0f, 0.0f });
	Matrix4x4f projection{};
	auto view_mat = DirectX::XMMatrixLookAtLH(lightPositionX, lightTargetX, lightUpX);
	auto aspect = 16.f / 9.f;
	auto projection_view = DirectX::XMMatrixPerspectiveFovLH(0.8F, aspect, 1.f, 1000.f);
	BuildPerspectiveFovLHMatrix(projection, 0.8F, aspect, 1.f, 1000.f);
	auto m = view * projection;
	auto xm = view_mat * projection_view;
	return 0;
}
