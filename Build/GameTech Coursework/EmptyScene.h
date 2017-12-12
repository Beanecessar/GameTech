#pragma once

#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\CuboidCollisionShape.h>
#include <ncltech\SphereCollisionShape.h>

//Fully striped back scene to use as a template for new scenes.
class EmptyScene : public Scene
{
public:
	EmptyScene(const std::string& friendly_name) 
		: Scene(friendly_name)
	{
	}

	virtual ~EmptyScene()
	{
	}

	virtual void OnInitializeScene() override
	{
		Scene::OnInitializeScene();

		//Who doesn't love finding some common ground?
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.5f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		GameObject* player = new GameObject("raptor");
		Mesh* m_Mesh = new OBJMesh(MESHDIR"raptor.obj");
		player->SetRender(new RenderNode(m_Mesh));
		player->Render()->SetBoundingRadius(1.0f);
		player->Render()->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		player->Render()->SetTransform(Matrix4::Scale(Vector3(5.0, 5.0, 5.0)));

		player->SetPhysics(new PhysicsNode());
		player->Physics()->SetPosition(Vector3(0.0f, 0.5f, 0.0f));
		player->Physics()->SetInverseMass(1.f);
		player->Physics()->SetInverseInertia(CuboidCollisionShape(Vector3(0.5f, 0.5f, 1.f)).BuildInverseInertia(1.f));
		player->Physics()->AddCollisionShape(new CuboidCollisionShape(Vector3(0.1f, 0.1f, 0.5f)), Vector3(0, 0.25f, 0.85f));
		player->Physics()->AddCollisionShape(new CuboidCollisionShape(Vector3(0.2f, 0.38f, 0.3f)), Vector3(0, -0.06f, 0));
		player->Physics()->AddCollisionShape(new SphereCollisionShape(0.3f), Vector3(0, 0.3f, -0.55f));

		this->AddGameObject(player);
	}

	void EmptyScene::OnCleanupScene()
	{
		//Just delete all created game objects 
		//  - this is the default command on any Scene instance so we don't really need to override this function here.
		Scene::OnCleanupScene();
	}

};