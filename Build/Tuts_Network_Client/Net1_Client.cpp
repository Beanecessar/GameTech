/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Server" at the same time.
- To do this:-
	1. right click on the entire solution (top of the solution exporerer) and go to properties
	2. Go to Common Properties -> Statup Project
	3. Select Multiple Statup Projects
	4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
  and going to Debug->Start New Instance.




This demo scene will demonstrate a very simple network example, with a single server
and multiple clients. The client will attempt to connect to the server, and say "Hellooo!" 
if it successfully connects. The server, will continually broadcast a packet containing a 
Vector3 position to all connected clients informing them where to place the server's player.

This designed as an example of how to setup networked communication between clients, it is
by no means the optimal way of handling a networked game (sending position updates at xhz).
If your interested in this sort of thing, I highly recommend finding a good book or an
online tutorial as there are many many different ways of handling networked game updates
all with varying pitfalls and benefits. In general, the problem always comes down to the
fact that sending updates for every game object 60+ frames a second is just not possible,
so sacrifices and approximations MUST be made. These approximations do result in a sub-optimal
solution however, so most work on networking (that I have found atleast) is on building
a network bespoke communication system that sends the minimal amount of data needed to
produce satisfactory results on the networked peers.


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::: IF YOUR BORED! :::
::::::::::::::::::::::
	1. Try setting up both the server and client within the same Scene (disabling collisions
	on the objects as they now share the same physics engine). This way we can clearly
	see the affect of packet-loss and latency on the network. There is a program called "Clumsy"
	which is found within the root directory of this framework that allows you to inject
	latency/packet loss etc on network. Try messing around with various latency/packet-loss
	values.

	2. Packet Loss
		This causes the object to jump in large (and VERY noticable) gaps from one position to 
		another.

	   A good place to start in compensating for this is to build a buffer and store the
	   last x update packets, now if we miss a packet it isn't too bad as the likelyhood is
	   that by the time we need that position update, we will already have the next position
	   packet which we can use to interpolate that missing data from. The number of packets we
	   will need to backup will be relative to the amount of expected packet loss. This method
	   will also insert additional 'buffer' latency to our system, so we better not make it wait
	   too long.

	3. Latency
	   There is no easy way of solving this, and will have all felt it's punishing effects
	   on all networked games. The way most games attempt to hide any latency is by actually
	   running different games on different machines, these will react instantly to your actions
	   such as shooting which the server will eventually process at some point and tell you if you
	   have hit anything. This makes it appear (client side) like have no latency at all as you
	   moving instantly when you hit forward and shoot when you hit shoot, though this is all smoke
	   and mirrors and the server is still doing all the hard stuff (except now it has to take into account
	   the fact that you shot at time - latency time).

	   This smoke and mirrors approach also leads into another major issue, which is what happens when
	   the instances are desyncrhonised. If player 1 shoots and and player 2 moves at the same time, does
	   player 1 hit player 2? On player 1's screen he/she does, but on player 2's screen he/she gets
	   hit. This leads to issues which the server has to decipher on it's own, this will also happen
	   alot with generic physical elements which will ocasional 'snap' back to it's actual position on 
	   the servers game simulation. This methodology is known as "Dead Reckoning".

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


*//////////////////////////////////////////////////////////////////////////////

#include "Net1_Client.h"
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>
#include <nclgl\OBJMesh.h>
#include <nclgl\Vector2.h>
#include "PacketFlag.h"

const Vector3 status_color3 = Vector3(1.0f, 0.6f, 0.6f);
const Vector4 status_color = Vector4(status_color3.x, status_color3.y, status_color3.z, 1.0f);

Net1_Client::Net1_Client(const std::string& friendly_name)
	: Scene(friendly_name)
	, serverConnection(NULL)
{
}

