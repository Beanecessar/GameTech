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

class Hazard {
public:
	Hazard(MazeGenerator* mg);
	~Hazard();

	inline const Vector3* GetPosition() const {}

	inline void SetPosition() const {}

	void Updata(float dt);

protected:
	HazardState state;

	float speed;
	float patrolRange;

	MazeGenerator* mazeGen;
	std::list<const GraphNode*> path;

	Vector3 position;
	Vector3* avatorPos;

	void(*StateChangeCallback)();
};

Hazard::Hazard(MazeGenerator *mg) {
	mazeGen = mg;
	state = HazardState::Patrol;
	speed = 8.0f;
	patrolRange = 20.f;
}

void Hazard::Updata(float dt) {
	if (state == HazardState::Patrol) {
		if ((*avatorPos - position).Length() < patrolRange)
		{
			state = HazardState::Pursue;
			StateChangeCallback();
		}
		else
		{
			if (path.empty())
			{
				//Generate new path
				GraphNode *start, *end;
				SearchAStar as_searcher;
				do 
				{
					unsigned x = rand() % mazeGen->GetSize();
					unsigned y = rand() % mazeGen->GetSize();

					start = &mazeGen->allNodes[y*mazeGen->size + x];

					unsigned x = rand() % mazeGen->GetSize();
					unsigned y = rand() % mazeGen->GetSize();

					end = &mazeGen->allNodes[y*mazeGen->size + x];

				} while (as_searcher.FindBestPath(start, end));

				path = as_searcher.GetFinalPath();
			}
		}
	}
}