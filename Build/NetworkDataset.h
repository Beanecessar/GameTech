#pragma once

#include "SearchAlgorithm.h"
#include <ncltech\NetworkBase.h>

enum ClientState {
	WaitingMazeData,
	InputingStartGoal,
	CreatingStartGoal,
	WaitingPath,
	CreatingAvator,
	WaitingPosition
};

enum ServerState
{
	Idle,
	WaitingMazeParameter,
	WaitingStartGoal,
	WaitingInstruction,
	SendingPosition
};

struct ClientData
{
	unsigned ID;
	unsigned state = ServerState::WaitingMazeParameter;
	Vector2 startPos = Vector2(0, 0);
	Vector2 velocityNormal = Vector2(0, 0);
	Vector2 goalPos = Vector2(0, 0);
	Vector2 stonePos = Vector2(-1, -1);
	list<const GraphNode*> path;
	Vector2 currentPos = Vector2(0, 0);
};