void Net1_Client::OnInitializeScene()
{
	state = ClientState::WaitingMazeData;
	wallMesh = new OBJMesh(MESHDIR"cube.obj");

	mazeRenderer = nullptr;

	mp.size = 16;
	mp.density = 0.5;

	GLuint whitetex;
	glGenTextures(1, &whitetex);
	glBindTexture(GL_TEXTURE_2D, whitetex);
	unsigned int pixel = 0xFFFFFFFF;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
	glBindTexture(GL_TEXTURE_2D, 0);

	wallMesh->SetTexture(whitetex);

	//Create Ground 
	GameObject* ground = CommonUtils::BuildCuboidObject(
		"Ground",
		Vector3(0.0f, -1.0f, 0.0f),
		Vector3(20.0f, 1.0f, 20.0f),
		false,
		0.0f,
		false,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f));

	this->AddGameObject(ground);

	//Initialize Client Network
	if (network.Initialize(0))
	{
		NCLDebug::Log("Network: Initialized!");

		//Attempt to connect to the server on localhost:1234
		serverConnection = network.ConnectPeer(127, 0, 0, 1, 1234);
		NCLDebug::Log("Network: Attempting to connect to server.");
	}
}

void Net1_Client::OnCleanupScene()
{
	if (mazeRenderer) {
		this->RemoveGameObject(mazeRenderer);
		if (mazeRenderer->GetScene())
			SAFE_DELETE(mazeRenderer)
		else
			mazeRenderer = nullptr;
	}

	Scene::OnCleanupScene();

	SAFE_DELETE(wallMesh);

	avator = nullptr;
	CleanHazards();

	//Send one final packet telling the server we are disconnecting
	// - We are not waiting to resend this, so if it fails to arrive
	//   the server will have to wait until we time out naturally
	enet_peer_disconnect_now(serverConnection, 0);

	//Release network and all associated data/peer connections
	network.Release();
	serverConnection = NULL;

	state = ClientState::WaitingMazeData;
}

void Net1_Client::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);

	//Handle Input
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1))
	{
		mp.size--;
		state = ClientState::WaitingMazeData;
		CleanHazards();
		if (mazeRenderer) {
			this->RemoveGameObject(mazeRenderer);
			if (mazeRenderer->GetScene())
				SAFE_DELETE(mazeRenderer)
			else
				mazeRenderer = nullptr;
		}
		avator = nullptr;
	}else if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))
	{
		mp.size++;
		state = ClientState::WaitingMazeData;
		CleanHazards();
		if (mazeRenderer) {
			this->RemoveGameObject(mazeRenderer);
			if (mazeRenderer->GetScene())
				SAFE_DELETE(mazeRenderer)
			else
				mazeRenderer = nullptr;
		}
		avator = nullptr;
	}

	//Update Network
	auto callback = std::bind(
		&Net1_Client::ProcessNetworkEvent,	// Function to call
		this,								// Associated class instance
		std::placeholders::_1);				// Where to place the first parameter
	network.ServiceNetwork(dt, callback);

	//Dead reckoning
	DeadReckoning(dt);
	//cout << dt <<endl;

	//Update state machine
	UpdateClientStateMachine(dt);

	//Add Debug Information to screen
	uint8_t ip1 = serverConnection->address.host & 0xFF;
	uint8_t ip2 = (serverConnection->address.host >> 8) & 0xFF;
	uint8_t ip3 = (serverConnection->address.host >> 16) & 0xFF;
	uint8_t ip4 = (serverConnection->address.host >> 24) & 0xFF;

// 	NCLDebug::DrawTextWs(box->Physics()->GetPosition() + Vector3(0.f, 0.6f, 0.f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0.f, 0.f, 0.f, 1.f),
// 		"Peer: %u.%u.%u.%u:%u", ip1, ip2, ip3, ip4, serverConnection->address.port);

	NCLDebug::AddStatusEntry(status_color, "Controls");
	NCLDebug::AddStatusEntry(status_color, "	Press 1 to decrease maze size");
	NCLDebug::AddStatusEntry(status_color, "	Press 2 to increase maze size");
	
	NCLDebug::AddStatusEntry(status_color, "Network Traffic");
	NCLDebug::AddStatusEntry(status_color, "    Incoming: %5.2fKbps", network.m_IncomingKb);
	NCLDebug::AddStatusEntry(status_color, "    Outgoing: %5.2fKbps", network.m_OutgoingKb);
}

