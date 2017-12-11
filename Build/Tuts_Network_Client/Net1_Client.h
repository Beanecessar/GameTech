#pragma once

#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>
#include "MazeData.h"
#include "MazeRenderer.h"

class MazeRenderer;

#define WAITING_MAZE_DATA	0
#define WAITING_PATH		1
#define CREATING_START_GOAL 2
#define WAITING_POSITION	3

//Basic Network Example

class Net1_Client : public Scene
{
public:
	Net1_Client(const std::string& friendly_name);

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;


	void ProcessNetworkEvent(const ENetEvent& evnt);

protected:
	ENetPacket* packet = nullptr;

	NetworkBase network;
	ENetPeer*	serverConnection;

	uint state;

	MazeParameter mp;
	MazeData md;

	unsigned pathSize;
	//std::list<pair<Vector3, Vector3>> searchHistory;
	list<Vector3> path;

	MazeRenderer* mazeRenderer;

	Mesh* wallMesh;
};