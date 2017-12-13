
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
<<<<<<< Updated upstream
=======
#include "MazeGenerator.h"
#include "MazeRenderer.h"
#include "MazeData.h"
#include "SearchAStar.h"
#include "PacketFlag.h"
#include "NetworkDataset.h"
#include "Hazard.h"

class SearchAStar;
>>>>>>> Stashed changes

//Needed to get computer adapter IPv4 addresses via windows
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")


#define SERVER_PORT 1234
#define UPDATE_TIMESTEP (1.0f / 30.0f) //send 30 position updates per second

NetworkBase server;
GameTimer timer;
<<<<<<< Updated upstream
float accum_time = 0.0f;
float rotation = 0.0f;
=======
MazeGenerator* mazeGen;
MazeRenderer* mazeRen;
vector<Hazard*> hazards;
std::vector<ClientData> clients;
float moving_speed = 1.f;
float accum_time = 0.0f;
size_t numOfHazards = 10;
>>>>>>> Stashed changes


void Win32_PrintAllAdapterIPAddresses();

int onExit(int exitcode)
{
	server.Release();
	system("pause");
	exit(exitcode);
}

<<<<<<< Updated upstream
=======
void InitializeServer() {
	mazeGen = new MazeGenerator();
	mazeRen = nullptr;

	srand(93225);
}

void UpdateServer(float dt) {
	if (!hazards.empty()) {
		for (auto i = hazards.begin(); i != hazards.end(); i++)
		{
			(*i)->Updata(dt);
		}
	}
}

