

//-----------------------------------------------------------------------------
// Dante Arruda - 100709110
//-----------------------------------------------------------------------------
//Ice Hockey Computer Graphics Midterm
// 
// Player 1 Controls: W,A,S,D
//	Player 2 Controls: Mouse Control
// 
// To Exit: Press alt+tab and close the window
// 
//-----------------------------------------------------------------------------
// Ryan Fieldhouse - 100784589
//-----------------------------------------------------------------------------


#include <Logging.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <json.hpp>
#include <fstream>
#include <sstream>
#include <typeindex>
#include <optional>
#include <string>

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)
#include "Utils/GlmBulletConversions.h"

// Graphics
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture2D.h"
#include "Graphics/VertexTypes.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/CharacterController.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/MoveThings.h"
#include "Gameplay/Components/MouseController.h"
#include "Gameplay/Components/ScoreComponent.h"
#include "Gameplay/Components/LevelMover.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"


//#define LOG_GL_NOTIFICATIONS

/*
	Handles debug messages from OpenGL
	https://www.khronos.org/opengl/wiki/Debug_Output#Message_Components
	@param source    Which part of OpenGL dispatched the message
	@param type      The type of message (ex: error, performance issues, deprecated behavior)
	@param id        The ID of the error or message (to distinguish between different types of errors, like nullref or index out of range)
	@param severity  The severity of the message (from High to Notification)
	@param length    The length of the message
	@param message   The human readable message from OpenGL
	@param userParam The pointer we set with glDebugMessageCallback (should be the game pointer)
*/
void GlDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string sourceTxt;
	switch (source) {
		case GL_DEBUG_SOURCE_API: sourceTxt = "DEBUG"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceTxt = "WINDOW"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceTxt = "SHADER"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: sourceTxt = "THIRD PARTY"; break;
		case GL_DEBUG_SOURCE_APPLICATION: sourceTxt = "APP"; break;
		case GL_DEBUG_SOURCE_OTHER: default: sourceTxt = "OTHER"; break;
	}
	switch (severity) {
		case GL_DEBUG_SEVERITY_LOW:          LOG_INFO("[{}] {}", sourceTxt, message); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN("[{}] {}", sourceTxt, message); break;
		case GL_DEBUG_SEVERITY_HIGH:         LOG_ERROR("[{}] {}", sourceTxt, message); break;
			#ifdef LOG_GL_NOTIFICATIONS
		case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_INFO("[{}] {}", sourceTxt, message); break;
			#endif
		default: break;
	}
}
// Stores our GLFW window in a global variable for now
GLFWwindow* window;
// The current size of our window in pixels
glm::ivec2 windowSize = glm::ivec2(1000, 1000);
// The title of our GLFW window
std::string windowTitle = "Beat!";

// using namespace should generally be avoided, and if used, make sure it's ONLY in cpp files
using namespace Gameplay;
using namespace Gameplay::Physics;

// The scene that we will be rendering
Scene::Sptr scene = nullptr;

void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	windowSize = glm::ivec2(width, height);
	if (windowSize.x * windowSize.y > 0) {
		scene->MainCamera->ResizeWindow(width, height);
	}
}

/// <summary>
/// Handles intializing GLFW, should be called before initGLAD, but after Logger::Init()
/// Also handles creating the GLFW window
/// </summary>
/// <returns>True if GLFW was initialized, false if otherwise</returns>
bool initGLFW() {
	// Initialize GLFW
	if (glfwInit() == GLFW_FALSE) {
		LOG_ERROR("Failed to initialize GLFW");
		return false;
	}

	//Create a new GLFW window and make it current
	window = glfwCreateWindow(windowSize.x, windowSize.y, windowTitle.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window);
	
	// Set our window resized callback
	glfwSetWindowSizeCallback(window, GlfwWindowResizedCallback);

	return true;
}

/// <summary>
/// Handles initializing GLAD and preparing our GLFW window for OpenGL calls
/// </summary>
/// <returns>True if GLAD is loaded, false if there was an error</returns>
bool initGLAD() {
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		LOG_ERROR("Failed to initialize Glad");
		return false;
	}
	return true;
}

/// <summary>
/// Draws a widget for saving or loading our scene
/// </summary>
/// <param name="scene">Reference to scene pointer</param>
/// <param name="path">Reference to path string storage</param>
/// <returns>True if a new scene has been loaded</returns>
bool DrawSaveLoadImGui(Scene::Sptr& scene, std::string& path) {
	// Since we can change the internal capacity of an std::string,
	// we can do cool things like this!
	ImGui::InputText("Path", path.data(), path.capacity());

	// Draw a save button, and save when pressed
	if (ImGui::Button("Save")) {
		scene->Save(path);
	}
	ImGui::SameLine();
	// Load scene from file button
	if (ImGui::Button("Load")) {
		// Since it's a reference to a ptr, this will
		// overwrite the existing scene!
		scene = nullptr;
		scene = Scene::Load(path);

		return true;
	}
	return false;
}

