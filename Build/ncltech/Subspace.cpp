#include "Subspace.h"
#include "nclgl/NCLDebug.h"

Subspace::Subspace(const float&size, const Vector3& centre, const unsigned depth, Subspace* const parent, const float& maxDepth, Subspace* const root) {
	this->size = size;
	this->centre = centre;
	this->parent = parent;
	this->depth = depth;
	this->root = root;
	this->maxDepth = maxDepth;

	if (depth<maxDepth)
	{
		flu = new Subspace(size / 2, centre + Vector3(-size / 2, size / 2, -size / 2), depth + 1, this, maxDepth, root);
		fld = new Subspace(size / 2, centre + Vector3(-size / 2, -size / 2, -size / 2), depth + 1, this, maxDepth, root);
		fru = new Subspace(size / 2, centre + Vector3(size / 2, size / 2, -size / 2), depth + 1, this, maxDepth, root);
		frd = new Subspace(size / 2, centre + Vector3(size / 2, -size / 2, -size / 2), depth + 1, this, maxDepth, root);
		blu = new Subspace(size / 2, centre + Vector3(-size / 2, size / 2, size / 2), depth + 1, this, maxDepth, root);
		bld = new Subspace(size / 2, centre + Vector3(-size / 2, -size / 2, size / 2), depth + 1, this, maxDepth, root);
		bru = new Subspace(size / 2, centre + Vector3(size / 2, size / 2, size / 2), depth + 1, this, maxDepth, root);
		brd = new Subspace(size / 2, centre + Vector3(size / 2, -size / 2, size / 2), depth + 1, this, maxDepth, root);
	}
	else
	{
		flu = nullptr;
		fld = nullptr;
		fru = nullptr;
		frd = nullptr;
		blu = nullptr;
		bld = nullptr;
		bru = nullptr;
		brd = nullptr;
	}
}

Subspace::Subspace(const Vector3& centre, const float& size, const unsigned& maxdepth) {
	root = this;
	this->size = size;
	this->centre = centre;
	parent = nullptr;
	maxDepth = maxdepth;

	depth = 0;
	
	if (depth < maxDepth)
	{
		flu = new Subspace(size / 2, centre + Vector3(-size / 2, size / 2, -size / 2), depth + 1, this, maxDepth, root);
		fld = new Subspace(size / 2, centre + Vector3(-size / 2, -size / 2, -size / 2), depth + 1, this, maxDepth, root);
		fru = new Subspace(size / 2, centre + Vector3(size / 2, size / 2, -size / 2), depth + 1, this, maxDepth, root);
		frd = new Subspace(size / 2, centre + Vector3(size / 2, -size / 2, -size / 2), depth + 1, this, maxDepth, root);
		blu = new Subspace(size / 2, centre + Vector3(-size / 2, size / 2, size / 2), depth + 1, this, maxDepth, root);
		bld = new Subspace(size / 2, centre + Vector3(-size / 2, -size / 2, size / 2), depth + 1, this, maxDepth, root);
		bru = new Subspace(size / 2, centre + Vector3(size / 2, size / 2, size / 2), depth + 1, this, maxDepth, root);
		brd = new Subspace(size / 2, centre + Vector3(size / 2, -size / 2, size / 2), depth + 1, this, maxDepth, root);
	}
	else
	{
		flu = nullptr;
		fld = nullptr;
		fru = nullptr;
		frd = nullptr;
		blu = nullptr;
		bld = nullptr;
		bru = nullptr;
		brd = nullptr;
	}
}

Subspace::~Subspace() 
{
	delete flu;
	delete fld;
	delete fru;
	delete frd;
	delete blu;
	delete bld;
	delete bru;
	delete brd;
}

void Subspace::DrawDebugFrame() {
	DrawDebugFrame(root);
}

