#pragma once

#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>
#include "MazeData.h"
#include "MazeRenderer.h"

class MazeRenderer;

#define WAITING_MAZE_DATA	0
#define CREATING_START_GOAL 1
#define WAITING_PATH		2
#define CREATING_AVATOR		3
#define WAITING_POSITION	4

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
<<<<<<< HEAD
=======
<<<<<<< Updated upstream
=======
>>>>>>> CUDA_DEBUG

	uint state;

	MazeParameter mp;
	MazeData md;

	unsigned pathSize;
	//std::list<pair<Vector3, Vector3>> searchHistory;
	list<Vector3> path;

	RenderNode *avator;
<<<<<<< HEAD
=======
	vector<pair<RenderNode*, Vector2>> hazards;
>>>>>>> CUDA_DEBUG
	Vector2 currentPos;

	MazeRenderer* mazeRenderer;

	Mesh* wallMesh;
<<<<<<< HEAD
=======
>>>>>>> Stashed changes
>>>>>>> CUDA_DEBUG
};