/// <summary>
/// Draws some ImGui controls for the given light
/// </summary>
/// <param name="title">The title for the light's header</param>
/// <param name="light">The light to modify</param>
/// <returns>True if the parameters have changed, false if otherwise</returns>
bool DrawLightImGui(const Scene::Sptr& scene, const char* title, int ix) {
	bool isEdited = false;
	bool result = false;
	Light& light = scene->Lights[ix];
	ImGui::PushID(&light); // We can also use pointers as numbers for unique IDs
	if (ImGui::CollapsingHeader(title)) {
		isEdited |= ImGui::DragFloat3("Pos", &light.Position.x, 0.01f);
		isEdited |= ImGui::ColorEdit3("Col", &light.Color.r);
		isEdited |= ImGui::DragFloat("Range", &light.Range, 0.1f);

		result = ImGui::Button("Delete");
	}
	if (isEdited) {
		scene->SetShaderLight(ix);
	}

	ImGui::PopID();
	return result;
}
// For spawning small platforms
void SpawnObj(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(-10.900f, 5.610f, -4.920f),
			glm::vec3 rot = glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3 scale = glm::vec3(0.350f, 0.350f, 0.350f)) {
	// Tutorial Stuff
	GameObject::Sptr Startplatform = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Startplatform->SetPostion(pos);
		Startplatform->SetRotation(rot);
		Startplatform->SetScale(scale);

		Startplatform->Add<LevelMover>();

		// Create and attach a renderer for the monkey
		RenderComponent::Sptr renderer = Startplatform->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Add a dynamic rigid body to this monkey
		RigidBody::Sptr physics = Startplatform->Add<RigidBody>(RigidBodyType::Kinematic);
		//physics->AddCollider(BoxCollider::Create(glm::vec3(1.0f, 1.0f, 1.0f)));

		// FIX THIS //
		ICollider::Sptr Box1 = physics->AddCollider(BoxCollider::Create(glm::vec3(1.2f, 1.0f, 0.5f)));
		Box1->SetPosition(glm::vec3(0.f, 0.f, 0.f));
		Box1->SetScale(glm::vec3(1,1,1));
	}
}

// for spawning start/end platforms
void SpawnStartPlat(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f)) {
	
	GameObject::Sptr Startplatform = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Startplatform->SetPostion(pos);
		Startplatform->SetRotation(rot);
		Startplatform->SetScale(scale);

		Startplatform->Add<LevelMover>();

		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Startplatform->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = Startplatform->Add<RigidBody>(RigidBodyType::Kinematic);
		physics->AddCollider(BoxCollider::Create(glm::vec3(1.8f, 0.7f, 1.0f)));

		// FIX THIS //
		//ICollider::Sptr Box1 = physics->AddCollider(BoxCollider::Create(glm::vec3(1.0f, 1.0f, 1.0f)));
		//Box1->SetPosition(glm::vec3(0.f, 0.f, 0.f));
		//Box1->SetScale(glm::vec3(1, 1, 1));
	}
}
// for spawning Beat Gems
void SpawnGem(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f)) {

	GameObject::Sptr Startplatform = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Startplatform->SetPostion(pos);
		Startplatform->SetRotation(rot);
		Startplatform->SetScale(scale);

		//Add Components
		Startplatform->Add<LevelMover>();
		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Startplatform->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = Startplatform->Add<RigidBody>(RigidBodyType::Kinematic);
		// For Gem Colliders X = left/right Y = Up/Down Z = Towards/Away
		physics->AddCollider(BoxCollider::Create(glm::vec3(0.5f, 0.3f, 0.5f)));

		// FIX THIS //
		//ICollider::Sptr Box1 = physics->AddCollider(BoxCollider::Create(glm::vec3(1.0f, 1.0f, 1.0f)));
		//Box1->SetPosition(glm::vec3(0.f, 0.f, 0.f));
		//Box1->SetScale(glm::vec3(1, 1, 1));
	}
}
// For Spawning Collectables (CDs/Vinyls
void SpawnCollectable(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f)) {

	GameObject::Sptr Startplatform = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Startplatform->SetPostion(pos);
		Startplatform->SetRotation(rot);
		Startplatform->SetScale(scale);

		//Add Components
		Startplatform->Add<LevelMover>();

		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Startplatform->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = Startplatform->Add<RigidBody>(RigidBodyType::Kinematic);
		// For Colliders X is towards Cam, Y is up/down , Z is Left and Right
		ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(0.5f, 0.5f, 0.5f)));
		CollectCollider->SetPosition(glm::vec3(0.0f, 0.5f, 0.0f));

	}
}

void SpawnWallJump(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f)) {

	GameObject::Sptr Startplatform = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Startplatform->SetPostion(pos);
		Startplatform->SetRotation(rot);
		Startplatform->SetScale(scale);

		Startplatform->Add<LevelMover>();

		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Startplatform->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = Startplatform->Add<RigidBody>(RigidBodyType::Kinematic);
		// For Wall Jump Colliders, X = Left/Right Y = towards/away, z = Up/Down
		ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(0.3f, 0.5f, 3.2f)));
		CollectCollider->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

	}
}

