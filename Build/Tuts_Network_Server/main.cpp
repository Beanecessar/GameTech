
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

#include <iomanip>
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
#include "NetworkDataset.h"
#include "Hazard.h"

class SearchAStar;

//Needed to get computer adapter IPv4 addresses via windows
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")


#define SERVER_PORT 1234
#define UPDATE_TIMESTEP 1.0f //Output network traffic per second

NetworkBase server;
GameTimer timer;
MazeGenerator* mazeGen;
MazeRenderer* mazeRen;
vector<Hazard*> hazards;
std::vector<ClientData> clients;
float moving_speed = 1.f;
float accum_time = 0.0f;
size_t numOfHazards = 10;


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

void UpdateServer(float dt) {
	if (!hazards.empty()) {
		for (auto i = hazards.begin(); i != hazards.end(); i++)
		{
			(*i)->Updata(dt);
		}
	}
}


void RandomizeHazardsTarget() {
	//count active player
	vector<unsigned> activePlayerID;
	for (auto i = clients.begin(); i != clients.end(); ++i)
	{
		if ((*i).state == ServerState::SendingPosition)
		{
			activePlayerID.push_back((*i).ID);
		}
	}

	//randomize target
	for (auto i = hazards.begin(); i != hazards.end(); ++i)
	{
		unsigned targetID;
		if (activePlayerID.empty())
			targetID = 0;
		else
			targetID = rand() % (activePlayerID.size());
		(*i)->SetTarget(&clients[targetID].currentPos);
	}
}

