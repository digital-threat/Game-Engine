#pragma once

class MeshManager
{
	MeshManager();

public:
	static void Allocate();
	static MeshManager& Get();

private:
	static MeshManager* instance;
};
