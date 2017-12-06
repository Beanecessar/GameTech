//#include "Subspace.h"
//
//Subspace::Subspace(const float&size, const Vector3& centre, const unsigned depth, Subspace* const parent) {
//	this->size = size;
//	this->centre = centre;
//	this->parent = parent;
//	this->depth = depth;
//
//	if (depth<maxDepth)
//	{
//		flu = new Subspace(size / 2, centre + Vector3(-size / 2, size / 2, -size / 2), depth + 1, this);
//		fld = new Subspace(size / 2, centre + Vector3(-size / 2, -size / 2, -size / 2), depth + 1, this);
//		fru = new Subspace(size / 2, centre + Vector3(size / 2, size / 2, -size / 2), depth + 1, this);
//		frd = new Subspace(size / 2, centre + Vector3(size / 2, -size / 2, -size / 2), depth + 1, this);
//		blu = new Subspace(size / 2, centre + Vector3(-size / 2, size / 2, size / 2), depth + 1, this);
//		bld = new Subspace(size / 2, centre + Vector3(-size / 2, -size / 2, size / 2), depth + 1, this);
//		bru = new Subspace(size / 2, centre + Vector3(size / 2, size / 2, size / 2), depth + 1, this);
//		brd = new Subspace(size / 2, centre + Vector3(size / 2, -size / 2, size / 2), depth + 1, this);
//	}
//	else
//	{
//		flu = nullptr;
//		fld = nullptr;
//		fru = nullptr;
//		frd = nullptr;
//		blu = nullptr;
//		bld = nullptr;
//		bru = nullptr;
//		brd = nullptr;
//	}
//}
//
//Subspace::Subspace(const float& size, const unsigned& maxdepth) {
//	root = this;
//	this->size = size;
//	centre = Vector3(0, 0, 0);
//	parent = nullptr;
//	maxDepth = maxdepth;
//
//	depth = 0;
//	
//	if (depth < maxDepth)
//	{
//		flu = new Subspace(size / 2, centre + Vector3(-size / 2, size / 2, -size / 2), depth + 1, this);
//		fld = new Subspace(size / 2, centre + Vector3(-size / 2, -size / 2, -size / 2), depth + 1, this);
//		fru = new Subspace(size / 2, centre + Vector3(size / 2, size / 2, -size / 2), depth + 1, this);
//		frd = new Subspace(size / 2, centre + Vector3(size / 2, -size / 2, -size / 2), depth + 1, this);
//		blu = new Subspace(size / 2, centre + Vector3(-size / 2, size / 2, size / 2), depth + 1, this);
//		bld = new Subspace(size / 2, centre + Vector3(-size / 2, -size / 2, size / 2), depth + 1, this);
//		bru = new Subspace(size / 2, centre + Vector3(size / 2, size / 2, size / 2), depth + 1, this);
//		brd = new Subspace(size / 2, centre + Vector3(size / 2, -size / 2, size / 2), depth + 1, this);
//	}
//	else
//	{
//		flu = nullptr;
//		fld = nullptr;
//		fru = nullptr;
//		frd = nullptr;
//		blu = nullptr;
//		bld = nullptr;
//		bru = nullptr;
//		brd = nullptr;
//	}
//}
//
//Subspace::~Subspace() 
//{
//	delete flu;
//	delete fld;
//	delete fru;
//	delete frd;
//	delete blu;
//	delete bld;
//	delete bru;
//	delete brd;
//}
//
//void Subspace::AddNode(PhysicsNode* const data) {
//	AddNode(data, root);
//}
//
//
//bool Subspace::IsObjectSpaceCollision(PhysicsNode* const data, Subspace* const ptr) {
//	Vector3 v_min, v_max;
//
//	//x axis
//	data->GetCollisionShape()->GetMinMaxVertexOnAxis(Vector3(1, 0, 0), v_min, v_max);
//	if (v_min.x<ptr->centre.x&&v_max.x>ptr->centre.x)
//	{
//		return true;
//	}
//	
//	//y axis
//	data->GetCollisionShape()->GetMinMaxVertexOnAxis(Vector3(0, 1, 0), v_min, v_max);
//	if (v_min.y<ptr->centre.y&&v_max.y>ptr->centre.y)
//	{
//		return true;
//	}
//
//	//z axis
//	data->GetCollisionShape()->GetMinMaxVertexOnAxis(Vector3(0, 0, 1), v_min, v_max);
//	if (v_min.z<ptr->centre.z&&v_max.z>ptr->centre.z)
//	{
//		return true;
//	}
//
//	return false;
//}
//
//bool Subspace::IsObjectOutOfSpace(PhysicsNode* const data) {
//	Vector3 v_min, v_max;
//
//	//x axis
//	data->GetCollisionShape()->GetMinMaxVertexOnAxis(Vector3(1, 0, 0), v_min, v_max);
//	if (v_min.x < data->GetSubspace()->centre.x - size && v_max.x > data->GetSubspace()->centre.x + size)
//	{
//		return true;
//	}
//
//	//y axis
//	data->GetCollisionShape()->GetMinMaxVertexOnAxis(Vector3(0, 1, 0), v_min, v_max);
//	if (v_min.y < data->GetSubspace()->centre.y - size && v_max.y > data->GetSubspace()->centre.y + size)
//	{
//		return true;
//	}
//
//	//z axis
//	data->GetCollisionShape()->GetMinMaxVertexOnAxis(Vector3(0, 0, 1), v_min, v_max);
//	if (v_min.z < data->GetSubspace()->centre.z - size && v_max.z > data->GetSubspace()->centre.z + size)
//	{
//		return true;
//	}
//
//	return false;
//}
//
//
//void Subspace::AddNode(PhysicsNode* const data, Subspace* const ptr) {
//	if (IsObjectSpaceCollision(data,ptr)||ptr->depth>=maxDepth)
//	{
//		data->SetSubspace(ptr);
//		ptr->nodes.push_back(data);
//		return;
//	}
//
//	Vector3 pos = data->GetPosition();
//	if (pos.z < ptr->centre.z) {
//		//forward
//
//		if (pos.x<ptr->centre.x){
//			//left
//			
//			if (pos.y>ptr->centre.y){
//				//up
//				AddNode(data, ptr->flu);
//			}
//			else {
//				//down
//				AddNode(data, ptr->fld);
//			}
//		}
//		else {
//			//right
//
//			if (pos.y > ptr->centre.y) {
//				//up
//				AddNode(data, ptr->fru);
//			}
//			else {
//				//down
//				AddNode(data, ptr->frd);
//			}
//		}
//	}
//	else {
//		//backward
//
//		if (pos.x < ptr->centre.x) {
//			//left
//
//			if (pos.y > ptr->centre.y) {
//				//up
//				AddNode(data, ptr->blu);
//			}
//			else {
//				//down
//				AddNode(data, ptr->bld);
//			}
//		}
//		else {
//			//right
//
//			if (pos.y > ptr->centre.y) {
//				//up
//				AddNode(data, ptr->bru);
//			}
//			else {
//				//down
//				AddNode(data, ptr->brd);
//			}
//		}
//	}
//
//	return;
//}
//
//void Subspace::RemoveNode(PhysicsNode* const data) {
//	for (auto i = data->GetSubspace()->nodes.begin();i!= data->GetSubspace()->nodes.end();i++)
//	{
//		if (*i == data) {
//			data->GetSubspace()->nodes.erase(i);
//		}
//	}
//}
//
//void Subspace::UpdateNode(PhysicsNode* const data) {
//	if (!IsObjectOutOfSpace(data)) {
//		return;
//	}
//
//	RemoveNode(data);
//	AddNode(data, root);
//}