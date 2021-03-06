#include <ncltech\PhysicsEngine.h>
#include <ncltech\SceneManager.h>
#include <nclgl\Window.h>
#include <nclgl\NCLDebug.h>
#include <nclgl\PerfTimer.h>

#include "TestScene.h"
#include "EmptyScene.h"

const Vector4 status_colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
const Vector4 status_colour_header = Vector4(0.8f, 0.9f, 1.0f, 1.0f);

bool show_perf_metrics = false;
PerfTimer timer_total, timer_physics, timer_update, timer_render;
uint shadowCycleKey = 4;


// Program Deconstructor
//  - Releases all global components and memory
//  - Optionally prints out an error message and
//    stalls the runtime if requested.
void Quit(bool error = false, const std::string &reason = "") {
	//Release Singletons
	SceneManager::Release();
	PhysicsEngine::Release();
	GraphicsPipeline::Release();
	Window::Destroy();

	//Show console reason before exit
	if (error) {
		std::cout << reason << std::endl;
		system("PAUSE");
		exit(-1);
	}
}


// Program Initialise
//  - Generates all program wide components and enqueues all scenes
//    for the SceneManager to display
void Initialize()
{
	//Initialise the Window
	if (!Window::Initialise("Game Technologies", 1280, 800, false))
		Quit(true, "Window failed to initialise!");

	//Initialize Renderer
	GraphicsPipeline::Instance();

	//Initialise the PhysicsEngine
	PhysicsEngine::Instance();

	//Enqueue All Scenes
	SceneManager::Instance()->EnqueueScene(new TestScene("GameTech #1 - Framework Sandbox!"));
	SceneManager::Instance()->EnqueueScene(new EmptyScene("GameTech #2 - Peace and quiet"));
	SceneManager::Instance()->EnqueueScene(new EmptyScene("GameTech #3 - More peace and quiet"));
}

// Print Debug Info
//  - Prints a list of status entries to the top left
//    hand corner of the screen each frame.
void PrintStatusEntries()
{
	//Print Engine Options
	NCLDebug::AddStatusEntry(status_colour_header, "NCLTech Settings");
	NCLDebug::AddStatusEntry(status_colour, "     Physics Engine: %s (Press P to toggle)", PhysicsEngine::Instance()->IsPaused() ? "Paused  " : "Enabled ");
	NCLDebug::AddStatusEntry(status_colour, "     Broadphase Detection: %s (Press B to toggle)", PhysicsEngine::Instance()->IsBroadphase() ? "Enabled " : "Disabled ");
	NCLDebug::AddStatusEntry(status_colour, "     Monitor V-Sync: %s (Press V to toggle)", GraphicsPipeline::Instance()->GetVsyncEnabled() ? "Enabled " : "Disabled");
	NCLDebug::AddStatusEntry(status_colour, "");

	//Print Control Settings
	NCLDebug::AddStatusEntry(status_colour_header, "Controls");
	NCLDebug::AddStatusEntry(status_colour, "     J: Launch a sphere from camera position");
	NCLDebug::AddStatusEntry(status_colour, "");

	//Print Debug Draw
	uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();
	NCLDebug::AddStatusEntry(status_colour, "--- Debug Draw ---");
	NCLDebug::AddStatusEntry(status_colour, "Subspace Frame       : %s (Press U to toggle)", (drawFlags & DEBUGDRAW_FLAGS_SUBSPACE) ? "Enabled " : "Disabled");
	NCLDebug::AddStatusEntry(status_colour, "");

	//Print Debug Info
	NCLDebug::AddStatusEntry(status_colour, "--- Debug Info ---");
	NCLDebug::AddStatusEntry(status_colour, "Collision Pairs       : %d", PhysicsEngine::Instance()->GetCollisionPairsNum());
	NCLDebug::AddStatusEntry(status_colour, "");

	//Print Performance
	timer_total.PrintOutputToStatusEntry(status_colour, "Frame Total     :");
	timer_physics.PrintOutputToStatusEntry(status_colour, "Physics Total   :");
	PhysicsEngine::Instance()->PrintPerformanceTimers(status_colour);
	NCLDebug::AddStatusEntry(status_colour, "");

	//Print Current Scene Name
	NCLDebug::AddStatusEntry(status_colour_header, "[%d/%d]: %s",
		SceneManager::Instance()->GetCurrentSceneIndex() + 1,
		SceneManager::Instance()->SceneCount(),
		SceneManager::Instance()->GetCurrentScene()->GetSceneName().c_str()
		);
	NCLDebug::AddStatusEntry(status_colour, "     \x01 T/Y to cycle or R to reload scene");

	//Print Performance Timers
	NCLDebug::AddStatusEntry(status_colour, "     FPS: %5.2f  (Press G for %s info)", 1000.f / timer_total.GetAvg(), show_perf_metrics ? "less" : "more");
	if (show_perf_metrics)
	{
		timer_total.PrintOutputToStatusEntry(status_colour, "          Total Time     :");
		timer_update.PrintOutputToStatusEntry(status_colour, "          Scene Update   :");
		timer_physics.PrintOutputToStatusEntry(status_colour, "          Physics Update :");
		timer_render.PrintOutputToStatusEntry(status_colour, "          Render Scene   :");
	}
	NCLDebug::AddStatusEntry(status_colour, "");
}


