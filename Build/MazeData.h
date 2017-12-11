#pragma once

struct MazeData
{
	unsigned flat_maze_size;
	unsigned num_walls;
	bool*	flat_maze;
};

struct MazeParameter
{
	unsigned size;
	float density;
};