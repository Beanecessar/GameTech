#include "MazeRenderer.h"
#include <ncltech\CommonUtils.h>

const Vector4 wall_color = Vector4(1.f, 0.8f, 0.3f, 1);



MazeRenderer::MazeRenderer(MazeGenerator* gen, Mesh* wallmesh)
	: GameObject("")
	, mesh(wallmesh)
	, maze(gen)
	, flat_maze(NULL)
{
	this->SetRender(new RenderNode());

	if (maze)
	{
		sphere = CommonMeshes::Sphere();

		uint num_walls = Generate_FlatMaze();

		wall_descriptors.reserve(num_walls);

		Generate_ConstructWalls();

		Generate_BuildRenderNodes();
	}
}

MazeRenderer::MazeRenderer(uint flat_maze_size,uint num_walls, bool*& flat_maze, Mesh* wallmesh)
	: GameObject("")
	, mesh(wallmesh)
	, maze(NULL)
	, flat_maze_size(flat_maze_size)
	, flat_maze(flat_maze)
	, start_pos(Vector2(-1, -1))
	, goal_pos(Vector2(-1, -1))
	, stone_pos(Vector2(-1, -1))
	, IsStoneRenewed(false)
{
	sphere = CommonMeshes::Sphere();

	this->SetRender(new RenderNode());

	wall_descriptors.reserve(num_walls);

	Generate_ClickPoints();

	Generate_ConstructWalls();

	Generate_BuildRenderNodes();
}

MazeRenderer::~MazeRenderer()
{
	for (auto i = Render()->GetChildIteratorStart(); i != Render()->GetChildIteratorEnd(); ++i)
	{
		ScreenPicker::Instance()->UnregisterNodeForMouseCallback(*i);
	}

	//SAFE_DELETE(sphere);
	sphere = nullptr;

	mesh = NULL;
	maze = NULL;

	if (flat_maze)
	{
		delete[] flat_maze;
		flat_maze = NULL;
	}
}

//The search history draws from edges because they already store the 'to'
// and 'from' of GraphNodes.
void MazeRenderer::DrawSearchHistory(const list<pair<Vector3,Vector3>>& history,unsigned mazeSize,unsigned historySize, float line_width)
{
	float grid_scalar = 1.0f / (float)mazeSize;
	float col_factor = 0.2f / (float)historySize;

	Matrix4 transform = this->Render()->GetWorldTransform();

	float index = 0.0f;
	for (const pair<Vector3,Vector3>& edge : history)
	{
		Vector3 start = transform * Vector3(
			(edge.first.x + 0.5f) * grid_scalar,
			0.1f,
			(edge.first.y + 0.5f) * grid_scalar);

		Vector3 end = transform * Vector3(
			(edge.second.x + 0.5f) * grid_scalar,
			0.1f,
			(edge.second.y + 0.5f) * grid_scalar);

		NCLDebug::DrawThickLine(start, end, line_width, CommonUtils::GenColor(0.8f + index * col_factor));
		index += 1.0f;
	}
}

void MazeRenderer::DrawPath(const list<Vector3>& path, unsigned mazeSize, unsigned pathSize, float line_width)
{
	float grid_scalar = 1.0f / (float)mazeSize;
	float col_factor = 0.2f / (float)pathSize;

	Matrix4 transform = this->Render()->GetWorldTransform();

	float index = 0.0f;
	for (auto i=path.begin();i!=path.end();++i)
	{
		Vector3 start = transform * Vector3(
			((*i).x + 0.5f) * grid_scalar,
			0.1f,
			((*i).y + 0.5f) * grid_scalar);

		++i;
		if (i!=path.end())
		{
			Vector3 end = transform * Vector3(
				((*i).x + 0.5f) * grid_scalar,
				0.1f,
				((*i).y + 0.5f) * grid_scalar);

			NCLDebug::DrawThickLineNDT(start, end, line_width, CommonUtils::GenColor(0.8f + index * col_factor));
		}
		--i;

		NCLDebug::DrawPointNDT(start, line_width*1.5, Vector4(1.f, 1.0f, 0.5f, 1.0f));

		index += 1.0f;
	}
}