void Net1_Client::ProcessNetworkEvent(const ENetEvent& evnt)
{
	switch (evnt.type)
	{
	//New connection request or an existing peer accepted our connection request
	case ENET_EVENT_TYPE_CONNECT:
		{
			if (evnt.peer == serverConnection)
			{
				NCLDebug::Log(status_color3, "Network: Successfully connected to server!");

				//Send a 'hello' packet
				char* text_data = "Hellooo!";
				ENetPacket* packet = enet_packet_create(text_data, strlen(text_data) + 1, 0);
				enet_peer_send(serverConnection, 0, packet);
				enet_packet_destroy(packet);
			}	
		}
		break;


	//Server has sent us a new packet
	case ENET_EVENT_TYPE_RECEIVE:
	{
		//Loading packet flag
		if (evnt.packet->dataLength >= sizeof(PacketFlag))
		{
			PacketFlag pf;
			memcpy(&pf, evnt.packet->data, sizeof(PacketFlag));
			unsigned offset = sizeof(PacketFlag);

			cout << "Self-> State: " << state << ", Packet flag: " << pf << endl;

			if (pf == PacketFlag::MazeArray)
			{
				if (mazeRenderer) {
					this->RemoveGameObject(mazeRenderer);
					if (mazeRenderer->GetScene())
						SAFE_DELETE(mazeRenderer)
					else
						mazeRenderer = nullptr;
				}

				memcpy(&md.flat_maze_size, evnt.packet->data + offset, sizeof(unsigned));
				offset += sizeof(unsigned);
				mp.size = (md.flat_maze_size + 1) / 3;

				memcpy(&md.num_walls, evnt.packet->data + offset, sizeof(unsigned));
				offset += sizeof(unsigned);

				md.flat_maze = new bool[md.flat_maze_size * md.flat_maze_size];
				memcpy(md.flat_maze, evnt.packet->data + offset, sizeof(bool)*md.flat_maze_size * md.flat_maze_size);
				offset += sizeof(bool)*md.flat_maze_size * md.flat_maze_size;

				//Creating maze
				mazeRenderer = new MazeRenderer(md.flat_maze_size, md.num_walls, md.flat_maze, wallMesh);

				mazeRenderer->Render()->SetTransform(Matrix4::Scale(Vector3(5.f, 5.0f / float(mp.size), 5.f)) * Matrix4::Translation(Vector3(-0.5f, 0.f, -0.5f)));

				this->AddGameObject(mazeRenderer);

				state = ClientState::CreatingStartGoal;
			}

			if (state == ClientState::WaitingPath)
			{
				if (pf == PacketFlag::MazePath)
				{
					//loading list size
					memcpy(&pathSize, evnt.packet->data + offset, sizeof(unsigned));
					offset += sizeof(unsigned);

					//loading list data
					path.clear();
					Vector3 pathNode;
					float temp;
					for (unsigned i = 0; i < pathSize; ++i)
					{
						memcpy(&temp, evnt.packet->data + offset, sizeof(float));
						pathNode.x = temp;
						offset += sizeof(float);

						memcpy(&temp, evnt.packet->data + offset, sizeof(float));
						pathNode.y = temp;
						offset += sizeof(float);

						memcpy(&temp, evnt.packet->data + offset, sizeof(float));
						pathNode.z = temp;
						offset += sizeof(float);

						path.push_back(pathNode);
					}

					state = ClientState::CreatingAvator;
				}
			}
			else if (state == ClientState::WaitingPosition) {
				if (pf == PacketFlag::PlayerHazardPositions) {
					memcpy(&currentPos, evnt.packet->data + offset, sizeof(Vector2));
					offset += sizeof(Vector2);

					memcpy(&velocity, evnt.packet->data + offset, sizeof(Vector2));
					offset += sizeof(Vector2);

					size_t numOfHazards;
					memcpy(&numOfHazards, evnt.packet->data + offset, sizeof(size_t));
					offset += sizeof(size_t);

					//Recive hazard positions & velocitys
					if (hazards.size()!= numOfHazards || numOfHazards == 0)
					{
						//Initializing hazards
						CleanHazards();
						for (size_t i = 0; i < numOfHazards; i++)
						{
							Vector2 pos;
							memcpy(&pos, evnt.packet->data + offset, sizeof(Vector2));
							offset += sizeof(Vector2);

							Vector2 vel;
							memcpy(&vel, evnt.packet->data + offset, sizeof(Vector2));
							offset += sizeof(Vector2);

							float scalar = 1.f / (float)md.flat_maze_size;
							Vector3 cellpos = Vector3(
								pos.x * 3,
								0.0f,
								pos.y * 3
							) * scalar;
							Vector3 cellsize = Vector3(
								scalar * 2,
								1.0f,
								scalar * 2
							);

							RenderNode* hazardRender = new RenderNode(CommonMeshes::Cube(), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
							hazardRender->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.3f));
							mazeRenderer->Render()->AddChild(hazardRender);

							Character hazard = { hazardRender ,pos, vel };
							hazards.push_back(hazard);
						}
					}
					else
					{
						for (size_t i = 0; i < numOfHazards; i++)
						{
							memcpy(&hazards[i].position, evnt.packet->data + offset, sizeof(Vector2));
							offset += sizeof(Vector2);

							memcpy(&hazards[i].velocity, evnt.packet->data + offset, sizeof(Vector2));
							offset += sizeof(Vector2);
						}
					}

					size_t numOfOtherPlayers;
					memcpy(&numOfOtherPlayers, evnt.packet->data + offset, sizeof(size_t));
					offset += sizeof(size_t);

					//Recive other player positions & velocity
					if (otherPlayers.size() != numOfOtherPlayers || numOfOtherPlayers == 0)
					{
						//Initializing other players
						CleanOtherPlayers();
						for (size_t i = 0; i < numOfOtherPlayers; i++)
						{
							Vector2 pos;
							memcpy(&pos, evnt.packet->data + offset, sizeof(Vector2));
							offset += sizeof(Vector2);

							Vector2 vel;
							memcpy(&vel, evnt.packet->data + offset, sizeof(Vector2));
							offset += sizeof(Vector2);

							for (size_t i = 0; i < numOfOtherPlayers; i++)
							{
								float scalar = 1.f / (float)md.flat_maze_size;
								Vector3 cellpos = Vector3(
									pos.x * 3,
									0.0f,
									pos.y * 3
								) * scalar;
								Vector3 cellsize = Vector3(
									scalar * 2,
									1.0f,
									scalar * 2
								);

								RenderNode* playerRender = new RenderNode(CommonMeshes::Cube(), Vector4(1.0f, 1.0f, 0.0f, 1.0f));
								playerRender->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.4f));
								mazeRenderer->Render()->AddChild(playerRender);

								Character player = { playerRender,pos,vel };

								otherPlayers.push_back(player);
							}
						}
					}
					else
					{
						for (size_t i = 0; i < numOfOtherPlayers; i++)
						{
							memcpy(&otherPlayers[i].position, evnt.packet->data + offset, sizeof(Vector2));
							offset += sizeof(Vector2);

							memcpy(&otherPlayers[i].velocity, evnt.packet->data + offset, sizeof(Vector2));
							offset += sizeof(Vector2);
						}
					}

					size_t numOfStones;
					memcpy(&numOfStones, evnt.packet->data + offset, sizeof(size_t));
					offset += sizeof(size_t);

					//Recive stones
					if (stones.size() != numOfStones || numOfStones == 0)
					{
						//Initializing stones
						CleanStones();
						for (size_t i = 0; i < numOfStones; i++)
						{
							Vector2 pos;
							memcpy(&pos, evnt.packet->data + offset, sizeof(Vector2));
							offset += sizeof(Vector2);

							for (size_t i = 0; i < numOfStones; i++)
							{
								float scalar = 1.f / (float)md.flat_maze_size;
								Vector3 cellpos = Vector3(
									pos.x * 3,
									0.0f,
									pos.y * 3
								) * scalar;
								Vector3 cellsize = Vector3(
									scalar * 2,
									1.0f,
									scalar * 2
								);

								RenderNode* stoneRender = new RenderNode(CommonMeshes::Cube(), Vector4(1.0f, 0.6f, 0.4f, 1.0f));
								stoneRender->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.4f));
								mazeRenderer->Render()->AddChild(stoneRender);

								Character stone = { stoneRender,pos,Vector2(0,0) };

								stones.push_back(stone);
							}
						}
					}
					else
					{
						for (size_t i = 0; i < numOfStones; i++)
						{
							memcpy(&stones[i].position, evnt.packet->data + offset, sizeof(Vector2));
							offset += sizeof(Vector2);
						}
					}
				}
			}
			else
			{
				//NCLERROR("Recieved Invalid Network Packet!");
			}

		}
	}
		
		break;


	//Server has disconnected
	case ENET_EVENT_TYPE_DISCONNECT:
		{
			NCLDebug::Log(status_color3, "Network: Server has disconnected!");
		}
		break;
	}
}