void  SendPacket() {
	
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

>>>>>>> Stashed changes
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

	printf("Server Initiated\n");


	Win32_PrintAllAdapterIPAddresses();

	timer.GetTimedMS();
	while (true)
	{
		float dt = timer.GetTimedMS() * 0.001f;
		accum_time += dt;
		rotation += 0.5f * PI * dt;

		//Handle All Incoming Packets and Send any enqued packets
		server.ServiceNetwork(dt, [&](const ENetEvent& evnt)
		{
			switch (evnt.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				printf("- New Client Connected\n");
				break;

			case ENET_EVENT_TYPE_RECEIVE:
<<<<<<< Updated upstream
				printf("\t Client %d says: %s\n", evnt.peer->incomingPeerID, evnt.packet->data);
=======
			{
				//printf("\t Client %d says: %s\n", evnt.peer->incomingPeerID, evnt.packet->data);
				float dtp = clients[evnt.peer->incomingPeerID].timer.GetTimedMS()/1000.f;

				if (clients[evnt.peer->incomingPeerID].state == ServerState::WaitingMazeParameter)
				{
					PacketFlag pf;
					memcpy(&pf, evnt.packet->data, sizeof(PacketFlag));
					unsigned offset = sizeof(PacketFlag);

					if (pf == PacketFlag::MazeParam) {
						MazeParameter mp;
						memcpy(&mp, evnt.packet->data + offset, sizeof(MazeParameter));

						//Generating maze
						mazeGen->Generate(mp.size, mp.density);
						mazeRen = new MazeRenderer(mazeGen);
						unsigned flatSize = mazeRen->GetFlatMazeSize();
						unsigned numWalls = mazeRen->GetNumOfWalls();

						//Sending data
						char* data = new char[sizeof(PacketFlag) + sizeof(unsigned) * 2 + sizeof(bool)*flatSize*flatSize];

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
						enet_peer_send(evnt.peer, 0, packet);

						delete[] data;

						clients[evnt.peer->incomingPeerID].state = ServerState::WaitingStartGoal;
					}
				}

				//Waiting client send start and goal position
				else if (clients[evnt.peer->incomingPeerID].state == ServerState::WaitingStartGoal)
				{
					PacketFlag pf;
					memcpy(&pf, evnt.packet->data, sizeof(PacketFlag));
					unsigned offset = sizeof(PacketFlag);

					if (pf == PacketFlag::MazeStartGoal)
					{
						Vector2 start_pos, goal_pos;
						memcpy(&start_pos, evnt.packet->data + offset, sizeof(Vector2));
						offset += sizeof(Vector2);

						memcpy(&goal_pos, evnt.packet->data + offset, sizeof(Vector2));
						offset += sizeof(Vector2);

						mazeGen->SetStartGoal(start_pos, goal_pos);

						clients[evnt.peer->incomingPeerID].startPos = start_pos;
						clients[evnt.peer->incomingPeerID].goalPos = goal_pos;

						GraphNode* startNode = mazeGen->GetStartNode();
						GraphNode* goalNode = mazeGen->GetGoalNode();

						SearchAStar as_searcher;

						as_searcher.FindBestPath(startNode, goalNode);

						clients[evnt.peer->incomingPeerID].path = as_searcher.GetFinalPath();

						char* data = new char[sizeof(PacketFlag) + sizeof(unsigned) + sizeof(float)*clients[evnt.peer->incomingPeerID].path.size() * 3];

						PacketFlag pf = PacketFlag::MazePath;
						memcpy(data, &pf, sizeof(PacketFlag));
						unsigned offset = sizeof(PacketFlag);

						unsigned listSize = clients[evnt.peer->incomingPeerID].path.size();
						memcpy(data + offset, &listSize, sizeof(unsigned));
						offset += sizeof(unsigned);

						float temp;
						for (auto j = clients[evnt.peer->incomingPeerID].path.begin(); j != clients[evnt.peer->incomingPeerID].path.end(); ++j)
						{
							temp = (*j)->_pos.x;
							memcpy(data + offset, &temp, sizeof(unsigned));
							offset += sizeof(float);

							temp = (*j)->_pos.y;
							memcpy(data + offset, &temp, sizeof(unsigned));
							offset += sizeof(float);

							temp = (*j)->_pos.z;
							memcpy(data + offset, &temp, sizeof(unsigned));
							offset += sizeof(float);
						}

						ENetPacket* packet = enet_packet_create(data, sizeof(PacketFlag) + sizeof(float)*clients[evnt.peer->incomingPeerID].path.size() * 3, 0);
						enet_peer_send(evnt.peer, 0, packet);

						delete[] data;

						clients[evnt.peer->incomingPeerID].state = ServerState::WaitingInstruction;
					}
				}
				else if (clients[evnt.peer->incomingPeerID].state == ServerState::WaitingInstruction)
				{
					for (size_t i = 0; i < numOfHazards; i++)
					{
						Hazard* hazard = new Hazard(mazeGen);
						hazard->SetTarget(&clients[evnt.peer->incomingPeerID].currentPos);

						hazards.push_back(hazard);
					}

					PacketFlag pf;
					memcpy(&pf, evnt.packet->data, sizeof(PacketFlag));

					if (pf == PacketFlag::CreateAvator)
					{
						clients[evnt.peer->incomingPeerID].currentPos = clients[evnt.peer->incomingPeerID].startPos;
						clients[evnt.peer->incomingPeerID].state = ServerState::SendingPosition;
					}
				}

				//Sending Position
				else if (clients[evnt.peer->incomingPeerID].state == ServerState::SendingPosition) {

					//creating position packet
					//packet:
					//-------
					//Flag
					//-------
					//Avator position
					//-------
					//Number of hazards
					//-------
					//Hazard positions
					//-------
					printf("client %d in position (%.2f,%.2f)\n", evnt.peer->incomingPeerID, clients[evnt.peer->incomingPeerID].currentPos.x, clients[evnt.peer->incomingPeerID].currentPos.y);

					size_t numOfHazards = hazards.size();

					char* data = new char[sizeof(PacketFlag) +sizeof(size_t)+ sizeof(Vector2)*(numOfHazards+1)];

					PacketFlag pf = PacketFlag::AvatorPosition;
					memcpy(data, &pf, sizeof(PacketFlag));
					unsigned offset = sizeof(PacketFlag);

					memcpy(data + offset, &clients[evnt.peer->incomingPeerID].currentPos, sizeof(Vector2));
					offset += sizeof(Vector2);
					
					memcpy(data + offset, &numOfHazards, sizeof(size_t));
					offset += sizeof(size_t);

					for (auto i = hazards.begin(); i != hazards.end(); ++i) {
						memcpy(data + offset, &(*i)->GetPosition(), sizeof(Vector2));
						offset += sizeof(Vector2);
					}

					ENetPacket* packet = enet_packet_create(data, offset, 0);
					enet_peer_send(evnt.peer, 0, packet);

					delete[] data;

					//update position
					clients[evnt.peer->incomingPeerID].currentPos;
					if (clients[evnt.peer->incomingPeerID].path.empty())
					{
						//clients[evnt.peer->incomingPeerID].state = ServerState::Idle;
					}
					else
					{
						Vector2 nextCheckPoint = Vector2(clients[evnt.peer->incomingPeerID].path.front()->_pos.x, clients[evnt.peer->incomingPeerID].path.front()->_pos.y);
						Vector2 direction = nextCheckPoint - clients[evnt.peer->incomingPeerID].currentPos;

						if (direction.Length() > moving_speed*dtp) {
							//haven't reached next check point 
							direction.Normalise();

							clients[evnt.peer->incomingPeerID].currentPos = clients[evnt.peer->incomingPeerID].currentPos + direction*moving_speed*dtp;
						}
						else
						{
							clients[evnt.peer->incomingPeerID].currentPos = nextCheckPoint;
							clients[evnt.peer->incomingPeerID].path.pop_front();
						}
					}
				}

				//enet_peer_send

>>>>>>> Stashed changes
				enet_packet_destroy(evnt.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("- Client %d has disconnected.\n", evnt.peer->incomingPeerID);
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
			Vector3 pos = Vector3(
				cos(rotation) * 2.0f,
				1.5f,
				sin(rotation) * 2.0f);

			//Create the packet and broadcast it (unreliable transport) to all clients
			ENetPacket* position_update = enet_packet_create(&pos, sizeof(Vector3), 0);
			enet_host_broadcast(server.m_pNetwork, 0, position_update);
		}

		Sleep(0);
	}

	system("pause");
	server.Release();
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