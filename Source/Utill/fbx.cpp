#include "pch.h"
#include "fbx.h"

#include <fbxsdk/fileio/fbxiosettings.h>

#include "console.h"
#include "Core/Render/Mesh.h"

std::map<uint, Mesh*> gMeshes;

Mesh* fbx::GetMesh(uint id)
{
	auto result = gMeshes.find(id);
	return result != gMeshes.end() ? result->second : nullptr;
}

void fbx::LoadFBX(const char* path, uint id)
{
	// ID is already using
	if (GetMesh(id) != nullptr)
		return;

	FbxManager* sdkManager = FbxManager::Create();

	FbxIOSettings* ios = FbxIOSettings::Create(sdkManager, IOSROOT);
	sdkManager->SetIOSettings(ios);

	FbxImporter* importer = FbxImporter::Create(sdkManager, "");

	bool result = importer->Initialize(path, -1, sdkManager->GetIOSettings());
	if (!result)
	{
		// TODO : add error 
		PrintError("Fbx Importer initalizing failed(Path : %s) : %s", path, importer->GetStatus().GetErrorString());
		return;
	}
	else
	{
		FbxScene* scene = FbxScene::Create(sdkManager, "loadedScene");
		if (!importer->Import(scene))
		{
			// TODO : add error message
			PrintError("scene import failed : %s", importer->GetStatus().GetErrorString());
			return;
		}


		FbxNode* rootNode = scene->GetRootNode();
		if (rootNode == nullptr)
		{
			PrintError("Fbx file isn't have node, check your fbx file(%s)", path);
			return;
		}

		FbxGeometryConverter geometryConverter = FbxGeometryConverter(sdkManager);
		geometryConverter.Triangulate(rootNode->GetChild(0)->GetNodeAttribute(), false,true);

		FbxMesh* fbxMesh = rootNode->GetChild(0)->GetMesh();
		if (fbxMesh == nullptr)
		{
			PrintError("Your fbx file isn't have `Mesh`. Please check your fbx file(%s)", path);
			return;
		}


		

		// make Mesh instance
		Mesh* mesh = new Mesh(
			fbxMesh->GetControlPoints(),
			fbxMesh->GetControlPointsCount(),
			fbxMesh->GetPolygonVertices(),
			fbxMesh->GetPolygonVertexCount());

		gMeshes.emplace(id, mesh);

		PrintGood("Mesh(ID:%d) Loadded (Vertex : %d), (Index : %d)", id, mesh->Vertices.size(), mesh->Indices.size());

		scene->Destroy();
	}

	ios->Destroy();
	importer->Destroy();
	sdkManager->Destroy();
}

void fbx::Release()
{
	for (auto iter : gMeshes)
	{
		delete iter.second;
	}

	gMeshes.clear();
}