int main() {	
	Logger::Init(); // We'll borrow the logger from the toolkit, but we need to initialize it

	//Initialize GLFW
	if (!initGLFW())
		return 1;

	//Initialize GLAD
	if (!initGLAD())
		return 1;

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GlDebugMessage, nullptr);

	// Initialize our ImGui helper
	ImGuiHelper::Init(window);

	// Initialize our resource manager
	ResourceManager::Init();

	// Register all our resource types so we can load them from manifest files
	ResourceManager::RegisterType<Texture2D>();
	ResourceManager::RegisterType<Material>();
	ResourceManager::RegisterType<MeshResource>();
	ResourceManager::RegisterType<Shader>();

	// Register all of our component types so we can load them from files
	ComponentManager::RegisterType<Camera>();
	ComponentManager::RegisterType<RenderComponent>();
	ComponentManager::RegisterType<RigidBody>();
	ComponentManager::RegisterType<TriggerVolume>();
	ComponentManager::RegisterType<MoveThings>();
	ComponentManager::RegisterType<MouseController>();
	ComponentManager::RegisterType<RotatingBehaviour>();
	ComponentManager::RegisterType<CharacterController>();
	ComponentManager::RegisterType<JumpBehaviour>();
	ComponentManager::RegisterType<ScoreComponent>();
	ComponentManager::RegisterType<MaterialSwapBehaviour>();
	ComponentManager::RegisterType<LevelMover>();

	// GL states, we'll enable depth testing and backface fulling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene) {
		ResourceManager::LoadManifest("manifest.json");
		scene = Scene::Load("scene.json");
	} 
	else {
		// Create our OpenGL resources
		Shader::Sptr uboShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shader.glsl" }, 
			{ ShaderPartType::Fragment, "shaders/frag_blinn_phong_textured.glsl" }
		}); 

		//MeshResource::Sptr tableMesh = ResourceManager::CreateAsset<MeshResource>("table.obj");
		//MeshResource::Sptr puckMesh = ResourceManager::CreateAsset<MeshResource>("puck.obj");
		//MeshResource::Sptr paddleMesh = ResourceManager::CreateAsset<MeshResource>("paddle.obj");
		//MeshResource::Sptr ScoreBoard = ResourceManager::CreateAsset<MeshResource>("ScoreBoard.obj");
		
		
		MeshResource::Sptr SmallPlatform = ResourceManager::CreateAsset<MeshResource>("SmallSpeakerPlatformV5.obj");
		MeshResource::Sptr WallJump = ResourceManager::CreateAsset<MeshResource>("WallJumpV6.obj");
		MeshResource::Sptr BeatGem = ResourceManager::CreateAsset<MeshResource>("Gem.obj");
		MeshResource::Sptr Vinyl = ResourceManager::CreateAsset<MeshResource>("VinylV2.obj");
		MeshResource::Sptr TutorialSign = ResourceManager::CreateAsset<MeshResource>("TutorialSign.obj");
		MeshResource::Sptr CharacterMesh = ResourceManager::CreateAsset<MeshResource>("dudeCharacter.obj");
		MeshResource::Sptr StartPlatform = ResourceManager::CreateAsset<MeshResource>("StartPlatformV6.obj");

		Texture2D::Sptr tableTex = ResourceManager::CreateAsset<Texture2D>("textures/HockeyRink.png"); // delete this later 
		
		Texture2D::Sptr StartTex = ResourceManager::CreateAsset<Texture2D>("textures/DiscoBuildingTex.png"); // Does not exist yet loam
		Texture2D::Sptr SmallTex = ResourceManager::CreateAsset<Texture2D>("textures/DanceFloorTex2.png"); // Does not exist yet loam
		Texture2D::Sptr VinylTex = ResourceManager::CreateAsset<Texture2D>("textures/VinylTex.png"); // Does not exist yet loam
		Texture2D::Sptr GemTex = ResourceManager::CreateAsset<Texture2D>("textures/Gem.png"); // Does exist just reload loam
		Texture2D::Sptr TutorialSignTex = ResourceManager::CreateAsset<Texture2D>("textures/TutorialSign.png"); // Does not exist yet loam
		Texture2D::Sptr CharacterTex = ResourceManager::CreateAsset<Texture2D>("textures/shirt.png");
		Texture2D::Sptr LoseScreenTex = ResourceManager::CreateAsset<Texture2D>("textures/Game_Over_Screen.png");
		Texture2D::Sptr WallJumpTex = ResourceManager::CreateAsset<Texture2D>("textures/WallJumpTex.png");


		// Create an empty scene
		scene = std::make_shared<Scene>();

		// I hate this
		scene->BaseShader = uboShader;

		// Create our materials
		Material::Sptr StartPlatformMaterial = ResourceManager::CreateAsset<Material>();
		{
			StartPlatformMaterial->Name = "StartPlatform";
			StartPlatformMaterial->MatShader = scene->BaseShader;
			StartPlatformMaterial->Texture = StartTex;
			StartPlatformMaterial->Shininess = 2.0f;
		}

		Material::Sptr SmallPlatformMaterial = ResourceManager::CreateAsset<Material>();
		{
			SmallPlatformMaterial->Name = "SmallPlatform";
			SmallPlatformMaterial->MatShader = scene->BaseShader;
			SmallPlatformMaterial->Texture = SmallTex;
			SmallPlatformMaterial->Shininess = 2.0f;
		}

		Material::Sptr WallJumpMaterial = ResourceManager::CreateAsset<Material>();
		{
			WallJumpMaterial->Name = "WallJump";
			WallJumpMaterial->MatShader = scene->BaseShader;
			WallJumpMaterial->Texture = WallJumpTex;
			WallJumpMaterial->Shininess = 2.0f;
		}

		Material::Sptr BeatGemMaterial = ResourceManager::CreateAsset<Material>();
		{
			BeatGemMaterial->Name = "BeatGem";
			BeatGemMaterial->MatShader = scene->BaseShader;
			BeatGemMaterial->Texture = GemTex;
			BeatGemMaterial->Shininess = 2.0f;
		}

		Material::Sptr VinylMaterial = ResourceManager::CreateAsset<Material>();
		{
			VinylMaterial->Name = "Vinyl";
			VinylMaterial->MatShader = scene->BaseShader;
			VinylMaterial->Texture = VinylTex;
			VinylMaterial->Shininess = 2.0f;
		}

		Material::Sptr TutorialSignMaterial = ResourceManager::CreateAsset<Material>();
		{
			TutorialSignMaterial->Name = "Tutorial Sign";
			TutorialSignMaterial->MatShader = scene->BaseShader;
			TutorialSignMaterial->Texture = TutorialSignTex;
			TutorialSignMaterial->Shininess = 2.0f;
		}

		Material::Sptr CharacterMaterial = ResourceManager::CreateAsset<Material>();
		{
			CharacterMaterial->Name = "Character";
			CharacterMaterial->MatShader = scene->BaseShader;
			CharacterMaterial->Texture = CharacterTex; 
			CharacterMaterial->Shininess = 2.0f;
		}

		Material::Sptr LoseScreenMaterial = ResourceManager::CreateAsset<Material>();
		{
			LoseScreenMaterial->Name = "Lose Screen";
			LoseScreenMaterial->MatShader = scene->BaseShader;
			LoseScreenMaterial->Texture = TutorialSignTex;
			LoseScreenMaterial->Shininess = 2.0f;
		}




		// Format: Mesh, Material, IMGUI TEXT, position, rotation, scale
		// Position Axis Work as follows: X = Left and Right Y = Towards and away from camera Z = Up and Down
		// always keep objects at 5.610f to keep the y axis consisent since we are 2.5D

		// Tutorial Block
		
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-9.820f, 5.350, -4.450), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform 1", glm::vec3(-6.070f, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform 2", glm::vec3(-3.320f, 5.610f, -2.200f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform 3", glm::vec3(-0.400f, 5.610f, -4.040f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform 4", glm::vec3(-0.060f, 5.610f, 4.450f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump 1", glm::vec3(1.680f, 5.610f, 2.610f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
		SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump 2", glm::vec3(4.350f, 5.610f, 3.930f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
		SpawnGem(BeatGem, BeatGemMaterial, "BeatGem", glm::vec3(2.410f, 5.610f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnCollectable(Vinyl, VinylMaterial, "Vinyl", glm::vec3(-0.040f, 5.610f, 5.920f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		//SpawnObj(TutorialSign, TutorialSignMaterial, "Tutorial Sign 1", glm::vec3(-9.770f, 5.690f, -3.890f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.310f, 0.310f, 0.310f));
		//SpawnObj(TutorialSign, TutorialSignMaterial, "Tutorial Sign 2", glm::vec3(-0.390f, 5.690f, -3.440f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.310f, 0.310f, 0.310f));
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(6.360f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		

		// 1st Block
		/*
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform Block1", glm::vec3(-9.820f, 5.350, -4.450), glm::vec3(90.0f, 0.0f, 0.0f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block1 1", glm::vec3(-6.070f, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block1 2", glm::vec3(-2.840f, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block1 3", glm::vec3(2.760f, 10.990f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block1", glm::vec3(0.120f, 5.610f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnCollectable(Vinyl, VinylMaterial, "Vinyl Block1", glm::vec3(-0.120f, 5.610f, 0.290f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f)); //SCUFFED
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform Block1", glm::vec3(6.360f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		*/

		// 2nd Block
		/*
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform Block2", glm::vec3(-9.820f, 5.350, -4.450), glm::vec3(90.0f, 0.0f, 0.0f));
		SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump Block2 1", glm::vec3(-10.340f, 4.830f, 5.450f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.040f, 1.500f));
		SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump Block2 2", glm::vec3(-8.430f, 5.610f, 3.930f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.040f, 1.500f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block2 1", glm::vec3(-6.370f, 40.000f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block2 2", glm::vec3(1.940f, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block2", glm::vec3(-1.570f, 20.000f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform Block2", glm::vec3(6.360f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		*/

		// 3rd Block
		/*
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform Block3", glm::vec3(-9.820f, 5.350, -4.450), glm::vec3(90.0f, 0.0f, 0.0f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block2 1", glm::vec3(-4.360f, 5.6110, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block3 2", glm::vec3(0.350f, 5.6110, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block3 3", glm::vec3(0.390f, 5.6110, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block3 4", glm::vec3(3.220f, 5.6110f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block3 1", glm::vec3(-6.870f, 5.6110f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block3 2", glm::vec3(-1.870f, 5.6110f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		//SpawnCollectable(Vinyl, VinylMaterial, "Vinyl Block1", glm::vec3(-0.120f, -7.700f, 0.290f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f)); SCUFFED (Repostion)
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform Block3", glm::vec3(6.360f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		*/
		
		// Player:
		GameObject::Sptr character = scene->CreateGameObject("Character/Player");
		{
			// Set position in the scene
			character->SetPostion(glm::vec3(-8.970f, 5.710f, -3.800f));
			character->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
			character->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

			// Add some behaviour that relies on the physics body
			//character->Add<JumpBehaviour>();
			character->Add<CharacterController>();

			// Create and attach a renderer for the paddle
			RenderComponent::Sptr renderer = character->Add<RenderComponent>();
			renderer->SetMesh(CharacterMesh);
			renderer->SetMaterial(CharacterMaterial);

			// Add a kinematic rigid body to the paddle
			RigidBody::Sptr physics = character->Add<RigidBody>(RigidBodyType::Dynamic);
			physics->AddCollider(ConvexMeshCollider::Create());
			// world grav changed in scene.cpp
		}





		// (Delete this later)
		/*
		Material::Sptr tableMaterial = ResourceManager::CreateAsset<Material>();
		{
			tableMaterial->Name = "Table";
			tableMaterial->MatShader = scene->BaseShader;
			tableMaterial->Texture = tableTex;
			tableMaterial->Shininess = 2.0f;
		}
		Material::Sptr puckMaterial = ResourceManager::CreateAsset<Material>();
		{
			puckMaterial->Name = "puck";
			puckMaterial->MatShader = scene->BaseShader;
			puckMaterial->Texture = puckTex;
			puckMaterial->Shininess = 1.0f;
		}
		Material::Sptr paddleRedMaterial = ResourceManager::CreateAsset<Material>();
		{
			paddleRedMaterial->Name = "PaddleRed";
			paddleRedMaterial->MatShader = scene->BaseShader;
			paddleRedMaterial->Texture = paddleRedTex;
			paddleRedMaterial->Shininess = 2.0f;
		}
		Material::Sptr paddleBlueMaterial = ResourceManager::CreateAsset<Material>();
		{
			paddleBlueMaterial->Name = "PaddleBlue";
			paddleBlueMaterial->MatShader = scene->BaseShader;
			paddleBlueMaterial->Texture = paddleBlueTex;
			paddleBlueMaterial->Shininess = 2.0f;
		}
		*/
	
		// Create some lights for our scene
		scene->Lights.resize(3);
		scene->Lights[0].Position = glm::vec3(-2.8f, 1.0f, 3.0f);
		scene->Lights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[0].Range = 1000.0f;

		scene->Lights[1].Position = glm::vec3(1.0f, 0.0f, 3.0f);
		scene->Lights[1].Color = glm::vec3(0.2f, 0.8f, 1.0f);
		scene->Lights[1].Range = 0.5f;

		scene->Lights[2].Position = glm::vec3(0.0f, 1.0f, 3.0f);
		scene->Lights[2].Color = glm::vec3(1.0f, 0.2f, 1.0f);
		scene->Lights[2].Range = 0.5f;

		// We'll create a mesh that is a simple plane that we can resize later
	//	MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
	//	planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
	//	planeMesh->GenerateMesh();
	//
		// Set up the scene's camera
		GameObject::Sptr camera = scene->CreateGameObject("Main Camera");
		{
			camera->SetPostion(glm::vec3(-1.410, -3.500, 2.450));
			camera->LookAt(glm::vec3(0.0f));
			camera->SetRotation(glm::vec3(-103, 180, -180));

			Camera::Sptr cam = camera->Add<Camera>();
			cam->SetOrthoEnabled(true);
			cam->SetOrthoVerticalScale(19.0f);
			cam->SetFovRadians(105.f);
			cam->SetNearPlane(0.3);

			// Make sure that the camera is set as the scene's main camera!
			scene->MainCamera = cam;
		}

		/*
		// Tutorial Stuff
		GameObject::Sptr Startplatform = scene->CreateGameObject("Start Platform");
		{
			// Set position in the scene
			Startplatform->SetPostion(glm::vec3(-7.490f, 5.660f, -2.890f));
			Startplatform->SetRotation(glm::vec3(180.0f, 0.0f, 180.0f));
			Startplatform->SetScale(glm::vec3(0.180f, 0.070f, 0.180));

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = Startplatform->Add<RenderComponent>();
			renderer->SetMesh(StartPlatform);
			renderer->SetMaterial(StartPlatformMaterial);

			// Add a dynamic rigid body to this monkey
			RigidBody::Sptr physics = Startplatform->Add<RigidBody>(RigidBodyType::Static);
			//physics->AddCollider(BoxCollider::Create(glm::vec3(1.0f, 1.0f, 1.0f)));

			// FIX THIS //
			ICollider::Sptr Box1 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.05f, 0.2f, 0.5f)));
			Box1->SetPosition(glm::vec3(-0.930f, 0.140f, 0.0f));

		}

		GameObject::Sptr SmallPlatforms = scene->CreateGameObject("Small Platform");
		{
			// Set position in the scene
			SmallPlatforms->SetPostion(glm::vec3(-5.490f, 6.960f, -2.890f));
			SmallPlatforms->SetRotation(glm::vec3(180.0f, 0.0f, 180.0f));
			SmallPlatforms->SetScale(glm::vec3(0.280f, 0.070f, 0.180));

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = SmallPlatforms->Add<RenderComponent>();
			renderer->SetMesh(SmallPlatform);
			renderer->SetMaterial(SmallPlatformMaterial);

			// Add a dynamic rigid body to this monkey
			RigidBody::Sptr physics = SmallPlatforms->Add<RigidBody>(RigidBodyType::Static);
			//physics->AddCollider(BoxCollider::Create(glm::vec3(1.0f, 1.0f, 1.0f)));
		}
		*/


		// Set up all our sample objects
		//GameObject::Sptr plane = scene->CreateGameObject("Plane");
		//{
		//	// Scale up the plane
		//	plane->SetScale(glm::vec3(10.0F));
		//
		//	// Create and attach a RenderComponent to the object to draw our mesh
		//	RenderComponent::Sptr renderer = plane->Add<RenderComponent>();
		//	renderer->SetMesh(planeMesh);
		//	renderer->SetMaterial(boxMaterial);
		//
		//	// Attach a plane collider that extends infinitely along the X/Y axis
		//	RigidBody::Sptr physics = plane->Add<RigidBody>(/*static by default*/);
		//	physics->AddCollider(PlaneCollider::Create());
		//}
		//
		//GameObject::Sptr square = scene->CreateGameObject("Square");
		//{
		//	// Set position in the scene
		//	square->SetPostion(glm::vec3(0.0f, 0.0f, 2.0f));
		//	// Scale down the plane
		//	square->SetScale(glm::vec3(0.5f));
		//
		//	// Create and attach a render component
		//	RenderComponent::Sptr renderer = square->Add<RenderComponent>();
		//	renderer->SetMesh(planeMesh);
		//	renderer->SetMaterial(boxMaterial);
		//
		//	// This object is a renderable only, it doesn't have any behaviours or
		//	// physics bodies attached!
		//}
		//
		//GameObject::Sptr monkey1 = scene->CreateGameObject("Monkey 1");
		//{
		//	// Set position in the scene
		//	monkey1->SetPostion(glm::vec3(1.5f, 0.0f, 1.0f));
		//
		//	// Add some behaviour that relies on the physics body
		//	monkey1->Add<JumpBehaviour>();
		//
		//	// Create and attach a renderer for the monkey
		//	RenderComponent::Sptr renderer = monkey1->Add<RenderComponent>();
		//	renderer->SetMesh(monkeyMesh);
		//	renderer->SetMaterial(monkeyMaterial);
		//
		//	// Add a dynamic rigid body to this monkey
		//	RigidBody::Sptr physics = monkey1->Add<RigidBody>(RigidBodyType::Dynamic);
		//	physics->AddCollider(ConvexMeshCollider::Create());
		//
		//
		//	// We'll add a behaviour that will interact with our trigger volumes
		//	MaterialSwapBehaviour::Sptr triggerInteraction = monkey1->Add<MaterialSwapBehaviour>();
		//	triggerInteraction->EnterMaterial = boxMaterial;
		//	triggerInteraction->ExitMaterial = monkeyMaterial;
		//}
		//
		//GameObject::Sptr monkey2 = scene->CreateGameObject("Complex Object");
		//{
		//	// Set and rotation position in the scene
		//	monkey2->SetPostion(glm::vec3(-1.5f, 0.0f, 1.0f));
		//	monkey2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
		//
		//	// Add a render component
		//	RenderComponent::Sptr renderer = monkey2->Add<RenderComponent>();
		//	renderer->SetMesh(monkeyMesh);
		//	renderer->SetMaterial(boxMaterial);
		//
		//	// This is an example of attaching a component and setting some parameters
		//	RotatingBehaviour::Sptr behaviour = monkey2->Add<RotatingBehaviour>();
		//	behaviour->RotationSpeed = glm::vec3(0.0f, 0.0f, -90.0f);
		//}

		// Delete this later (Score stuff)
		/*
		GameObject::Sptr ScoreP2 = scene->CreateGameObject("ScoreP2");
		ScoreComponent::Sptr score2 = ScoreP2->Add<ScoreComponent>();
		std::string p2 = score2 != nullptr ? score2->GetGUID().str() : "null";
		{
			ScoreP2->SetPostion(glm::vec3(2.0f, 1.5f, 1.00f));
			ScoreP2->SetRotation(glm::vec3(45, 0, -180));
			ScoreP2->SetScale(glm::vec3(2.0f, 2.0f, 2.0f));



			RenderComponent::Sptr renderer = ScoreP2->Add<RenderComponent>();

			renderer->SetMesh(ScoreBoard);
			renderer->SetMaterial(R0_mat);
			
			// We'll add a behaviour that will interact with our trigger volumes
			MaterialSwapBehaviour::Sptr triggerInteraction = ScoreP2->Add<MaterialSwapBehaviour>();

			RigidBody::Sptr physics = ScoreP2->Add<RigidBody>(RigidBodyType::Kinematic);
			ICollider::Sptr Box1 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.25f)));

			triggerInteraction->EnterMaterial = R0_mat;
			triggerInteraction->ExitMaterial = R1_mat;
		}

		
		GameObject::Sptr ScoreP1 = scene->CreateGameObject("ScoreP1");
		ScoreComponent::Sptr score1 = ScoreP1->Add<ScoreComponent>();
		std::string p1 = score1 != nullptr ? score1->GetGUID().str() : "null";
		{
			ScoreP1->SetPostion(glm::vec3(1.0f, 1.5f, 1.0f));
			ScoreP1->SetRotation(glm::vec3(45, 0, -180));
			ScoreP1->SetScale(glm::vec3(2.0f, 2.0f, 2.0f));
			
		
			RenderComponent::Sptr renderer = ScoreP1->Add<RenderComponent>();
			renderer->SetMesh(ScoreBoard);
			renderer->SetMaterial(R0_mat);

			RigidBody::Sptr physics = ScoreP1->Add<RigidBody>(RigidBodyType::Kinematic);
			ICollider::Sptr Box1 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.25f)));

		
			// We'll add a behaviour that will interact with our trigger volumes
			MaterialSwapBehaviour::Sptr triggerInteraction = ScoreP1->Add<MaterialSwapBehaviour>();
		
			triggerInteraction->EnterMaterial = R0_mat;
			triggerInteraction->ExitMaterial = R1_mat;
		}
		*/
		
		// Delete this later (Table, Paddle, Puck, etc)
		/*
		GameObject::Sptr table = scene->CreateGameObject("Table");
		{
			// Set position in the scene
			table->SetPostion(glm::vec3(1.5f, 0.0f, 1.0f));
			table->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = table->Add<RenderComponent>();
			renderer->SetMesh(tableMesh);
			renderer->SetMaterial(tableMaterial);

			// Add a dynamic rigid body to this monkey
			RigidBody::Sptr physics = table->Add<RigidBody>(RigidBodyType::Static);
			//physics->AddCollider(BoxCollider::Create(glm::vec3(1.0f, 1.0f, 1.0f)));

			ICollider::Sptr Box1 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.05f, 0.2f, 0.5f)));
			Box1->SetPosition(glm::vec3( -0.930f, 0.140f, 0.0f));

			ICollider::Sptr Box2 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.05f, 0.2f, 0.5f)));
			Box2->SetPosition(glm::vec3(0.930f, 0.140f, 0.0f));

			ICollider::Sptr Box3 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.05f, 0.2f, 1.0f)));
			Box3->SetPosition(glm::vec3(0.0f, 0.140f, 0.5f));
			Box3->SetRotation(glm::vec3(0.0f, 90.0f, 0.0f));

			ICollider::Sptr Box4 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.05f, 0.2f, 1.0f)));
			Box4->SetPosition(glm::vec3(-0.0f, 0.140f, -0.5f));
			Box4->SetRotation(glm::vec3(0.0f, 90.0f, 0.0f));

			ICollider::Sptr Box5 = physics->AddCollider(BoxCollider::Create(glm::vec3(2.0f, 0.7f, 0.5f)));
			Box5->SetPosition(glm::vec3(0.0f, -0.7f, 0.0f));
			Box5->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

			ICollider::Sptr Box6 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.05f, 0.2f, 0.2f)));
			Box6->SetPosition(glm::vec3(-0.84f, 0.10f, -0.390f));
			Box6->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

			ICollider::Sptr Box7 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.05f, 0.2f, 0.2f)));
			Box7->SetPosition(glm::vec3(-0.84f, 0.150f, 0.320f));
			Box7->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

			ICollider::Sptr Box8 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.05f, 0.2f, 0.2f)));
			Box8->SetPosition(glm::vec3(0.84f, 0.10f, -0.390f));
			Box8->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

			ICollider::Sptr Box9 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.05f, 0.2f, 0.2f)));
			Box9->SetPosition(glm::vec3(0.84f, 0.150f, 0.320f));
			Box9->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		}

		GameObject::Sptr paddle = scene->CreateGameObject("Paddle");
		{
			// Set position in the scene
			paddle->SetPostion(glm::vec3(1.0f, -0.01f, 1.00f));
			paddle->SetRotation(glm::vec3(90.0f, 0.04f, 0.0f));

			paddle->Add<CharacterController>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = paddle->Add<RenderComponent>();
			renderer->SetMesh(paddleMesh);
			renderer->SetMaterial(paddleBlueMaterial);

			// Add a dynamic rigid body
			RigidBody::Sptr physics = paddle->Add<RigidBody>(RigidBodyType::Kinematic);

			ICollider::Sptr Box1 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.05f, 0.04f, 0.05f)));
			Box1->SetPosition(glm::vec3(0.0f, 0.04f, 0.0f));


		}
		GameObject::Sptr paddle2 = scene->CreateGameObject("Paddle2");
		{
			// Set position in the scene
			paddle2->SetPostion(glm::vec3(2.0f, -0.01f, 1.00f));
			paddle2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

			paddle2->Add<MouseController>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = paddle2->Add<RenderComponent>();
			renderer->SetMesh(paddleMesh);
			renderer->SetMaterial(paddleRedMaterial);

			// Add a dynamic rigid body
			RigidBody::Sptr physics = paddle2->Add<RigidBody>(RigidBodyType::Kinematic);
			ICollider::Sptr Box1 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.05f, 0.04f, 0.05f)));
			Box1->SetPosition(glm::vec3(0.0f, 0.04f, 0.0f));
		}
		
		// Delete this later (Puck stuff)
		/*
		GameObject::Sptr puck = scene->CreateGameObject("Puck");
		{
			// Set position in the scene
			puck->SetPostion(glm::vec3(1.5f, 0.01f, 1.255f));
			puck->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

			MoveThings::Sptr movement = puck->Add<MoveThings>();
			movement->GrabGuid1(p1);
			movement->GrabGuid2(p2);

			movement->SetCoefficient(0.1);

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = puck->Add<RenderComponent>();
			renderer->SetMesh(puckMesh);
			renderer->SetMaterial(puckMaterial);

			// Add a dynamic rigid body
			RigidBody::Sptr physics = puck->Add<RigidBody>(RigidBodyType::Dynamic);

			ICollider::Sptr Box1 = physics->AddCollider(ConvexMeshCollider::Create());
		}
		*/

		//Create a trigger volume for testing how we can detect collisions with objects!
	/*	GameObject::Sptr trigger = scene->CreateGameObject("Trigger");
		{
			TriggerVolume::Sptr volume = trigger->Add<TriggerVolume>();
			BoxCollider::Sptr collider = BoxCollider::Create(glm::vec3(0.5f, 0.5f, 0.5f));
			collider->SetPosition(glm::vec3(0.0f, 0.0f, 0.5f));
			volume->AddCollider(collider);
	}*/

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");
	}

	// Call scene awake to start up all of our components
	scene->Window = window;
	scene->Awake();

	// We'll use this to allow editing the save/load path
	// via ImGui, note the reserve to allocate extra space
	// for input!
	std::string scenePath = "scene.json"; 
	scenePath.reserve(256); 

	bool isRotating = true;
	float rotateSpeed = 90.0f;

	// Our high-precision timer
	double lastFrame = glfwGetTime();


	BulletDebugMode physicsDebugMode = BulletDebugMode::None;
	float playbackSpeed = 1.0f;

	nlohmann::json editorSceneState;

	///// Game loop /////
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		ImGuiHelper::StartFrame();
		
		// Calculate the time since our last frame (dt)
		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);

		int windowWidth;
		int windowHeight;

		glfwGetWindowSize(scene->Window, &windowWidth, &windowHeight);

		// Showcasing how to use the imGui library!
		//bool isDebugWindowOpen = ImGui::Begin("Debugging");
		bool isDebugWindowOpen = true;
		if (isDebugWindowOpen) {
			// Draws a button to control whether or not the game is currently playing
			static char buttonLabel[64];
			sprintf_s(buttonLabel, "%s###playmode", scene->IsPlaying ? "Exit Play Mode" : "Enter Play Mode");
			if (ImGui::Button(buttonLabel)) {
				// Save scene so it can be restored when exiting play mode
				if (!scene->IsPlaying) {
					editorSceneState = scene->ToJson();
				}

				// Toggle state
				scene->IsPlaying = !scene->IsPlaying;

				// If we've gone from playing to not playing, restore the state from before we started playing
				if (!scene->IsPlaying) {
					scene = nullptr;
					// We reload to scene from our cached state
					scene = Scene::FromJson(editorSceneState);
					// Don't forget to reset the scene's window and wake all the objects!
					scene->Window = window;
					scene->Awake();
				}
			}

			// Make a new area for the scene saving/loading
			ImGui::Separator();
			if (DrawSaveLoadImGui(scene, scenePath)) {
				// C++ strings keep internal lengths which can get annoying
				// when we edit it's underlying datastore, so recalcualte size
				scenePath.resize(strlen(scenePath.c_str()));

				// We have loaded a new scene, call awake to set
				// up all our components
				scene->Window = window;
				scene->Awake();
			}
			ImGui::Separator();

			// Draw a dropdown to select our physics debug draw mode
			if (BulletDebugDraw::DrawModeGui("Physics Debug Mode:", physicsDebugMode)) {
				scene->SetPhysicsDebugDrawMode(physicsDebugMode);
			}
			LABEL_LEFT(ImGui::SliderFloat, "Playback Speed:    ", &playbackSpeed, 0.0f, 10.0f);
			ImGui::Separator();
		}

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update our application level uniforms every frame

		// Draw some ImGui stuff for the lights
		if (isDebugWindowOpen) {
			for (int ix = 0; ix < scene->Lights.size(); ix++) {
				char buff[256];
				sprintf_s(buff, "Light %d##%d", ix, ix);
				// DrawLightImGui will return true if the light was deleted
				if (DrawLightImGui(scene, buff, ix)) {
					// Remove light from scene, restore all lighting data
					scene->Lights.erase(scene->Lights.begin() + ix);
					scene->SetupShaderAndLights();
					// Move back one element so we don't skip anything!
					ix--;
				}
			}
			// As long as we don't have max lights, draw a button
			// to add another one
			if (scene->Lights.size() < scene->MAX_LIGHTS) {
				if (ImGui::Button("Add Light")) {
					scene->Lights.push_back(Light());
					scene->SetupShaderAndLights();
				}
			}
			// Split lights from the objects in ImGui
			ImGui::Separator();
		}

		dt *= playbackSpeed;

		// Perform updates for all components
		scene->Update(dt);

		// Grab shorthands to the camera and shader from the scene
		Camera::Sptr camera = scene->MainCamera;

		// Cache the camera's viewprojection
		glm::mat4 viewProj = camera->GetViewProjection();
		DebugDrawer::Get().SetViewProjection(viewProj);

		// Update our worlds physics!
		scene->DoPhysics(dt);

		// Draw object GUIs
		if (isDebugWindowOpen) {
			scene->DrawAllGameObjectGUIs();
		}
		
		// The current material that is bound for rendering
		Material::Sptr currentMat = nullptr;
		Shader::Sptr shader = nullptr;

		// Render all our objects
		ComponentManager::Each<RenderComponent>([&](const RenderComponent::Sptr& renderable) {

			// If the material has changed, we need to bind the new shader and set up our material and frame data
			// Note: This is a good reason why we should be sorting the render components in ComponentManager
			if (renderable->GetMaterial() != currentMat) {
				currentMat = renderable->GetMaterial();
				shader = currentMat->MatShader;

				shader->Bind();
				shader->SetUniform("u_CamPos", scene->MainCamera->GetGameObject()->GetPosition());
				currentMat->Apply();
			}

			// Grab the game object so we can do some stuff with it
			GameObject* object = renderable->GetGameObject();

			// Set vertex shader parameters
			shader->SetUniformMatrix("u_ModelViewProjection", viewProj * object->GetTransform());
			shader->SetUniformMatrix("u_Model", object->GetTransform());
			shader->SetUniformMatrix("u_NormalMatrix", glm::mat3(glm::transpose(glm::inverse(object->GetTransform()))));

			// Draw the object
			renderable->GetMesh()->Draw();
		});


		// End our ImGui window
		//ImGui::End();

		VertexArrayObject::Unbind();

		lastFrame = thisFrame;
		ImGuiHelper::EndFrame();
		glfwSwapBuffers(window);
	}

	// Clean up the ImGui library
	ImGuiHelper::Cleanup();

	// Clean up the resource manager
	ResourceManager::Cleanup();

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}