void Subspace::DrawDebugFrame(Subspace* const ptr) {
	//centre
	NCLDebug::DrawPointNDT(ptr->centre, 0.05f, Vector4((float)ptr->depth / ptr->maxDepth, (float)ptr->depth / (float)ptr->maxDepth, ptr->depth / ptr->maxDepth, 1.0f));

	//x axis
	NCLDebug::DrawThickLineNDT(	Vector3(ptr->centre.x - ptr->size, ptr->centre.y - ptr->size, ptr->centre.z - ptr->size),
								Vector3(ptr->centre.x + ptr->size, ptr->centre.y - ptr->size, ptr->centre.z - ptr->size),
								0.01f,
								Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	NCLDebug::DrawThickLineNDT(	Vector3(ptr->centre.x - ptr->size, ptr->centre.y - ptr->size, ptr->centre.z + ptr->size),
								Vector3(ptr->centre.x + ptr->size, ptr->centre.y - ptr->size, ptr->centre.z + ptr->size),
								0.01f,
								Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	NCLDebug::DrawThickLineNDT(	Vector3(ptr->centre.x - ptr->size, ptr->centre.y + ptr->size, ptr->centre.z - ptr->size),
								Vector3(ptr->centre.x + ptr->size, ptr->centre.y + ptr->size, ptr->centre.z - ptr->size),
								0.01f,
								Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	NCLDebug::DrawThickLineNDT(	Vector3(ptr->centre.x - ptr->size, ptr->centre.y + ptr->size, ptr->centre.z + ptr->size),
								Vector3(ptr->centre.x + ptr->size, ptr->centre.y + ptr->size, ptr->centre.z + ptr->size),
								0.01f,
								Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	//y axis
	NCLDebug::DrawThickLineNDT(	Vector3(ptr->centre.x - ptr->size, ptr->centre.y - ptr->size, ptr->centre.z - ptr->size),
								Vector3(ptr->centre.x - ptr->size, ptr->centre.y + ptr->size, ptr->centre.z - ptr->size),
								0.01f,
								Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	NCLDebug::DrawThickLineNDT(	Vector3(ptr->centre.x - ptr->size, ptr->centre.y - ptr->size, ptr->centre.z + ptr->size),
								Vector3(ptr->centre.x - ptr->size, ptr->centre.y + ptr->size, ptr->centre.z + ptr->size),
								0.01f,
								Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	NCLDebug::DrawThickLineNDT(	Vector3(ptr->centre.x + ptr->size, ptr->centre.y - ptr->size, ptr->centre.z - ptr->size),
								Vector3(ptr->centre.x + ptr->size, ptr->centre.y + ptr->size, ptr->centre.z - ptr->size),
								0.01f,
								Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	NCLDebug::DrawThickLineNDT(	Vector3(ptr->centre.x + ptr->size, ptr->centre.y - ptr->size, ptr->centre.z + ptr->size),
								Vector3(ptr->centre.x + ptr->size, ptr->centre.y + ptr->size, ptr->centre.z + ptr->size),
								0.01f,
								Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	//z axis
	NCLDebug::DrawThickLineNDT(	Vector3(ptr->centre.x - ptr->size, ptr->centre.y - ptr->size, ptr->centre.z - ptr->size),
								Vector3(ptr->centre.x - ptr->size, ptr->centre.y - ptr->size, ptr->centre.z + ptr->size),
								0.01f,
								Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	NCLDebug::DrawThickLineNDT(	Vector3(ptr->centre.x - ptr->size, ptr->centre.y + ptr->size, ptr->centre.z - ptr->size),
								Vector3(ptr->centre.x - ptr->size, ptr->centre.y + ptr->size, ptr->centre.z + ptr->size),
								0.01f,
								Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	NCLDebug::DrawThickLineNDT(	Vector3(ptr->centre.x + ptr->size, ptr->centre.y - ptr->size, ptr->centre.z - ptr->size),
								Vector3(ptr->centre.x + ptr->size, ptr->centre.y - ptr->size, ptr->centre.z + ptr->size),
								0.01f,
								Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	NCLDebug::DrawThickLineNDT(	Vector3(ptr->centre.x + ptr->size, ptr->centre.y + ptr->size, ptr->centre.z - ptr->size),
								Vector3(ptr->centre.x + ptr->size, ptr->centre.y + ptr->size, ptr->centre.z + ptr->size),
								0.01f,
								Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	if (ptr->depth < ptr->maxDepth)
	{
		DrawDebugFrame(ptr->flu);
		DrawDebugFrame(ptr->fld);
		DrawDebugFrame(ptr->fru);
		DrawDebugFrame(ptr->frd);
		DrawDebugFrame(ptr->blu);
		DrawDebugFrame(ptr->bld);
		DrawDebugFrame(ptr->bru);
		DrawDebugFrame(ptr->brd);
	}
}

bool Subspace::IsObjectSpaceCollision(PhysicsNode* const data, Subspace* const ptr) {
	if (!data->GetCollisionShape())
	{
		return false;
	}

	Vector3 v_min, v_max;

	//x axis
	data->GetCollisionShape()->GetMinMaxVertexOnAxis(Vector3(1, 0, 0), v_min, v_max);
	if (v_min.x<ptr->centre.x&&v_max.x>ptr->centre.x)
	{
		return true;
	}
	
	//y axis
	data->GetCollisionShape()->GetMinMaxVertexOnAxis(Vector3(0, 1, 0), v_min, v_max);
	if (v_min.y<ptr->centre.y&&v_max.y>ptr->centre.y)
	{
		return true;
	}

	//z axis
	data->GetCollisionShape()->GetMinMaxVertexOnAxis(Vector3(0, 0, 1), v_min, v_max);
	if (v_min.z<ptr->centre.z&&v_max.z>ptr->centre.z)
	{
		return true;
	}

	return false;
}

bool Subspace::IsObjectOutOfSpace(PhysicsNode* const data) {
	if (!data->GetCollisionShape())
	{
		return false;
	}

	Vector3 v_min, v_max;

	//x axis
	data->GetCollisionShape()->GetMinMaxVertexOnAxis(Vector3(1, 0, 0), v_min, v_max);
	if (v_min.x < data->GetSubspace()->centre.x - size || v_max.x > data->GetSubspace()->centre.x + size)
	{
		return true;
	}

	//y axis
	data->GetCollisionShape()->GetMinMaxVertexOnAxis(Vector3(0, 1, 0), v_min, v_max);
	if (v_min.y < data->GetSubspace()->centre.y - size || v_max.y > data->GetSubspace()->centre.y + size)
	{
		return true;
	}

	//z axis
	data->GetCollisionShape()->GetMinMaxVertexOnAxis(Vector3(0, 0, 1), v_min, v_max);
	if (v_min.z < data->GetSubspace()->centre.z - size || v_max.z > data->GetSubspace()->centre.z + size)
	{
		return true;
	}

	return false;
}

void Subspace::AddNode(PhysicsNode* const data) {
	AddNode(data, root);
}


void Subspace::AddNode(PhysicsNode* const data, Subspace* const ptr) {
	if (IsObjectSpaceCollision(data,ptr)||ptr->depth>=maxDepth)
	{
		data->SetSubspace(ptr);
		ptr->nodes.push_back(data);
		return;
	}

	Vector3 pos = data->GetPosition();
	if (pos.z < ptr->centre.z) {
		//forward

		if (pos.x<ptr->centre.x){
			//left
			
			if (pos.y>ptr->centre.y){
				//up
				AddNode(data, ptr->flu);
			}
			else {
				//down
				AddNode(data, ptr->fld);
			}
		}
		else {
			//right

			if (pos.y > ptr->centre.y) {
				//up
				AddNode(data, ptr->fru);
			}
			else {
				//down
				AddNode(data, ptr->frd);
			}
		}
	}
	else {
		//backward

		if (pos.x < ptr->centre.x) {
			//left

			if (pos.y > ptr->centre.y) {
				//up
				AddNode(data, ptr->blu);
			}
			else {
				//down
				AddNode(data, ptr->bld);
			}
		}
		else {
			//right

			if (pos.y > ptr->centre.y) {
				//up
				AddNode(data, ptr->bru);
			}
			else {
				//down
				AddNode(data, ptr->brd);
			}
		}
	}

	return;
}

void Subspace::RemoveNode(PhysicsNode* const data) {
	if (data->GetSubspace()->nodes.empty())
	{
		return;
	}

	for (auto i = data->GetSubspace()->nodes.begin(); i != data->GetSubspace()->nodes.end(); i++)
	{
		if (*i == data) {
			data->GetSubspace()->nodes.erase(i);
			return;
		}
	}

	NCLERROR("Fail to remove from subspace!");
}

void Subspace::Clear() {
	Clear(root);
}

void Subspace::Clear(Subspace* ptr) {
	ptr->nodes.clear();

	if (ptr->depth < ptr->maxDepth)
	{
		Clear(ptr->flu);
		Clear(ptr->fld);
		Clear(ptr->fru);
		Clear(ptr->frd);
		Clear(ptr->blu);
		Clear(ptr->bld);
		Clear(ptr->bru);
		Clear(ptr->brd);
	}
}


void Subspace::UpdateNode(PhysicsNode* const data) {
	if (IsObjectOutOfSpace(data) || (data->GetSubspace()->depth < data->GetSubspace()->maxDepth && !IsObjectSpaceCollision(data, data->GetSubspace())))
	{
		RemoveNode(data);
		AddNode(data, root);
	}
}

bool Subspace::FastCollisionDetection(PhysicsNode* const pnodeA, PhysicsNode* const pnodeB) {
	//x axis
	if (pnodeA->GetPosition().x + pnodeA->getAABB().x<pnodeB->GetPosition().x - pnodeB->getAABB().x ||
		pnodeA->GetPosition().x - pnodeA->getAABB().x>pnodeB->GetPosition().x + pnodeB->getAABB().x)
		return false;

	//y axis
	if (pnodeA->GetPosition().y + pnodeA->getAABB().y<pnodeB->GetPosition().y - pnodeB->getAABB().y ||
		pnodeA->GetPosition().y - pnodeA->getAABB().y>pnodeB->GetPosition().y + pnodeB->getAABB().y)
		return false;

	//z axis
	if (pnodeA->GetPosition().z + pnodeA->getAABB().z<pnodeB->GetPosition().z - pnodeB->getAABB().z ||
		pnodeA->GetPosition().z - pnodeA->getAABB().z>pnodeB->GetPosition().z + pnodeB->getAABB().z)
		return false;

	return true;
}

void Subspace::GetCollisionPairs(std::vector<CollisionPair>& collisionpairs) {
	std::vector<PhysicsNode*> emptyVector;
	emptyVector.clear();
	GetCollisionPairs(collisionpairs, root, emptyVector);
}

void Subspace::GetCollisionPairs(std::vector<CollisionPair>& collisionpairs, Subspace* const ptr, const std::vector<PhysicsNode*>& parentNodes) {
	std::vector<PhysicsNode*> parentnodes = parentNodes;


	if (ptr->nodes.size() > 0)
	{
		PhysicsNode *pnodeA, *pnodeB;
		for (size_t i = 0; i < ptr->nodes.size() - 1; ++i)
		{
			for (size_t j = i + 1; j < ptr->nodes.size(); ++j)
			{
				pnodeA = ptr->nodes[i];
				pnodeB = ptr->nodes[j];

				if (FastCollisionDetection(pnodeA, pnodeB))
				{
					//Check they both at least have collision shapes
					if (pnodeA->GetCollisionShape() != NULL
						&& pnodeB->GetCollisionShape() != NULL)
					{
						CollisionPair cp;
						cp.pObjectA = pnodeA;
						cp.pObjectB = pnodeB;
						collisionpairs.push_back(cp);
					}
				}
			}
		}

		if (parentNodes.size() > 0)
		{
			PhysicsNode *pnodeA, *pnodeB;
			for (size_t i = 0; i < parentNodes.size(); ++i)
			{
				for (size_t j = 0; j < ptr->nodes.size(); ++j)
				{
					pnodeA = parentNodes[i];
					pnodeB = ptr->nodes[j];

					if (FastCollisionDetection(pnodeA, pnodeB))
					{
						//Check they both at least have collision shapes
						if (pnodeA->GetCollisionShape() != NULL
							&& pnodeB->GetCollisionShape() != NULL)
						{
							CollisionPair cp;
							cp.pObjectA = pnodeA;
							cp.pObjectB = pnodeB;
							collisionpairs.push_back(cp);
						}
					}
				}
			}
		}

		parentnodes.insert(parentnodes.end(), ptr->nodes.begin(), ptr->nodes.end());
	}
	

	if (ptr->depth < ptr->maxDepth)
	{
		GetCollisionPairs(collisionpairs, ptr->flu, parentnodes);
		GetCollisionPairs(collisionpairs, ptr->fld, parentnodes);
		GetCollisionPairs(collisionpairs, ptr->fru, parentnodes);
		GetCollisionPairs(collisionpairs, ptr->frd, parentnodes);
		GetCollisionPairs(collisionpairs, ptr->blu, parentnodes);
		GetCollisionPairs(collisionpairs, ptr->bld, parentnodes);
		GetCollisionPairs(collisionpairs, ptr->bru, parentnodes);
		GetCollisionPairs(collisionpairs, ptr->brd, parentnodes);
	}
}