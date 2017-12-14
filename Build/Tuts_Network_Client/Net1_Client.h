#pragma once

#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>
#include "MazeData.h"
#include "MazeRenderer.h"
#include "NetworkDataset.h"

class MazeRenderer;

//Basic Network Example

class Net1_Client : public Scene
{
public:
	Net1_Client(const std::string& friendly_name);

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;

	void ProcessNetworkEvent(const ENetEvent& evnt);

	void UpdateClientStateMachine(float dt);

	void CleanHazards();

	void SetMazeParameter(MazeParameter p) { mp = p; }
	const MazeParameter GetMazeParameter() const { return mp; }

protected:
	ENetPacket* packet = nullptr;

	NetworkBase network;
	ENetPeer*	serverConnection;

	ClientState state;

	MazeParameter mp;
	MazeData md;

	unsigned pathSize;
	//std::list<pair<Vector3, Vector3>> searchHistory;
	list<Vector3> path;

	RenderNode *avator;
	vector<pair<RenderNode*, Vector2>> hazards;
	Vector2 currentPos;

	MazeRenderer* mazeRenderer;

	Mesh* wallMesh;
};