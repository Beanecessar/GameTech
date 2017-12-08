#include "PhysicsNode.h"
#include "PhysicsEngine.h"


void PhysicsNode::IntegrateForVelocity(float dt)
{
	if (invMass > 0.0f)
		linVelocity += PhysicsEngine::Instance()->GetGravity() * dt;

	linVelocity += force * invMass * dt;

	linVelocity = linVelocity * PhysicsEngine::Instance()->GetDampingFactor();

	angVelocity += invInertia * torque * dt;

	angVelocity = angVelocity * PhysicsEngine::Instance() -> GetDampingFactor();
}

/* Between these two functions the physics engine will solve for velocity
   based on collisions/constraints etc. So we need to integrate velocity, solve 
   constraints, then use final velocity to update position. 
*/

void PhysicsNode::IntegrateForPosition(float dt)
{
	position += linVelocity * dt;

	orientation = orientation + Quaternion(angVelocity * dt * 0.5f, 0.0f) * orientation;

	orientation.Normalise();

	Vector3 v_min, v_max;

	//x axis
	collisionShape->GetMinMaxVertexOnAxis(Vector3(1, 0, 0), v_min, v_max);
	axisAlignedBoundingBox.x = v_max.x - position.x;

	//y axis
	collisionShape->GetMinMaxVertexOnAxis(Vector3(0, 1, 0), v_min, v_max);
	axisAlignedBoundingBox.y = v_max.y - position.y;

	//z axis
	collisionShape->GetMinMaxVertexOnAxis(Vector3(0, 0, 1), v_min, v_max);
	axisAlignedBoundingBox.z = v_max.z - position.z;

	if (subspace)
		subspace->UpdateNode(this);

	//Finally: Notify any listener's that this PhysicsNode has a new world transform.
	// - This is used by GameObject to set the worldTransform of any RenderNode's. 
	//   Please don't delete this!!!!!
	FireOnUpdateCallback();
}

void PhysicsNode::SetPosition(const Vector3& v) {
	position = v;
	if(subspace)
		subspace->UpdateNode(this);
	FireOnUpdateCallback();
}