// Process Input
//  - Handles all program wide keyboard inputs for
//    things such toggling the physics engine and 
//    cycling through scenes.
void HandleKeyboardInputs()
{
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P))
		PhysicsEngine::Instance()->SetPaused(!PhysicsEngine::Instance()->IsPaused());

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_B))
		PhysicsEngine::Instance()->SetBroadphase(!PhysicsEngine::Instance()->IsBroadphase());

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_V))
		GraphicsPipeline::Instance()->SetVsyncEnabled(!GraphicsPipeline::Instance()->GetVsyncEnabled());

	uint sceneIdx = SceneManager::Instance()->GetCurrentSceneIndex();
	uint sceneMax = SceneManager::Instance()->SceneCount();
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Y))
		SceneManager::Instance()->JumpToScene((sceneIdx + 1) % sceneMax);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_T))
		SceneManager::Instance()->JumpToScene((sceneIdx == 0 ? sceneMax : sceneIdx) - 1);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_R))
		SceneManager::Instance()->JumpToScene(sceneIdx);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_G))
		show_perf_metrics = !show_perf_metrics;

	uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_U))
		drawFlags ^= DEBUGDRAW_FLAGS_SUBSPACE;
	PhysicsEngine::Instance()->SetDebugDrawFlags(drawFlags);

	//Launch a sphere to current scene.
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
		SceneManager::Instance()->LaunchSphere(50.f, 0.5f);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_X))
		Quit();
}


// Program Entry Point
int main()
{
	//Initialize our Window, Physics, Scenes etc
	Initialize();
	GraphicsPipeline::Instance()->SetVsyncEnabled(false);

	Window::GetWindow().GetTimer()->GetTimedMS();

	//Create main game-loop
	while (Window::GetWindow().UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		//Start Timing
		
		float dt = Window::GetWindow().GetTimer()->GetTimedMS() * 0.001f;	//How many milliseconds since last update?
																		//Update Performance Timers (Show results every second)
		timer_total.UpdateRealElapsedTime(dt);
		timer_physics.UpdateRealElapsedTime(dt);
		timer_update.UpdateRealElapsedTime(dt);
		timer_render.UpdateRealElapsedTime(dt);

		//Print Status Entries
		PrintStatusEntries();

		//Handle Keyboard Inputs
		HandleKeyboardInputs();

		
		timer_total.BeginTimingSection();

		//Update Scene
		timer_update.BeginTimingSection();
		SceneManager::Instance()->GetCurrentScene()->OnUpdateScene(dt);
		timer_update.EndTimingSection();

		//Update Physics	
		timer_physics.BeginTimingSection();
		PhysicsEngine::Instance()->Update(dt);
		timer_physics.EndTimingSection();

		//Render Scene
		timer_render.BeginTimingSection();
		GraphicsPipeline::Instance()->UpdateScene(dt);
		GraphicsPipeline::Instance()->RenderScene();
		{
			//Forces synchronisation if vsync is disabled
			// - This is solely to allow accurate estimation of render time
			// - We should NEVER normally lock our render or game loop!		
		//	glClientWaitSync(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, NULL), GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
		}
		timer_render.EndTimingSection();

		

		//Finish Timing
		timer_total.EndTimingSection();		
	}

	//Cleanup
	Quit();
	return 0;
}