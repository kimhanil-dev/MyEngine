#include "pch.h"
#include "fbx.h"

#include <fbxsdk/fileio/fbxiosettings.h>

#include "console.h"


// weak? shared?
std::map<uint, shared_ptr<Mesh>> gMeshes;

template <typename FbxObject>
class Releaser
{
public:
	Releaser(FbxObject* fbxObject) : mFbxObject(fbxObject) {
		_ASSERT(fbxObject);
	}
	~Releaser()
	{
		if (mFbxObject)
		{
			mFbxObject->Destroy();
		}
	}

	FbxObject* Get()
	{
		_ASSERT(mFbxObject);
		return mFbxObject;
	}

	FbxObject* operator->()
	{
		_ASSERT(mFbxObject);
		return mFbxObject;
	}

	bool operator==(const FbxObject* obj)
	{
		return mFbxObject == obj;
	}

private:
	FbxObject* mFbxObject = nullptr;
};

shared_ptr<Mesh> fbx::GetMesh(uint id)
{
	auto result = gMeshes.find(id);
	return result != gMeshes.end() ? result->second : nullptr;
}

void fbx::LoadFBX(const char* path, uint id)
{
	// ID is already using
	if (GetMesh(id) != nullptr)
		return;

	Releaser sdkManager = FbxManager::Create();

	Releaser ios = FbxIOSettings::Create(sdkManager.Get(), IOSROOT);
	sdkManager->SetIOSettings(ios.Get());

	Releaser importer = FbxImporter::Create(sdkManager.Get(), "");

	bool result = importer->Initialize(path, -1, sdkManager->GetIOSettings());
	if (!result)
	{
		// TODO : add error 
		PrintError("Fbx Importer initalizing failed(Path : %s) : %s", path, importer->GetStatus().GetErrorString());
		return;
	}
	else
	{
		Releaser scene = FbxScene::Create(sdkManager.Get(), "loadedScene");
		if (!importer->Import(scene.Get()))
		{
			// TODO : add error message
			PrintError("scene import failed : %s", importer->GetStatus().GetErrorString());
			return;
		}


		Releaser rootNode = scene->GetRootNode();
		if (rootNode == nullptr)
		{
			PrintError("Fbx file isn't have node, check your fbx file(%s)", path);
			return;
		}

		Releaser fbxMesh = rootNode->GetChild(0)->GetMesh();
		if (fbxMesh == nullptr)
		{
			PrintError("Your fbx file isn't have `Mesh`. Please check your fbx file(%s)", path);
			return;
		}


		// fbx에 저장된 fbxMesh의 구조는 항상 Triangle의 형태를 보장하지 않습니다. (삼각형, 사각형, 등 다양할 수 있다)
		// 따라서 삼각화를 별도로 진행해줍니다.
		FbxGeometryConverter geometryConverter = FbxGeometryConverter(sdkManager.Get());
		geometryConverter.Triangulate(rootNode->GetChild(0)->GetNodeAttribute(), false, true);

		// fbx sdk에 대하여...
		// ControlPoint는 Vertex와 대응된다.
		// PolygonVertex는 Index와 대응된다.


		// Store data in a mesh instance
		shared_ptr<Mesh> mesh = make_shared<Mesh>();

		// vertices
		{
			const FbxVector4* vertices = fbxMesh->GetControlPoints();
			const int vertexCount = fbxMesh->GetControlPointsCount();

			mesh->Vertices.resize(vertexCount);
			for (int i = 0; i < vertexCount; ++i)
			{
				mesh->Vertices[i].Position = XMFLOAT3(
					(float)vertices[i].mData[0],
					(float)vertices[i].mData[1],
					(float)vertices[i].mData[2]);
			}
		}

		// normals
		//{
		//	FbxArray<FbxVector4> normals;
		//	fbxMesh->GetPolygonVertexNormals(normals);
		//}

		// indices
		{
			const int* indices = fbxMesh->GetPolygonVertices();
			const int indexCount = fbxMesh->GetPolygonVertexCount();

			mesh->Indices.resize(indexCount);
			memcpy_s(mesh->Indices.data(), mesh->Indices.size() * sizeof(int), indices, indexCount * sizeof(int));
		}

		gMeshes.emplace(id, mesh);

		PrintGood("Mesh(ID:%d) Loadded (Vertex : %d), (Index : %d)", id, mesh->Vertices.size(), mesh->Indices.size());
	}
}

void fbx::Release()
{
	gMeshes.clear();
}
