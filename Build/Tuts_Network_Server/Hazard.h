#pragma once

#include "SearchAlgorithm.h"
#include "SearchAStar.h"
#include "MazeGenerator.h"
#include <nclgl/Vector3.h>

enum HazardState {
	Patrol,
	Pursue,
	Flee
};

enum HazardEvent {
	Find,
	Catch,
	Away
};

class Hazard {
public:
	Hazard(MazeGenerator* mg);
	~Hazard();

	inline const Vector2 GetPosition() const { return position; }

	inline void SetPosition(Vector2 pos) { position = pos; }

	inline void SetTarget(Vector2* tar) { avatorPos = tar; }

	//Updata hazard's position
	void Updata(float dt);


protected:
	HazardState state;

	float speed;
	float patrolRange;

	MazeGenerator* mazeGen;
	SearchAStar as_searcher;
	std::list<const GraphNode*> path;

	Vector2 position;
	Vector2* avatorPos;

	void(*EventCallback)(HazardEvent evt);
};

Hazard::Hazard(MazeGenerator *mg) {
	mazeGen = mg;
	state = HazardState::Patrol;
	speed = 2.0f;
	patrolRange = 5.f;
	EventCallback = nullptr;

	as_searcher.SetWeightings(1.0f, 1.0f);
}

void Hazard::Updata(float dt) {
	switch (state)
	{
	case Patrol:
		//Change to pursue state if the avator in the patrol range
	{
		if ((*avatorPos - position).Length() < patrolRange)
		{
			if(EventCallback)
				EventCallback(HazardEvent::Find);
			path.clear();
			state = HazardState::Pursue;
			break;
		}

		if (path.empty())
		{
			//Generate new path
			GraphNode *start, *end;
			do
			{
				unsigned x = (unsigned)(position.x + 0.5);
				unsigned y = (unsigned)(position.y + 0.5);

				start = &mazeGen->allNodes[y*mazeGen->size + x];

				x = rand() % mazeGen->GetSize();
				y = rand() % mazeGen->GetSize();

				end = &mazeGen->allNodes[y*mazeGen->size + x];

			} while (!as_searcher.FindBestPath(start, end));

			path = as_searcher.GetFinalPath();
		}
		else
		{
			//Follow the current path
			Vector2 nextCheckPoint = Vector2(path.front()->_pos.x, path.front()->_pos.y);
			Vector2 direction = nextCheckPoint - position;

			if (direction.Length() > speed*dt) {
				//Haven't reached next check point 
				direction.Normalise();

				position = position + direction*speed*dt;
			}
			else
			{
				position = nextCheckPoint;
				path.pop_front();
			}
		}
	}
	break;
	case Pursue:
		//Change to flee if catch the avator
	{
		if ((*avatorPos - position).Length() < speed*dt)
		{
			if (EventCallback)
				EventCallback(HazardEvent::Catch);
			path.clear();
			state = HazardState::Flee;
			break;
		}
		
		auto generateNewPath = [&]()->void {
			unsigned startPosX = (unsigned)(position.x + 0.5);
			unsigned startPosY = (unsigned)(position.y + 0.5);
			GraphNode* start = mazeGen->GetGraphNode(startPosX, startPosY);

			unsigned goalPosX = (unsigned)(avatorPos->x + 0.5);
			unsigned goalPosY = (unsigned)(avatorPos->y + 0.5);
			GraphNode* goal = mazeGen->GetGraphNode(goalPosX, goalPosY);

			if (start&&goal)
			{
				as_searcher.FindBestPath(start, goal);
				path = as_searcher.GetFinalPath();
				if (!path.empty())
				{
					path.pop_front();
				}
			}
		};

		if (path.empty())
		{
			generateNewPath();
		}
		else
		{
			//Follow the current path
			Vector2 nextCheckPoint = Vector2(path.front()->_pos.x, path.front()->_pos.y);
			Vector2 direction = nextCheckPoint - position;

			if (direction.Length() > speed*dt) {
				//Haven't reached next check point 
				direction.Normalise();

				position = position + direction*speed*dt;
			}
			else
			{
				position = nextCheckPoint;
				generateNewPath();
			}
		}
	}
	break;
	case Flee:
		//Change to patrol if the avator out of patrol range
	{
		if ((*avatorPos - position).Length() > patrolRange)
		{
			if (EventCallback)
				EventCallback(HazardEvent::Away);
			path.clear();
			state = HazardState::Patrol;
			break;
		}

		if (path.empty())
		{
			//Generate new path
			GraphNode *start, *end;
			do
			{
				unsigned x = (unsigned)(position.x + 0.5);
				unsigned y = (unsigned)(position.y + 0.5);

				start = &mazeGen->allNodes[y*mazeGen->size + x];

				x = rand() % mazeGen->GetSize();
				y = rand() % mazeGen->GetSize();

				//Find a goal position which far from avator
				for (unsigned i = 0; i < 10; ++i) {
					unsigned tx = rand() % mazeGen->GetSize();
					unsigned ty = rand() % mazeGen->GetSize();
					if ((Vector2(x, y) - *avatorPos).Length() < (Vector2(tx, ty) - *avatorPos).Length())
					{
						x = tx;
						y = ty;
					}
				}

				end = &mazeGen->allNodes[y*mazeGen->size + x];

			} while (!as_searcher.FindBestPath(start, end));

			path = as_searcher.GetFinalPath();
		}
		else
		{
			//Follow the current path
			Vector2 nextCheckPoint = Vector2(path.front()->_pos.x, path.front()->_pos.y);
			Vector2 direction = nextCheckPoint - position;

			if (direction.Length() > speed*dt) {
				//Haven't reached next check point 
				direction.Normalise();

				position = position + direction*speed*dt;
			}
			else
			{
				position = nextCheckPoint;
				path.pop_front();
			}
		}
	}
		break;
	default:
		break;
	}
}