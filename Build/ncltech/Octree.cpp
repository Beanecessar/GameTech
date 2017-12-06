#include "Octree.h"

Octree::Octree() {
	root = nullptr;

	size = Vector3(0, 0, 0);
	centre = Vector3(0, 0, 0);

	parent = nullptr;
	flu = nullptr;
	fld = nullptr;
	fru = nullptr;
	frd = nullptr;
	blu = nullptr;
	bld = nullptr;
	bru = nullptr;
	brd = nullptr;
	maxDepth = 0;
}

Octree::Octree(const Vector3& size) {
	root = nullptr;

	this->size = size;
	centre = Vector3(0, 0, 0);
	
	parent = nullptr;
	flu = nullptr;
	fld = nullptr;
	fru = nullptr;
	frd = nullptr;
	blu = nullptr;
	bld = nullptr;
	bru = nullptr;
	brd = nullptr;
	maxDepth = 0;
}

Octree::~Octree() 
{
	delete root;

	for (auto i = nodes.begin(); i != nodes.end(); i++)
	{
		delete *i;
	}

	delete parent;

	delete flu;
	delete fld;
	delete fru;
	delete frd;
	delete blu;
	delete bld;
	delete bru;
	delete brd;
}

const Octree* Octree::AddNode(const PhysicsNode* data) {
	return AddNode(data, root);
}

const Octree* Octree::AddNode(const PhysicsNode* data, Octree* const ptr) {
	Octree* newNode = new Octree();
	//TODO
	return nullptr;
}