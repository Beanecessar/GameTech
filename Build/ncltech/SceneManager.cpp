#include "SceneManager.h"
#include "PhysicsEngine.h"
#include "CommonMeshes.h"
#include <nclgl\NCLDebug.h>
#include <ncltech\CommonUtils.h>
#include "GraphicsPipeline.h"

SceneManager::SceneManager()
	: m_SceneIdx(NULL)
{
	CommonMeshes::InitializeMeshes();
}

SceneManager::~SceneManager()
{
	NCLLOG("[SceneManager] Closing scene manager");
	m_SceneIdx = 0;
	for (Scene* scene : m_vpAllScenes)
	{
		if (scene != scene)
		{
			scene->OnCleanupScene();
			delete scene;
		}
	}
	m_vpAllScenes.clear();

	CommonMeshes::ReleaseMeshes();
}

bool IsAGoodTarget(PhysicsNode* ball, PhysicsNode* target) {
	if (target->IsGoodTarget())
		SceneManager::Instance()->GetCurrentScene()->SetScore(SceneManager::Instance()->GetCurrentScene()->GetScore() + 100);
	if (target->IsBadTarget())
		SceneManager::Instance()->GetCurrentScene()->SetScore(SceneManager::Instance()->GetCurrentScene()->GetScore() - 50);
	return true;
}

void SceneManager::LaunchSphere(float speed, float ballsize) {
	const int dims = 4;
	const Vector4 col = Vector4(1.0f, 0.5f, 0.2f, 1.0f);
	Vector3 pos = GraphicsPipeline::Instance()->GetCamera()->GetPosition();
	float pitch = GraphicsPipeline::Instance()->GetCamera()->GetPitch();
	float yaw = GraphicsPipeline::Instance()->GetCamera()->GetYaw();

	Vector3 dir;

	dir.y = sin((float)DegToRad(pitch));
	dir.z = -cos((float)DegToRad(pitch));

	dir.x = dir.z*sin((float)DegToRad(yaw));
	dir.z = dir.z*cos((float)DegToRad(yaw));
	
	GameObject* sphere = CommonUtils::BuildSphereObject(
		"",					// Optional: Name
		pos,				// Position
		ballsize,			// Half-Dimensions
		true,				// Physics Enabled?
		10.f,				// Physical Mass (must have physics enabled)
		true,				// Physically Collidable (has collision shape)
		false,				// Dragable by user?
		col);// Render color
	sphere->Physics()->SetLinearVelocity(dir*speed);
	sphere->Physics()->SetOnCollisionCallback(IsAGoodTarget);
	SceneManager::Instance()->GetCurrentScene()->AddGameObject(sphere);
}


void SceneManager::EnqueueScene(Scene* scene)
{
	if (scene == NULL)
	{
		NCLERROR("Attempting to enqueue NULL scene");
		return;
	}

	m_vpAllScenes.push_back(scene);
	NCLLOG("[SceneManager] - Enqueued scene: \"%s\"", scene->GetSceneName().c_str());

	//If this was the first scene, activate it immediately
	if (m_vpAllScenes.size() == 1)
		JumpToScene(0);
	else
		Window::GetWindow().SetWindowTitle("NCLTech - [%d/%d] %s", m_SceneIdx + 1, m_vpAllScenes.size(), scene->GetSceneName().c_str());
}

void SceneManager::JumpToScene()
{
	JumpToScene((m_SceneIdx + 1) % m_vpAllScenes.size());
}

void SceneManager::JumpToScene(int idx)
{
	if (idx < 0 || idx >= (int)m_vpAllScenes.size())
	{
		NCLERROR("Invalid Scene Index: %d", idx);
		return;
	}

	//Clear up old scene
	if (scene)
	{
		NCLLOG("[SceneManager] - Exiting scene -");
		scene->OnCleanupScene();
		PhysicsEngine::Instance()->RemoveAllPhysicsObjects();
	}

	m_SceneIdx = idx;
	scene = m_vpAllScenes[idx];
	NCLLOG("");

	//Initialize new scene
	PhysicsEngine::Instance()->SetDefaults();
	GraphicsPipeline::Instance()->InitializeDefaults();
	scene->OnInitializeScene();
	Window::GetWindow().SetWindowTitle("NCLTech - [%d/%d] %s", idx + 1, m_vpAllScenes.size(), scene->GetSceneName().c_str());
	NCLLOG("[SceneManager] - Scene switched to: \"%s\"", scene->GetSceneName().c_str());
}

void SceneManager::JumpToScene(const std::string& friendly_name)
{
	bool found = false;
	uint idx = 0;
	for (uint i = 0; found == false && i < m_vpAllScenes.size(); ++i)
	{
		if (m_vpAllScenes[i]->GetSceneName() == friendly_name)
		{
			found = true;
			idx = i;
			break;
		}
	}

	if (found)
	{
		JumpToScene(idx);
	}
	else
	{
		NCLERROR("Unknown Scene Alias: \"%s\"", friendly_name.c_str());
	}
}