uint MazeRenderer::Generate_FlatMaze()
{
	//Generates a 3xsize by 3xsize array of booleans, where 
	// a true value corresponds to a solid wall and false to open space.
	// - Each GraphNode is a 2x2 open space with a 1 pixel wall around it.
	uint size = maze->GetSize();
	GraphEdge* allEdges = maze->allEdges;

	flat_maze_size = size * 3 - 1;

	if (flat_maze) delete[] flat_maze;
	flat_maze = new bool[flat_maze_size * flat_maze_size];
	memset(flat_maze, 0, flat_maze_size * flat_maze_size * sizeof(bool));


	uint base_offset = size * (size - 1);
	uint num_walls = 0;
	//Iterate over each cell in the maze
	for (uint y = 0; y < size; ++y)
	{
		uint y3 = y * 3;
		for (uint x = 0; x < size; ++x)
		{
			int x3 = x * 3;

			//Lookup the corresponding edges that occupy that grid cell
			// and if they are walls, set plot their locations on our 2D
			// map.
			//- Yes... it's a horrible branching inner for-loop, my bad! :(
			if (x < size - 1)
			{
				GraphEdge& edgeX = allEdges[(y * (size - 1) + x)];
				if (edgeX._iswall)
				{
					flat_maze[(y * 3) * flat_maze_size + (x * 3 + 2)] = true;
					flat_maze[(y * 3 + 1) * flat_maze_size + (x * 3 + 2)] = true;
					num_walls += 2;
				}
			}
			if (y < size - 1)
			{
				GraphEdge& edgeY = allEdges[base_offset + (x * (size - 1) + y)];
				if (edgeY._iswall)
				{
					flat_maze[(y * 3 + 2) * flat_maze_size + (x * 3)] = true;
					flat_maze[(y * 3 + 2) * flat_maze_size + (x * 3 + 1)] = true;
					num_walls += 2;
				}
			}

			//As it's now a 3x3 cell for each, and the doorways are 2x1 or 1x2
			// we need to add an extra wall for the diagonals.
			if (x < size - 1 && y < size - 1)
			{
				flat_maze[(y3 + 2) * flat_maze_size + x3 + 2] = true;
				num_walls++;
			}
		}
	}

	return num_walls;
}

void ClickPointCallback(RenderNode * clickPoint, MazeRenderer * mRender, const Vector2 mazePos, float dt, const Vector3 & newWsPos, const Vector3 & wsMovedAmount, bool stopDragging) {
	static unsigned LastClickLeft = 0;
	Vector3 clickedPos = newWsPos + wsMovedAmount;
	
	if (Window::GetMouse()->ButtonDown(MOUSE_LEFT))
	{
		if (mRender->GetStartSphere())
		{
			mRender->GetStartSphere()->SetColor(Vector4(0, 0, 0, 0));
		}
		clickPoint->SetColor(Vector4(1.f, 0, 0, 1.f));

		LastClickLeft = 0;
	}
	else if (Window::GetMouse()->ButtonDown(MOUSE_RIGHT))
	{
		if (mRender->GetGoalSphere())
		{
			mRender->GetGoalSphere()->SetColor(Vector4(0, 0, 0, 0));
		}
		clickPoint->SetColor(Vector4(0, 1.f, 0, 1.f));
		
		LastClickLeft = 1;
	}
	else if (Window::GetMouse()->ButtonDown(MOUSE_MIDDLE))
	{
		LastClickLeft = 2;
	}

	if (stopDragging) {
		if (LastClickLeft == 0) {
			mRender->SetStartSphere(clickPoint);
			mRender->SetStartPosition(mazePos);
		}
		else if(LastClickLeft == 1) {
			mRender->SetGoalSphere(clickPoint);
			mRender->SetGoalPosition(mazePos);
		}
		else {
			mRender->SetStoneSphere(clickPoint);
			mRender->SetStonePosition(mazePos);
		}
	}
}

