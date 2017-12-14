#pragma once

#include <ncltech\GameObject.h>
#include <ncltech\CommonMeshes.h>
#include <ncltech\ScreenPicker.h>
#include "MazeGenerator.h"
#include "SearchAlgorithm.h"
#include <nclgl/Vector2.h>


struct WallDescriptor
{
	uint _xs, _xe;
	uint _ys, _ye;

	WallDescriptor(uint x, uint y) : _xs(x), _xe(x + 1), _ys(y), _ye(y + 1) {}
};

typedef std::vector<WallDescriptor> WallDescriptorVec;


class MazeRenderer : public GameObject
{
public:
	MazeRenderer(MazeGenerator* gen, Mesh* wallmesh = CommonMeshes::Cube());
	MazeRenderer(uint flat_maze_size,uint num_walls , bool*& flat_maze,Mesh* wallmesh = CommonMeshes::Cube());
	virtual ~MazeRenderer();

	//The search history draws from edges because they already store the 'to'
	// and 'from' of GraphNodes.
	void DrawSearchHistory(const list<pair<Vector3, Vector3>>& history, unsigned mazeSize, unsigned historySize, float line_width);
	void DrawPath(const list<Vector3>& path, unsigned mazeSize, unsigned pathSize, float line_width);

	//bool* GetFlatMaze() const { return flat_maze; }
	inline void SetStartPosition(Vector2 const start)	{	start_pos = start; }
	inline const Vector2 GetStartPosition() const { return start_pos; }
	inline void SetGoalPosition(Vector2 const goal)	{	goal_pos = goal; IsGoalRenewed = true;}
	inline const Vector2 GetGoalPosition() const { return goal_pos; }
	inline void SetStonePosition(Vector2 const stone) { stone_pos = stone; IsStoneRenewed = true; }
	inline const Vector2 GetStonePosition() const { return stone_pos; }

	inline RenderNode* GetStartSphere() const { return startSphere; }
	inline void SetStartSphere(RenderNode* node) { startSphere = node; }
	inline RenderNode* GetGoalSphere() const { return goalSphere; }
	inline void SetGoalSphere(RenderNode* node) { goalSphere = node; }
	inline RenderNode* GetStoneSphere() const { return stoneSphere; }
	inline void SetStoneSphere(RenderNode* node) { stoneSphere = node; }

	bool IsGoalRenewed;
	bool IsStoneRenewed;

protected:
	//Turn MazeGenerator data into flat 2D map (3 size x 3 size) of boolean's
	// - True for wall
	// - False for empty
	//Returns uint: Guess at the number of walls required
	uint Generate_FlatMaze();

	void Generate_ClickPoints();

	//Construct a list of WallDescriptors from the flat 2D map generated above.
	void Generate_ConstructWalls();

	//Finally turns the wall descriptors into actual renderable walls ready
	// to be seen on screen.
	void Generate_BuildRenderNodes();

protected:
	MazeGenerator*	maze;
	Mesh*			mesh;
	Mesh*			sphere;

	bool*	flat_maze;	//[flat_maze_size x flat_maze_size]
	uint	flat_maze_size;

	Vector2 start_pos;
	Vector2 goal_pos;
	Vector2 stone_pos;

	RenderNode* startSphere;
	RenderNode* goalSphere;
	RenderNode* stoneSphere;

	WallDescriptorVec	wall_descriptors;
};