void Net1_Client::UpdateClientStateMachine(float dt) {
	if (mazeRenderer&&mazeRenderer->IsStoneRenewed)
	{
		mazeRenderer->IsStoneRenewed = false;

		char* data = new char[sizeof(PacketFlag)+sizeof(Vector2)];
		unsigned offset = 0;
		PacketFlag pf = PacketFlag::StonePosition;
		memcpy(data, &pf, sizeof(PacketFlag));
		offset += sizeof(PacketFlag);

		Vector2 pos = mazeRenderer->GetStonePosition();
		memcpy(data + offset, &pos, sizeof(Vector2));
		offset += sizeof(Vector2);

		packet = enet_packet_create(data, offset, 0);
		enet_peer_send(serverConnection, 0, packet);
	}

	if (mazeRenderer&&mazeRenderer->IsGoalRenewed && state!= ClientState::CreatingStartGoal)
	{
		Vector2 startPos;
		startPos.x = (unsigned)(currentPos.x + 0.5);
		startPos.y = (unsigned)(currentPos.y + 0.5);

		mazeRenderer->SetStartPosition(startPos);

		CleanHazards();
		mazeRenderer->IsGoalRenewed = false;
		state = ClientState::CreatingStartGoal;
	}

	switch (state)
	{
	case ClientState::WaitingMazeData:
	{
		char* data = new char[sizeof(PacketFlag) + sizeof(MazeParameter)];

		PacketFlag pf = PacketFlag::MazeParam;
		memcpy(data, &pf, sizeof(PacketFlag));
		unsigned offset = sizeof(PacketFlag);

		memcpy(data + offset, &mp, sizeof(MazeParameter));
		offset += sizeof(MazeParameter);

		packet = enet_packet_create(data, sizeof(PacketFlag) + sizeof(MazeParameter), 0);
		enet_peer_send(serverConnection, 0, packet);

		delete[] data;
	}
	break;

	case ClientState::CreatingStartGoal:
	{
		Vector2 start_position = mazeRenderer->GetStartPosition();
		Vector2 goal_position = mazeRenderer->GetGoalPosition();

		if (start_position.x >= 0 && goal_position.x >= 0)
		{
			mazeRenderer->IsGoalRenewed = false;

			currentPos = start_position;

			char* data = new char[sizeof(PacketFlag) + sizeof(Vector2) * 2];

			PacketFlag pf = PacketFlag::MazeStartGoal;
			memcpy(data, &pf, sizeof(PacketFlag));
			unsigned offset = sizeof(PacketFlag);

			memcpy(data + offset, &start_position, sizeof(Vector2));
			offset += sizeof(Vector2);

			memcpy(data + offset, &goal_position, sizeof(Vector2));
			offset += sizeof(Vector2);

			packet = enet_packet_create(data, sizeof(PacketFlag) + sizeof(Vector2) * 2, 0);

			enet_peer_send(serverConnection, 0, packet);

			delete[] data;

			state = ClientState::WaitingPath;
		}
	}
	break;

	case ClientState::WaitingPath:
		if (packet)
		{
			enet_peer_send(serverConnection, 0, packet);
		}
		break;

	case ClientState::CreatingAvator:
	{
		float scalar = 1.f / (float)md.flat_maze_size;
		Vector3 cellpos = Vector3(
			currentPos.x * 3,
			0.0f,
			currentPos.y * 3
		) * scalar;
		Vector3 cellsize = Vector3(
			scalar * 2,
			1.0f,
			scalar * 2
		);

		if(!avator)
			avator = new RenderNode(CommonMeshes::Cube(), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		avator->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
		mazeRenderer->Render()->AddChild(avator);

		state = ClientState::WaitingPosition;
	}
	break;
	case ClientState::WaitingPosition:
	{
		float scalar = 1.f / (float)md.flat_maze_size;
		Vector3 curpos = Vector3(
			currentPos.x * 3,
			0.0f,
			currentPos.y * 3
		) * scalar;
		Vector3 cellsize = Vector3(
			scalar * 2,
			1.0f,
			scalar * 2
		);

		avator->SetTransform(Matrix4::Translation(curpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));

		if (!hazards.empty())
		{
			//Update hazard positions
			for (auto i = hazards.begin(); i != hazards.end(); i++)
			{
				Vector3 hazpos = Vector3(
					(*i).position.x * 3,
					0.0f,
					(*i).position.y * 3
				) * scalar;

				(*i).render->SetTransform(Matrix4::Translation(hazpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.3f));
			}
		}

		if (!otherPlayers.empty())
		{
			//Update other player positions
			for (auto i = otherPlayers.begin(); i != otherPlayers.end(); i++)
			{
				Vector3 oppos = Vector3(
					(*i).position.x * 3,
					0.0f,
					(*i).position.y * 3
				) * scalar;

				(*i).render->SetTransform(Matrix4::Translation(oppos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.4f));
			}
		}

		if (!stones.empty())
		{
			//Update other player positions
			for (auto i = stones.begin(); i != stones.end(); i++)
			{
				Vector3 stnpos = Vector3(
					(*i).position.x * 3,
					0.0f,
					(*i).position.y * 3
				) * scalar;

				(*i).render->SetTransform(Matrix4::Translation(stnpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.4f));
			}
		}

		mazeRenderer->DrawPath(path, mp.size, pathSize, 1.0f / mp.size);

		PacketFlag pf = PacketFlag::CreateAvator;

		packet = enet_packet_create(&pf, sizeof(PacketFlag), 0);

		enet_peer_send(serverConnection, 0, packet);
	}
	break;
	default:
		break;
	}
}

void Net1_Client::CleanHazards() {
	if (mazeRenderer)
	{
		for (auto i = hazards.begin(); i != hazards.end(); ++i)
		{
			mazeRenderer->Render()->RemoveChild((*i).render);
			delete (*i).render;
		}
	}
	
	hazards.clear();
}

void Net1_Client::CleanOtherPlayers() {
	if (mazeRenderer)
	{
		for (auto i = otherPlayers.begin(); i != otherPlayers.end(); ++i)
		{
			mazeRenderer->Render()->RemoveChild((*i).render);
			delete (*i).render;
		}
	}

	otherPlayers.clear();
}

void Net1_Client::CleanStones() {
	if (mazeRenderer)
	{
		for (auto i = stones.begin(); i != stones.end(); ++i)
		{
			mazeRenderer->Render()->RemoveChild((*i).render);
			delete (*i).render;
		}
	}

	stones.clear();
}

void Net1_Client::DeadReckoning(float dt) {
	//player
	currentPos += velocity*dt;

	//hazards
	if (!hazards.empty())
		for (auto i = hazards.begin(); i != hazards.end(); ++i)
			(*i).position += (*i).velocity*dt;

	//other players
	if (!otherPlayers.empty())
		for (auto i = otherPlayers.begin(); i != otherPlayers.end(); ++i)
			(*i).position += (*i).velocity*dt;
}