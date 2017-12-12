
/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Client" at the same time.
- To do this:-
	1. right click on the entire solution (top of the solution exporerer) and go to properties
	2. Go to Common Properties -> Statup Project
	3. Select Multiple Statup Projects
	4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
and going to Debug->Start New Instance.




FOR MORE NETWORKING INFORMATION SEE "Tuts_Network_Client -> Net1_Client.h"



		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <enet\enet.h>
#include <nclgl\GameTimer.h>
#include <nclgl\Vector3.h>
#include <nclgl\common.h>
#include <ncltech\NetworkBase.h>
#include "MazeGenerator.h"
#include "MazeRenderer.h"
#include "MazeData.h"
#include "SearchAStar.h"
#include "PacketFlag.h"

class SearchAStar;

//Needed to get computer adapter IPv4 addresses via windows
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")


#define SERVER_PORT 1234
#define UPDATE_TIMESTEP (1.0f / 30.0f) //send 30 position updates per second

#define WAITING_MAZE_PARAMETER			0
#define WAITING_START_GOAL				1
#define WAITING_INSTRUCTION				2

NetworkBase server;
GameTimer timer;
MazeGenerator* mazeGen;
MazeRenderer* mazeRen;
std::vector<uint> state;
float accum_time = 0.0f;


void Win32_PrintAllAdapterIPAddresses();

int onExit(int exitcode)
{
	server.Release();
	system("pause");
	exit(exitcode);
}

void InitializeServer() {
	mazeGen = new MazeGenerator();
	mazeRen = nullptr;

	srand(93225);
}

void UpdateServer() {

}

ENetPacket* UpdatePacket() {
	return NULL;
}


//Yay Win32 code >.>
//  - Grabs a list of all network adapters on the computer and prints out all IPv4 addresses associated with them.
void Win32_PrintAllAdapterIPAddresses()
{
	//Initially allocate 5KB of memory to store all adapter info
	ULONG outBufLen = 5000;
	

	IP_ADAPTER_INFO* pAdapters = NULL;
	DWORD status = ERROR_BUFFER_OVERFLOW;

	//Keep attempting to fit all adapter info inside our buffer, allocating more memory if needed
	// Note: Will exit after 5 failed attempts, or not enough memory. Lets pray it never comes to this!
	for (int i = 0; i < 5 && (status == ERROR_BUFFER_OVERFLOW); i++)
	{
		pAdapters = (IP_ADAPTER_INFO *)malloc(outBufLen);
		if (pAdapters != NULL) {

			//Get Network Adapter Info
			status = GetAdaptersInfo(pAdapters, &outBufLen);

			// Increase memory pool if needed
			if (status == ERROR_BUFFER_OVERFLOW) {
				free(pAdapters);
				pAdapters = NULL;
			}
			else {
				break;
			}
		}
	}

	
	if (pAdapters != NULL)
	{
		//Iterate through all Network Adapters, and print all IPv4 addresses associated with them to the console
		// - Adapters here are stored as a linked list termenated with a NULL next-pointer
		IP_ADAPTER_INFO* cAdapter = &pAdapters[0];
		while (cAdapter != NULL)
		{
			IP_ADDR_STRING* cIpAddress = &cAdapter->IpAddressList;
			while (cIpAddress != NULL)
			{
				printf("\t - Listening for connections on %s:%u\n", cIpAddress->IpAddress.String, SERVER_PORT);
				cIpAddress = cIpAddress->Next;
			}
			cAdapter = cAdapter->Next;
		}

		free(pAdapters);
	}
	
}

