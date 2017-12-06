#pragma once
#include "nclgl/Vector3.h"
#include "PhysicsNode.h"
#include <vector>

class Octree
{
	Octree();
	Octree(const Vector3& size);
	~Octree();

public:
	const Octree* AddNode(const PhysicsNode* data);
	void UpdateNode(const Octree* node);

	void TraversalNodes(void* functionCall);

	//Set
	void SetSize	(const Vector3& v)	{ size = v; }
	void SetCentre	(const Vector3& v)	{ centre = v; }
	void SetParent	(Octree* o)			{ parent = o; }

	//Get
	const Vector3 GetSize()		{ return size; }
	const Vector3 GetCentre()	{ return centre; }
	const Octree* GetParent()	{ return parent; }

protected:
	const Octree* AddNode(const PhysicsNode* data, Octree* const ptr);


	static unsigned maxDepth;
	unsigned depth;

	Octree* root;

	Vector3 centre;
	Vector3 size;

	std::vector<PhysicsNode*> nodes;

	Octree* parent;

	Octree* flu; //forward, left , up node
	Octree* fld;
	Octree* fru;
	Octree* frd;
	Octree* blu;
	Octree* bld;
	Octree* bru;
	Octree* brd; //backward, right, down node
};