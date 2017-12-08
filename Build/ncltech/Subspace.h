#pragma once
#include "nclgl/Vector3.h"
#include "PhysicsNode.h"
#include "CollisionPair.h"
#include <vector>

class Subspace
{
public:
	Subspace(const Vector3& centre, const float& size, const unsigned& maxdepth);
	Subspace(const float&size, const Vector3& centre, const unsigned depth, Subspace* const parent, const float& maxDepth, Subspace* const root);
	~Subspace();

	void DrawDebugFrame();

	void AddNode		(PhysicsNode* const data);
	void RemoveNode		(PhysicsNode* const data);
	void Clear();

	void UpdateNode		(PhysicsNode* const data);

	void GetCollisionPairs	(std::vector<CollisionPair>& collisionpairs);

	//Set
	void SetSize	(const float& v)	{ size = v; }
	void SetCentre	(const Vector3& v)	{ centre = v; }
	void SetParent	(Subspace* o)		{ parent = o; }

	//Get
	const float GetSize()		{ return size; }
	const Vector3 GetCentre()	{ return centre; }
	const Subspace* GetParent()	{ return parent; }

protected:
	void DrawDebugFrame(Subspace* const ptr);

	bool IsObjectSpaceCollision(PhysicsNode* const data, Subspace* const ptr);
	bool IsObjectOutOfSpace(PhysicsNode* const data);							//include the case that object is on the bound

	void AddNode(PhysicsNode* const data, Subspace* const ptr);
	void Clear(Subspace* ptr);

	bool FastCollisionDetection(PhysicsNode* const pnodeA, PhysicsNode* const pnodeB);
	void GetCollisionPairs(std::vector<CollisionPair>& collisionpairs,Subspace* const ptr, const std::vector<PhysicsNode*>& parentNodes);

	float maxDepth;
	unsigned depth;

	Subspace* root;

	Vector3 centre;
	unsigned size;

	std::vector<PhysicsNode*> nodes;

	Subspace* parent;

	Subspace* flu; //forward, left , up node
	Subspace* fld;
	Subspace* fru;
	Subspace* frd;
	Subspace* blu;
	Subspace* bld;
	Subspace* bru;
	Subspace* brd; //backward, right, down node
};