void UpdatePlayerPosition(float dt) 
{
	//count active player
	vector<unsigned> activePlayerID;
	for (auto i = clients.begin(); i != clients.end(); ++i)
	{
		if ((*i).state == ServerState::SendingPosition)
		{
			activePlayerID.push_back((*i).ID);
		}
	}

	//updating position
	for (auto i = activePlayerID.begin(); i != activePlayerID.end(); ++i)
	{
		if (clients[(*i)].path.empty())
		{
			clients[*i].velocityNormal = Vector2(0, 0);
			//clients[(*i)].state = ServerState::Idle;
		}
		else
		{
			Vector2 nextCheckPoint = Vector2(clients[(*i)].path.front()->_pos.x, clients[(*i)].path.front()->_pos.y);
			Vector2 direction = nextCheckPoint - clients[(*i)].currentPos;

			if (direction.Length() > moving_speed*dt) {
				//haven't reached next check point 
				direction.Normalise();
				clients[(*i)].velocityNormal = direction;
				clients[(*i)].currentPos = clients[(*i)].currentPos + direction*moving_speed*dt;
			}
			else
			{
				direction.Normalise();
				clients[(*i)].velocityNormal = direction;
				clients[(*i)].currentPos = nextCheckPoint;
				clients[(*i)].path.pop_front();
			}
		}
	}

	//collision detection

	//player-player
	if(activePlayerID.size() > 1)
		for (auto i = activePlayerID.begin(); i != activePlayerID.end()-1; ++i)
		{
			for (auto j = i+1; j != activePlayerID.end(); ++j)
			{
				Vector2 normal = clients[*i].currentPos - clients[*j].currentPos;
				float distance = normal.Length();

				if (distance<0.8f)
				{
					normal.Normalise();
					float penetration = (0.8f - distance) / 2;
					
					clients[*i].currentPos = clients[*i].currentPos - clients[*i].velocityNormal *penetration *std::abs(Vector2::Dot(clients[*i].velocityNormal,normal));
					clients[*j].currentPos = clients[*j].currentPos - clients[*j].velocityNormal *penetration*std::abs(Vector2::Dot(clients[*j].velocityNormal, normal));
				}
			}
		}


	//player-stone
	for (auto i = activePlayerID.begin(); i != activePlayerID.end(); ++i)
	{
		for (auto j = clients.begin(); j != clients.end(); ++j)
		{
			if ((*j).stonePos.x >= 0)
			{
				Vector2 normal = clients[*i].currentPos - (*j).stonePos;
				float distance = normal.Length();

				if (distance < 0.8f)
				{
					normal.Normalise();
					float penetration = 0.8f - distance;

					clients[*i].currentPos = clients[*i].currentPos - clients[*i].velocityNormal *penetration *std::abs(Vector2::Dot(clients[*i].velocityNormal, normal));

				}
			}
		}
	}
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

		UpdateServer(dt);
		//cout << dt << endl;

		//Handle All Incoming Packets and Send any enqued packets
		server.ServiceNetwork(dt, [&](const ENetEvent& evnt)
		{
			switch (evnt.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				if (evnt.peer->incomingPeerID + 1 > clients.size())
				{
					clients.resize(evnt.peer->incomingPeerID + 1);
				}
				clients[evnt.peer->incomingPeerID] = (ClientData());
				clients[evnt.peer->incomingPeerID].ID = evnt.peer->incomingPeerID;

				printf("- New Client Connected\n");
				break;



			case ENET_EVENT_TYPE_RECEIVE:

			{
				//printf("\t Client %d says: %s\n", evnt.peer->incomingPeerID, evnt.packet->data);

				if (evnt.packet->dataLength >= sizeof(PacketFlag)) {
					//Loading packet flag
					PacketFlag pf;
					memcpy(&pf, evnt.packet->data, sizeof(PacketFlag));
					unsigned offset = sizeof(PacketFlag);

					cout << evnt.peer->incomingPeerID << "-> State: " << clients[evnt.peer->incomingPeerID].state << ", Packet flag: " << pf << endl;

					if (pf == PacketFlag::MazeParam) {
						//Reinitializing
						hazards.clear();

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

						ENetPacket* packet = enet_packet_create(data, offset, 0);
						enet_host_broadcast(server.m_pNetwork, 0, packet);

						for(auto i=clients.begin();i!=clients.end();++i)
						{
							(*i).state = ServerState::WaitingStartGoal;
						}

						delete[] data;
					}

					if (pf == PacketFlag::StonePosition) {
						memcpy(&clients[evnt.peer->incomingPeerID].stonePos, evnt.packet->data + offset, sizeof(Vector2));
						offset += sizeof(Vector2);
					}

					//Waiting client send start and goal position
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

							list<const GraphNode*> rowPath = as_searcher.GetFinalPath();
							clients[evnt.peer->incomingPeerID].path.clear();

							//String-pulling
							if (!rowPath.empty())
							{
								Vector3 pos = (*rowPath.begin())->_pos;
								clients[evnt.peer->incomingPeerID].path.push_back(*rowPath.begin());
								for (auto i = ++(rowPath.begin()); i != rowPath.end(); ++i)
								{
									if (i != rowPath.end())
									{
										if ((*i)->_pos.x != pos.x && (*i)->_pos.y != pos.y)
										{
											clients[evnt.peer->incomingPeerID].path.push_back(*(--i));
											pos = (*i)->_pos;
										}
									}
								}
								clients[evnt.peer->incomingPeerID].path.push_back(*(--rowPath.end()));
							}

							char* data = new char[sizeof(PacketFlag) + sizeof(unsigned) + sizeof(float)*clients[evnt.peer->incomingPeerID].path.size() * 3];

							PacketFlag pf = PacketFlag::MazePath;
							memcpy(data, &pf, sizeof(PacketFlag));
							unsigned offset = sizeof(PacketFlag);

							unsigned listSize = clients[evnt.peer->incomingPeerID].path.size();
							memcpy(data + offset, &listSize, sizeof(unsigned));
							offset += sizeof(unsigned);

							if (!clients[evnt.peer->incomingPeerID].path.empty()) 
							{
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
							}

							ENetPacket* packet = enet_packet_create(data, sizeof(PacketFlag) + sizeof(float)*clients[evnt.peer->incomingPeerID].path.size() * 3, 0);
							enet_peer_send(evnt.peer, 0, packet);

							delete[] data;

							clients[evnt.peer->incomingPeerID].state = ServerState::WaitingInstruction;
						}
					if (clients[evnt.peer->incomingPeerID].state == ServerState::WaitingInstruction)
					{
						if (pf == PacketFlag::CreateAvator)
						{
							if (hazards.empty())
							{
								for (size_t i = 0; i < numOfHazards; i++)
								{
									Hazard* hazard = new Hazard(mazeGen);

									hazards.push_back(hazard);
								}
							}

							RandomizeHazardsTarget();

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
						//Avator position & velocity
						//-------
						//Number of hazards
						//-------
						//Hazard positions & velocity
						//-------
						//Number of other players
						//-------
						//other player positions & velocity
						//-------
						//Number of stones
						//-------
						//stone positions

						//printf("client %d in position (%.2f,%.2f)\n", evnt.peer->incomingPeerID, clients[evnt.peer->incomingPeerID].currentPos.x, clients[evnt.peer->incomingPeerID].currentPos.y);

						size_t numOfHazards = hazards.size();

						//Counting active plays
						size_t numOfPlays = 0;
						vector<unsigned> otherPlayerID;
						for (auto i = clients.begin(); i != clients.end(); ++i)
						{
							if ((*i).state == ServerState::SendingPosition)
							{
								numOfPlays += 1;
								if ((*i).ID != evnt.peer->incomingPeerID)
									otherPlayerID.push_back((*i).ID);
							}
						}

						//Counting stones
						size_t numOfStones = 0;
						vector<Vector2> stones;
						for (auto i = clients.begin(); i != clients.end(); ++i)
						{
							if ((*i).stonePos.x>=0)
							{
								numOfStones += 1;
								stones.push_back((*i).stonePos);
							}
						}

						char* data = new char[sizeof(PacketFlag) + sizeof(size_t)*3 + sizeof(Vector2)*(numOfHazards + numOfPlays)*2 + sizeof(Vector2)*numOfStones];

						PacketFlag pf = PacketFlag::PlayerHazardPositions;
						memcpy(data, &pf, sizeof(PacketFlag));
						unsigned offset = sizeof(PacketFlag);

						memcpy(data + offset, &clients[evnt.peer->incomingPeerID].currentPos, sizeof(Vector2));
						offset += sizeof(Vector2);

						Vector2 velocity = clients[evnt.peer->incomingPeerID].velocityNormal*moving_speed;
						memcpy(data + offset, &velocity, sizeof(Vector2));
						offset += sizeof(Vector2);

						memcpy(data + offset, &numOfHazards, sizeof(size_t));
						offset += sizeof(size_t);

						for (auto i = hazards.begin(); i != hazards.end(); ++i) {
							//position
							memcpy(data + offset, &(*i)->GetPosition(), sizeof(Vector2));
							offset += sizeof(Vector2);

							//volecity
							memcpy(data + offset, &(*i)->GetVelocity(), sizeof(Vector2));
							offset += sizeof(Vector2);
						}

						numOfPlays -= 1;
						memcpy(data + offset, &numOfPlays, sizeof(size_t));
						offset += sizeof(size_t);

						for (auto i = otherPlayerID.begin(); i!= otherPlayerID.end(); ++i) {
							//position
							memcpy(data + offset, &clients[(*i)].currentPos, sizeof(Vector2));
							offset += sizeof(Vector2);

							//velocity
							velocity= clients[(*i)].velocityNormal*moving_speed;
							memcpy(data + offset, &velocity, sizeof(Vector2));
							offset += sizeof(Vector2);
						}

						memcpy(data + offset, &numOfStones, sizeof(size_t));
						offset += sizeof(size_t);

						for (auto i = stones.begin(); i != stones.end(); ++i) {
							//position
							memcpy(data + offset, &(*i), sizeof(Vector2));
							offset += sizeof(Vector2);
						}

						ENetPacket* packet = enet_packet_create(data, offset, 0);
						enet_peer_send(evnt.peer, 0, packet);

						delete[] data;
					}

					//enet_peer_send
					enet_packet_destroy(evnt.packet);
				}
			}

				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("- Client %d has disconnected.\n", evnt.peer->incomingPeerID);
				clients[evnt.peer->incomingPeerID].state = ServerState::Idle;
				RandomizeHazardsTarget();
				break;
			}
		});

		UpdatePlayerPosition(dt);

		//Broadcast update packet to all connected clients at a rate of UPDATE_TIMESTEP updates per second
		if (accum_time >= UPDATE_TIMESTEP)
		{

			//Packet data
			// - At the moment this is just a position update that rotates around the origin of the world
			//   though this can be any variable, structure or class you wish. Just remember that everything 
			//   you send takes up valuable network bandwidth so no sending every PhysicsObject struct each frame ;)
			accum_time = 0.0f;

			cout << "Network traffic:" << endl;
			cout << "Incoming: " << setprecision(2) << server.m_IncomingKb << "Kbps" << endl;
			cout << "Outgoing: " << setprecision(2) << server.m_OutgoingKb << "Kbps" << endl;

			//Create the packet and broadcast it (unreliable transport) to all clients

			//ENetPacket* packet = UpdatePacket();

			//enet_host_broadcast(server.m_pNetwork, 0, packet);
		}

		Sleep(0);
	}

	system("pause");
	server.Release();
}