int main(int arcg, char** argv)
{
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}

	//Initialize Server on Port 1234, with a possible 32 clients connected at any time
	if (!server.Initialize(SERVER_PORT, 32))
	{
		fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
		onExit(EXIT_FAILURE);
	}

	InitializeServer();

	printf("Server Initiated\n");


	Win32_PrintAllAdapterIPAddresses();

	timer.GetTimedMS();
	while (true)
	{
		float dt = timer.GetTimedMS() * 0.001f;
		accum_time += dt;

		UpdateServer();

		//Handle All Incoming Packets and Send any enqued packets
		server.ServiceNetwork(dt, [&](const ENetEvent& evnt)
		{
			switch (evnt.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				state.push_back(WAITING_MAZE_PARAMETER);
				printf("- New Client Connected\n");
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				//printf("\t Client %d says: %s\n", evnt.peer->incomingPeerID, evnt.packet->data);

				if (state[evnt.peer->incomingPeerID]==WAITING_MAZE_PARAMETER && evnt.packet->dataLength==sizeof(MazeParameter))
				{
					MazeParameter* mp = (MazeParameter*)evnt.packet->data;
					mazeGen->Generate(mp->size, mp->density);
					mazeRen = new MazeRenderer(mazeGen);
					unsigned flatSize = mazeRen->GetFlatMazeSize();
					unsigned numWalls = mazeRen->GetNumOfWalls();

					char* data = new char[sizeof(PacketFlag)+sizeof(unsigned)*2 + sizeof(bool)*flatSize*flatSize];
					
					PacketFlag pf = PacketFlag::MazeArray;
					memcpy(data, &pf, sizeof(unsigned));
					unsigned offset = sizeof(PacketFlag);

					memcpy(data + offset, &flatSize, sizeof(unsigned));
					offset += sizeof(unsigned);

					memcpy(data + offset, &numWalls, sizeof(unsigned));
					offset += sizeof(unsigned);

					memcpy(data + offset, mazeRen->GetFlatMaze(), sizeof(bool)*flatSize*flatSize);
					offset += sizeof(bool)*flatSize*flatSize;
					
					bool* maze = mazeRen->GetFlatMaze();

// 					for (unsigned i = 0; i < flatSize; ++i)
// 					{
// 						for (unsigned j = 0; j < flatSize; ++j)
// 						{
// 							cout << (maze[flatSize*i + j] ? "1" : ".");
// 						}
// 						cout << endl;
// 					}

					ENetPacket* packet = enet_packet_create(data, offset, 0);
					enet_peer_send(evnt.peer,0,packet);

					delete[] data;

					state[evnt.peer->incomingPeerID] = WAITING_START_GOAL;
				}
				else if(state[evnt.peer->incomingPeerID] == WAITING_START_GOAL&& evnt.packet->dataLength == 2*sizeof(Vector2))
				{
					Vector2 start_pos, goal_pos;
					memcpy(&start_pos, evnt.packet->data, sizeof(Vector2));
					memcpy(&goal_pos, evnt.packet->data + sizeof(Vector2), sizeof(Vector2));

					mazeGen->SetStartGoal(start_pos, goal_pos);
					GraphNode* startNode = mazeGen->GetStartNode();
					GraphNode* goalNode = mazeGen->GetGoalNode();

					SearchAStar as_searcher;

					as_searcher.FindBestPath(startNode, goalNode);

					list<const GraphNode*> path = as_searcher.GetFinalPath();

					char* data = new char[sizeof(PacketFlag) + sizeof(unsigned) + sizeof(float)*path.size() * 3];

					PacketFlag pf = PacketFlag::MazePath;
					memcpy(data, &pf, sizeof(PacketFlag));
					unsigned offset = sizeof(PacketFlag);

					unsigned listSize = path.size();
					memcpy(data + offset, &listSize, sizeof(unsigned));
					offset += sizeof(unsigned);

					float temp;
					for (auto i = path.begin(); i != path.end(); ++i)
					{
						temp = (*i)->_pos.x;
						memcpy(data + offset, &temp, sizeof(unsigned));
						offset += sizeof(float);

						temp = (*i)->_pos.y;
						memcpy(data + offset, &temp, sizeof(unsigned));
						offset += sizeof(float);

						temp = (*i)->_pos.z;
						memcpy(data + offset, &temp, sizeof(unsigned));
						offset += sizeof(float);
					}
					
					ENetPacket* packet = enet_packet_create(data, sizeof(PacketFlag) + sizeof(float)*path.size() * 3, 0);
					enet_peer_send(evnt.peer, 0, packet);

					delete[] data;
				}
				else if (state[evnt.peer->incomingPeerID] == WAITING_INSTRUCTION)
				{
					;
				}

				enet_packet_destroy(evnt.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("- Client %d has disconnected.\n", evnt.peer->incomingPeerID);
				state[evnt.peer->incomingPeerID] = WAITING_MAZE_PARAMETER;
				break;
			}
		});

		//Broadcast update packet to all connected clients at a rate of UPDATE_TIMESTEP updates per second
		if (accum_time >= UPDATE_TIMESTEP)
		{

			//Packet data
			// - At the moment this is just a position update that rotates around the origin of the world
			//   though this can be any variable, structure or class you wish. Just remember that everything 
			//   you send takes up valuable network bandwidth so no sending every PhysicsObject struct each frame ;)
			accum_time = 0.0f;

			//Create the packet and broadcast it (unreliable transport) to all clients

			//ENetPacket* packet = UpdatePacket();

			//enet_host_broadcast(server.m_pNetwork, 0, packet);
		}

		Sleep(0);
	}

	system("pause");
	server.Release();
}