void MazeRenderer::Generate_ClickPoints() {
	RenderNode *clickPoint,*root = Render();
	const float scalar = 1.f / (float)flat_maze_size;

	for (size_t i = 0; i < (flat_maze_size + 1) / 3; i++)
	{
		for (size_t j = 0; j < (flat_maze_size + 1) / 3; j++)
		{
			Vector3 cellpos = Vector3(
				i * 3,
				0.0f,
				j * 3
			) * scalar;
			Vector3 cellsize = Vector3(
				scalar * 2,
				1.0f,
				scalar * 2
			);

			clickPoint = new RenderNode(sphere, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
			clickPoint->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
			ScreenPicker::Instance()->RegisterNodeForMouseCallback(
				clickPoint,
				std::bind(&ClickPointCallback, clickPoint,this, Vector2(i, j), std::placeholders::_1,std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
			);
			root->AddChild(clickPoint);
		}
	}
}

void MazeRenderer::Generate_ConstructWalls()
{
	//First try and compact adjacent walls down, so we don't
	// just end up creating lots of little cube's.

	//Horizontal wall pass
	for (uint y = 0; y < flat_maze_size; ++y)
	{
		for (uint x = 0; x < flat_maze_size; ++x)
		{
			//Is the current cell a wall?
			if (flat_maze[y*flat_maze_size + x])
			{
				WallDescriptor w(x, y);

				uint old_x = x;

				//If we found a wall, keep iterating in the current
				// search direction and see if we can join it with
				// adjacent walls.
				for (++x; x < flat_maze_size; ++x)
				{
					if (!flat_maze[y * flat_maze_size + x])
						break;

					flat_maze[y * flat_maze_size + x] = false;
				}

				w._xe = x;

				//If the wall is only 1x1, ignore it for the vertical-pass.
				if (w._xe - w._xs > 1)
				{
					flat_maze[y * flat_maze_size + old_x] = false;
					wall_descriptors.push_back(w);
				}
			}
		}
	}

	//Vertical wall pass
	for (uint x = 0; x < flat_maze_size; ++x)
	{
		for (uint y = 0; y < flat_maze_size; ++y)
		{
			if (flat_maze[y * flat_maze_size + x])
			{
				WallDescriptor w(x, y);

				for (++y; y < flat_maze_size && flat_maze[y * flat_maze_size + x]; ++y) {}

				w._ye = y;
				wall_descriptors.push_back(w);
			}
		}
	}
}

void MazeRenderer::Generate_BuildRenderNodes()
{
	//Turn compacted walls into RenderNodes
	RenderNode *cube, *root = Render();

	//Turn walls into 3D Cuboids
	const float scalar = 1.f / (float)flat_maze_size;
	for (const WallDescriptor& w : wall_descriptors)
	{
		Vector3 start = Vector3(
			float(w._xs),
			0.0f,
			float(w._ys));

		Vector3 size = Vector3(
			float(w._xe - w._xs),
			0.0f,
			float(w._ye - w._ys)
		);


		start = start * scalar;
		Vector3 end = start + size * scalar;
		end.y = 0.75f;

		Vector3 centre = (end + start) * 0.5f;
		Vector3 halfDims = centre - start;

		cube = new RenderNode(mesh, wall_color);
		cube->SetTransform(Matrix4::Translation(centre) * Matrix4::Scale(halfDims));
		root->AddChild(cube);
	}

	//Add bounding edge walls to the maze
	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(-scalar*0.5f, 0.25f, 0.5)) * Matrix4::Scale(Vector3(scalar*0.5f, 0.25f, scalar + 0.5f)));
	root->AddChild(cube);

	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(1.f + scalar*0.5f, 0.25f, 0.5)) * Matrix4::Scale(Vector3(scalar*0.5f, 0.25f, scalar + 0.5f)));
	root->AddChild(cube);

	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(0.5, 0.25f, -scalar*0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.25f, scalar*0.5f)));
	root->AddChild(cube);

	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(0.5, 0.25f, 1.f + scalar*0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.25f, scalar*0.5f)));
	root->AddChild(cube);

	this->SetRender(root);
}

// void MazeRenderer::SetStartGoal(Vector2 start, Vector2 goal) {
// 	start_pos = start;
// 	goal_pos = goal;
// 
// 	if (startSphere && goalSphere) {
// 		const float scalar = 1.f / (float)flat_maze_size;
// 
// 		Vector3 cellpos = Vector3(
// 			start_pos.x * 3,
// 			0.0f,
// 			start_pos.y * 3
// 		) * scalar;
// 		Vector3 cellsize = Vector3(
// 			scalar * 2,
// 			1.0f,
// 			scalar * 2
// 		);
// 		startSphere->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
// 
// 		cellpos = Vector3(
// 			goal_pos.x * 3,
// 			0.0f,
// 			goal_pos.y * 3
// 		) * scalar;
// 		goalSphere->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
// 	}
// 	else {
// 		RenderNode *root = Render();
// 		const float scalar = 1.f / (float)flat_maze_size;
// 
// 		Vector3 cellpos = Vector3(
// 			start_pos.x * 3,
// 			0.0f,
// 			start_pos.y * 3
// 		) * scalar;
// 		Vector3 cellsize = Vector3(
// 			scalar * 2,
// 			1.0f,
// 			scalar * 2
// 		);
// 
// 		startSphere = new RenderNode(sphere, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
// 		startSphere->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
// 		root->AddChild(startSphere);
// 
// 		cellpos = Vector3(
// 			goal_pos.x * 3,
// 			0.0f,
// 			goal_pos.y * 3
// 		) * scalar;
// 		goalSphere = new RenderNode(sphere, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
// 		goalSphere->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
// 		root->AddChild(goalSphere);
// 	}
// }