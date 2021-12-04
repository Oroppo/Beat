

//-----------------------------------------------------------------------------
// Amnesia Interactive
//-----------------------------------------------------------------------------
//have a nice flight, space cowboy.
//have a nice flight, space cowboy.
//have a nice flight, space cowboy.
//have a nice flight, space cowboy.
//have a nice flight, space cowboy.
//have a nice flight, space cowboy.
// 
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
#include "Graphics/TextureCube.h"
#include "Graphics/VertexTypes.h"+
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"

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
#include "Gameplay/Components/ScoreComponent.h"
#include "Gameplay/Components/LevelMover.h"
#include "Gameplay/Components/BackgroundMover.h"
#include "Gameplay/Components/VinylAnim.h"
#include "Gameplay/Components/ForeGroundMover.h"
#include "Gameplay/Components/SeekBehaviour.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

//Sound
#include "Sound/AudioEngine.h"
#include "Fmod.h"
#include "FMOD/ToneFire.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"


//#define LOG_GL_NOTIFICATIONS

/*
	Handles debug messages from OpenGL
	https://www.khronos.org/opengl/wiki/Debug_Output#Message_Compon ents
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

		std::string newFilename = std::filesystem::path(path).stem().string() + "-manifest.json";
		ResourceManager::SaveManifest(newFilename);
	}
	ImGui::SameLine();
	// Load scene from file button
	if (ImGui::Button("Load")) {
		// Since it's a reference to a ptr, this will
		// overwrite the existing scene!
		scene = nullptr;

		std::string newFilename = std::filesystem::path(path).stem().string() + "-manifest.json";
		ResourceManager::LoadManifest(newFilename);
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
	 glm::vec3 rot = glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3 scale = glm::vec3(0.350f, 0.350f, 0.350f), GameObject::Sptr parent = nullptr) {
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
		ICollider::Sptr Box1 = physics->AddCollider(BoxCollider::Create(glm::vec3(0.87f, 0.5f, 0.4f)));
		Box1->SetPosition(glm::vec3(0.f, 0.f, 0.f));
		Box1->SetScale(glm::vec3(1,1,1));

		if (parent != nullptr) {
			parent->AddChild(Startplatform);
		}
	}
}

// for spawning start/end platforms
void SpawnStartPlat(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr ) {
	
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

		if (parent != nullptr) {
			parent->AddChild(Startplatform);
		}

	}
}
// for spawning Beat Gems
void SpawnGem(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr) {

	GameObject::Sptr Startplatform = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Startplatform->SetPostion(pos);
		Startplatform->SetRotation(rot);
		Startplatform->SetScale(scale);

		//Add Components
		Startplatform->Add<LevelMover>();
		Startplatform->Add<RotatingBehaviour>();
		
		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Startplatform->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		TriggerVolume::Sptr volume = Startplatform->Add<TriggerVolume>();
		volume->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);

		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = Startplatform->Add<RigidBody>(RigidBodyType::Kinematic);
		// For Gem Colliders X = left/right Y = Up/Down Z = Towards/Away
		physics->AddCollider(BoxCollider::Create(glm::vec3(0.5f, 0.5f, 0.5f)));


		if (parent != nullptr) {
			parent->AddChild(Startplatform);
		}

		// FIX THIS //
		//ICollider::Sptr Box1 = physics->AddCollider(BoxCollider::Create(glm::vec3(1.0f, 1.0f, 1.0f)));
		//Box1->SetPosition(glm::vec3(0.f, 0.f, 0.f));
		//Box1->SetScale(glm::vec3(1, 1, 1));
	}
}
// For Spawning Collectables (Vinyls)
void SpawnCollectable(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr) {

	GameObject::Sptr Startplatform = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Startplatform->SetPostion(pos);
		Startplatform->SetRotation(rot);
		Startplatform->SetScale(scale);

		//Add Components
		Startplatform->Add<LevelMover>();
		Startplatform->Add<VinylAnim>();
		Startplatform->Add<RotatingBehaviour>();
		
		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Startplatform->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = Startplatform->Add<RigidBody>(RigidBodyType::Kinematic);
		// For Colliders X is towards Cam, Y is up/down , Z is Left and Right
		ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(0.5f, 0.5f, 0.5f)));
		CollectCollider->SetPosition(glm::vec3(0.0f, 0.5f, 0.0f));

		if (parent != nullptr) {
			parent->AddChild(Startplatform);
		}

	}
}

// Spawns CDs
void SpawnCD(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr) {

	GameObject::Sptr Startplatform = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Startplatform->SetPostion(pos);
		Startplatform->SetRotation(rot);
		Startplatform->SetScale(scale);

		//Add Components
		Startplatform->Add<LevelMover>();
		
		Startplatform->Add<RotatingBehaviourCD>();

		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Startplatform->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		if (parent != nullptr) {
			parent->AddChild(Startplatform);
		}

		// Add a dynamic rigid body to this object
		//RigidBody::Sptr physics = Startplatform->Add<RigidBody>(RigidBodyType::Kinematic);
		// For Colliders X is towards Cam, Y is up/down , Z is Left and Right
		//ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(0.5f, 0.5f, 0.5f)));
		//CollectCollider->SetPosition(glm::vec3(0.0f, 0.5f, 0.0f));

	}
}

void SpawnWallJump(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr) {

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

		if (parent != nullptr) {
			parent->AddChild(Startplatform);
		}

	}
}

void SpawnBuilding(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr) {

	GameObject::Sptr Building = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Building->SetPostion(pos);
		Building->SetRotation(rot);
		Building->SetScale(scale);

		Building->Add<LevelMover>();

		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Building->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = Building->Add<RigidBody>(RigidBodyType::Kinematic);
		// For Wall Jump Colliders, X = Left/Right Y = towards/away, z = Up/Down
		ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(1.000f, 3.350f, 1.000f)));
		CollectCollider->SetPosition(glm::vec3(0.020f, 3.660f, -0.060f));

		if (parent != nullptr) {
			parent->AddChild(Building);
		}

	}
}

void SpawnBuilding2(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr) {

	GameObject::Sptr Building2 = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Building2->SetPostion(pos);
		Building2->SetRotation(rot);
		Building2->SetScale(scale);

		Building2->Add<LevelMover>();

		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Building2->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = Building2->Add<RigidBody>(RigidBodyType::Kinematic);
		// For Wall Jump Colliders, X = Left/Right Y = towards/away, z = Up/Down
		ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(1.700f, 3.500f, 1.000f)));
		CollectCollider->SetPosition(glm::vec3(-0.050f, 3.290f, 0.0f));

		if (parent != nullptr) {
			parent->AddChild(Building2);
		}

	}
}

void SpawnBuilding3(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr) {

	GameObject::Sptr Building3 = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Building3->SetPostion(pos);
		Building3->SetRotation(rot);
		Building3->SetScale(scale);

		Building3->Add<LevelMover>();

		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Building3->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = Building3->Add<RigidBody>(RigidBodyType::Kinematic);
		// For Wall Jump Colliders, X = Left/Right Y = towards/away, z = Up/Down
		ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(1.700f, 4.900f, 1.700f)));
		CollectCollider->SetPosition(glm::vec3(-0.030f, 5.030f, 0.240f));

		if (parent != nullptr) {
			parent->AddChild(Building3);
		}

	}
}

void SpawnSmallWallJump(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr) {

	GameObject::Sptr SmallWallJump = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		SmallWallJump->SetPostion(pos);
		SmallWallJump->SetRotation(rot);
		SmallWallJump->SetScale(scale);

		SmallWallJump->Add<LevelMover>();

		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = SmallWallJump->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = SmallWallJump->Add<RigidBody>(RigidBodyType::Kinematic);
		// For Wall Jump Colliders, X = Left/Right Y = towards/away, z = Up/Down
		ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(0.3f, 0.5f, 2.5f)));
		CollectCollider->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

		if (parent != nullptr) {
			parent->AddChild(SmallWallJump);

		}

	}
}

void SpawnSuperSmallWallJump(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr) {

	GameObject::Sptr SuperSmallWallJump = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		SuperSmallWallJump->SetPostion(pos);
		SuperSmallWallJump->SetRotation(rot);
		SuperSmallWallJump->SetScale(scale);

		SuperSmallWallJump->Add<LevelMover>();

		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = SuperSmallWallJump->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = SuperSmallWallJump->Add<RigidBody>(RigidBodyType::Kinematic);
		// For Wall Jump Colliders, X = Left/Right Y = towards/away, z = Up/Down
		ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(0.100f, 0.200f, 0.800f)));
		CollectCollider->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

		if (parent != nullptr) {
			parent->AddChild(SuperSmallWallJump);

		}

	}
}

/// <summary>
/// Draws a simple window for displaying materials and their editors
/// </summary>
void DrawMaterialsWindow() {
	if (ImGui::Begin("Materials")) {
		ResourceManager::Each<Material>([](Material::Sptr material) {
			material->RenderImGui();
			});
	}
	ImGui::End();
}

void SpawnBackGroundCar(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr) {

	GameObject::Sptr Car1 = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Car1->SetPostion(pos);
		Car1->SetRotation(rot);
		Car1->SetScale(scale);

		//Add Components
		Car1->Add<BackgroundMover>();

		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Car1->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Is background Object and therefore has no colliders
		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = Car1->Add<RigidBody>(RigidBodyType::Kinematic);

		if (parent != nullptr) {
			parent->AddChild(Car1);

		}
	}
}


void SpawnForeGroundCar(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr) {

	GameObject::Sptr Car1 = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		Car1->SetPostion(pos);
		Car1->SetRotation(rot);
		Car1->SetScale(scale);

		//Add Components
		Car1->Add<ForeGroundMover>();

		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Car1->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Is background Object and therefore has no colliders
		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = Car1->Add<RigidBody>(RigidBodyType::Kinematic);

		if (parent != nullptr) {
			parent->AddChild(Car1);

		}
	}
}

void SpawnBackGroundBuilding(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), GameObject::Sptr parent = nullptr) {

	GameObject::Sptr KBuilding = scene->CreateGameObject(ObjName);
	{
		// Set position in the scene
		KBuilding->SetPostion(pos);
		KBuilding->SetRotation(rot);
		KBuilding->SetScale(scale);

		//Add Components
		// Background mover ment for cars
		KBuilding->Add<BackgroundBuildingMover>();

		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = KBuilding->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(Material);

		// Is background Object and therefore has no colliders
		// Add a dynamic rigid body to this object
		RigidBody::Sptr physics = KBuilding->Add<RigidBody>(RigidBodyType::Kinematic);

		if (parent != nullptr) {
			parent->AddChild(KBuilding);

		}
	}
}


void CreateScene() {
	bool loadScene = false;

	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene) {
		ResourceManager::LoadManifest("manifest.json");
		scene = Scene::Load("scene.json");

		// Call scene awake to start up all of our components
		scene->Window = window;
		scene->Awake();
	}
	else {
		// This time we'll have 2 different shaders, and share data between both of them using the UBO
		// This shader will handle reflective materials 
		Shader::Sptr reflectiveShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_environment_reflective.glsl" }
		});

		// This shader handles our basic materials without reflections (cause they expensive)
		Shader::Sptr basicShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_blinn_phong_textured.glsl" }
		});

		// This shader handles our basic materials without reflections (cause they expensive)
		//Shader::Sptr specShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
		//	{ ShaderPartType::Vertex, "shaders/vertex_shaders/water-vert.glsl" },
		//	{ ShaderPartType::Fragment, "shaders/fragment_shaders/water-frag.glsl" }
		//});


		///////////////////// NEW SHADERS ////////////////////////////////////////////

		// This shader handles our foliage vertex shader example
		Shader::Sptr foliageShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/foliage.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/screendoor_transparency.glsl" }
		});

		// This shader handles our cel shading example
		Shader::Sptr toonShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/toon_shading.glsl" }
		});

		//Meshes
		MeshResource::Sptr SmallPlatform = ResourceManager::CreateAsset<MeshResource>("SmallSpeakerPlatformV5.obj");
		MeshResource::Sptr WallJump = ResourceManager::CreateAsset<MeshResource>("WallJumpV6.obj");
		MeshResource::Sptr BeatGem = ResourceManager::CreateAsset<MeshResource>("Gem.obj");
		MeshResource::Sptr Vinyl = ResourceManager::CreateAsset<MeshResource>("VinylV2.obj");
		MeshResource::Sptr CD = ResourceManager::CreateAsset<MeshResource>("CDwithUnwrap.obj");
	//	MeshResource::Sptr TutorialSign = ResourceManager::CreateAsset<MeshResource>("TutorialSign.obj");
		MeshResource::Sptr Building = ResourceManager::CreateAsset<MeshResource>("RBuilding.obj");
		MeshResource::Sptr Building2 = ResourceManager::CreateAsset<MeshResource>("RBuilding.obj");
		MeshResource::Sptr Building3 = ResourceManager::CreateAsset<MeshResource>("RBuilding.obj");
		MeshResource::Sptr KBuilding1Mesh = ResourceManager::CreateAsset<MeshResource>("KBuilding.obj");
		MeshResource::Sptr CharacterMesh = ResourceManager::CreateAsset<MeshResource>("dudeCharacter.obj");
		MeshResource::Sptr DiscoBallMesh = ResourceManager::CreateAsset<MeshResource>("DiscoBall2.obj");
		MeshResource::Sptr StartPlatform = ResourceManager::CreateAsset<MeshResource>("StartPlatformV6.obj");
		MeshResource::Sptr Car1Mesh = ResourceManager::CreateAsset<MeshResource>("FutureCar1.obj");
		MeshResource::Sptr SemiTruckMesh = ResourceManager::CreateAsset<MeshResource>("Semitruck.obj");
		MeshResource::Sptr PickupTruckMesh = ResourceManager::CreateAsset<MeshResource>("FuturePickup.obj");
		MeshResource::Sptr SmallWallJump = ResourceManager::CreateAsset<MeshResource>("SmallWallJump.obj");
		//change back to super small walljump later
		MeshResource::Sptr SuperSmallWallJump = ResourceManager::CreateAsset<MeshResource>("WallJumpV6.obj");
		MeshResource::Sptr FallingPlat = ResourceManager::CreateAsset<MeshResource>("pianoplatform.obj");
		MeshResource::Sptr HalfCirclePlat = ResourceManager::CreateAsset<MeshResource>("HalfCriclePlat.obj");

		//Textures
		Texture2D::Sptr StartTex = ResourceManager::CreateAsset<Texture2D>("textures/DiscoBuildingTex.png"); 
		Texture2D::Sptr SmallTex = ResourceManager::CreateAsset<Texture2D>("textures/DanceFloorTex2.png"); 
		Texture2D::Sptr VinylTex = ResourceManager::CreateAsset<Texture2D>("textures/VinylTex.png");
		Texture2D::Sptr CDTex = ResourceManager::CreateAsset<Texture2D>("textures/CDTex.png");
		Texture2D::Sptr GemTex = ResourceManager::CreateAsset<Texture2D>("textures/Gem.png"); 
	//	Texture2D::Sptr TutorialSignTex = ResourceManager::CreateAsset<Texture2D>("textures/TutorialSign.png"); 
		Texture2D::Sptr CharacterTex = ResourceManager::CreateAsset<Texture2D>("textures/shirt.png");
		Texture2D::Sptr LoseScreenTex = ResourceManager::CreateAsset<Texture2D>("textures/Game_Over_Screen.png");
		Texture2D::Sptr SmallWallJumpTex = ResourceManager::CreateAsset<Texture2D>("textures/SmallWallJumpTexBlue.png");
		Texture2D::Sptr SuperSmallWallJumpTex = ResourceManager::CreateAsset<Texture2D>("textures/SmallWallJumpTexRed.png");
		Texture2D::Sptr WallJumpTex = ResourceManager::CreateAsset<Texture2D>("textures/WallJumpTex.png");
		Texture2D::Sptr Car1Tex = ResourceManager::CreateAsset<Texture2D>("textures/FutureCarTex.png");
		Texture2D::Sptr SemiTruckTex = ResourceManager::CreateAsset<Texture2D>("textures/SemiTruckTexV2.png");
		Texture2D::Sptr PickupTruckTex = ResourceManager::CreateAsset<Texture2D>("textures/PickupTruckTex.png");
		Texture2D::Sptr BuildingTex = ResourceManager::CreateAsset<Texture2D>("textures/Building.png");
		Texture2D::Sptr KBuilding1Tex = ResourceManager::CreateAsset<Texture2D>("textures/KBuildingTex.png");
		Texture2D::Sptr DiscoBallTex = ResourceManager::CreateAsset<Texture2D>("textures/DiscoBallTexV2.png");
		Texture2D::Sptr FallingPlatTex = ResourceManager::CreateAsset<Texture2D>("textures/pianotex.png");
		Texture2D::Sptr HalfCirclePlatTex = ResourceManager::CreateAsset<Texture2D>("textures/halfCircleTex.png");
		Texture2D::Sptr UITex = ResourceManager::CreateAsset<Texture2D>("textures/UI.png");

		//Minification and Magnification
		//leafTex->SetMinFilter(MinFilter::Nearest);
		//leafTex->SetMagFilter(MagFilter::Nearest);

		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/space/space.png");
		Shader::Sptr      skyboxShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" }
		});

		// Create an empty scene
		scene = std::make_shared<Scene>();

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap);
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		// Create our materials
		Material::Sptr StartPlatformMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			StartPlatformMaterial->Name = "StartPlatform";
			StartPlatformMaterial->Set("u_Material.Diffuse", StartTex);
			StartPlatformMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr UIMat = ResourceManager::CreateAsset<Material>(basicShader);
		{
			UIMat->Name = "UIButton";
			UIMat->Set("u_Material.Diffuse", StartTex);
			UIMat->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr SmallPlatformMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			SmallPlatformMaterial->Name = "SmallPlatform";
			SmallPlatformMaterial->Set("u_Material.Diffuse", SmallTex);
			SmallPlatformMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr WallJumpMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			WallJumpMaterial->Name = "WallJump";
			WallJumpMaterial->Set("u_Material.Diffuse", WallJumpTex);
			WallJumpMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr BeatGemMaterial = ResourceManager::CreateAsset<Material>(reflectiveShader);
		{
			BeatGemMaterial->Name = "BeatGem";
			BeatGemMaterial->Set("u_Material.Diffuse", GemTex);
			BeatGemMaterial->Set("u_Material.Shininess", 1.0f);
		}

		Material::Sptr VinylMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			VinylMaterial->Name = "Vinyl";
			VinylMaterial->Set("u_Material.Diffuse", VinylTex);
			VinylMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr CDMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			CDMaterial->Name = "CD";
			CDMaterial->Set("u_Material.Diffuse", CDTex);
			CDMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr CharacterMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			CharacterMaterial->Name = "Character";
			CharacterMaterial->Set("u_Material.Diffuse", CharacterTex);
			CharacterMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr DiscoBallMaterial = ResourceManager::CreateAsset<Material>(reflectiveShader);
		{
			DiscoBallMaterial->Name = "DiscoBall";
			DiscoBallMaterial->Set("u_Material.Diffuse", DiscoBallTex);
			DiscoBallMaterial->Set("u_Material.Shininess", -0.4f);
		}

		Material::Sptr LoseScreenMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			LoseScreenMaterial->Name = "Lose Screen";
			LoseScreenMaterial->Set("u_Material.Diffuse", LoseScreenTex);
			LoseScreenMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr Car1Material = ResourceManager::CreateAsset<Material>(basicShader);
		{
			Car1Material->Name = "Car1";
			Car1Material->Set("u_Material.Diffuse", Car1Tex);
			Car1Material->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr SemiTruckMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			SemiTruckMaterial->Name = "Semi1";
			SemiTruckMaterial->Set("u_Material.Diffuse", SemiTruckTex);
			SemiTruckMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr PickupTruckMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			PickupTruckMaterial->Name = "Pickup1";
			PickupTruckMaterial->Set("u_Material.Diffuse", PickupTruckTex);
			PickupTruckMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr BuildingMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			BuildingMaterial->Name = "Building";
			BuildingMaterial->Set("u_Material.Diffuse", BuildingTex);
			BuildingMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr KBuildingMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			KBuildingMaterial->Name = "KBuilding";
			KBuildingMaterial->Set("u_Material.Diffuse", KBuilding1Tex);
			KBuildingMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr SmallWallJumpMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			SmallWallJumpMaterial->Name = "Small Wall Jump";
			SmallWallJumpMaterial->Set("u_Material.Diffuse", SmallWallJumpTex);
			SmallWallJumpMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr SuperSmallWallJumpMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			SuperSmallWallJumpMaterial->Name = "Super Small Wall Jump";
			SuperSmallWallJumpMaterial->Set("u_Material.Diffuse", SuperSmallWallJumpTex);
			SuperSmallWallJumpMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr PianoMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			PianoMaterial->Name = "Piano";
			PianoMaterial->Set("u_Material.Diffuse", FallingPlatTex);
			PianoMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr HalfCirclePlatMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			HalfCirclePlatMaterial->Name = "Half Circle Plat";
			HalfCirclePlatMaterial->Set("u_Material.Diffuse", HalfCirclePlatTex);
			HalfCirclePlatMaterial->Set("u_Material.Shininess", 0.1f);
		}

		// Create some lights for our scene
		scene->Lights.resize(2);
		scene->Lights[0].Position = glm::vec3(0.0f, 1.0f, 3.0f);
		scene->Lights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[0].Range = 100.0f;



		// Set up the scene's camera
		GameObject::Sptr camera = scene->CreateGameObject("Main Camera");
		{
			camera->SetPostion(glm::vec3(-1.410, -3.500, 2.450));
			camera->LookAt(glm::vec3(0.0f));
			camera->SetRotation(glm::vec3(-103, 180, -180));

			Camera::Sptr cam = camera->Add<Camera>();
			//cam->SetOrthoEnabled(true);
			//cam->SetOrthoVerticalScale(19.0f);
			//cam->SetFovRadians(105.f);
			//cam->SetNearPlane(0.3);

			// Make sure that the camera is set as the scene's main camera!
			scene->MainCamera = cam;
		}

		//MAIN BLOCK GAMEOBJECTS
		//Move these if you want to move entire blocks 
		//It's Good practice to place these at the approximate centre of each block

		GameObject::Sptr TutorialBlock = scene->CreateGameObject("TutorialBlock");
		GameObject::Sptr Block1 = scene->CreateGameObject("Block1");
		GameObject::Sptr Block2 = scene->CreateGameObject("Block2");
		GameObject::Sptr Block3 = scene->CreateGameObject("Block3");
		GameObject::Sptr Block4 = scene->CreateGameObject("Block4");
		GameObject::Sptr Block5 = scene->CreateGameObject("Block5");
		GameObject::Sptr Block6 = scene->CreateGameObject("Block6");
		GameObject::Sptr Block7 = scene->CreateGameObject("Block7");
		GameObject::Sptr Block8 = scene->CreateGameObject("Block8");
		{
		//Block1->Add<LevelMover>();
		//Block2->Add<LevelMover>();
		//Block3->Add<LevelMover>();
		//Block4->Add<LevelMover>();
		//Block5->Add<LevelMover>();
		//Block6->Add<LevelMover>();
		//Block7->Add<LevelMover>();
		//Block8->Add<LevelMover>();
		//TutorialBlock->Add<LevelMover>();

			//These have rigid bodies so that our objects can move
			auto rb1 = Block1->Add<RigidBody>();
			auto rb2 = Block2->Add<RigidBody>();
			auto rb3 = Block3->Add<RigidBody>();
			auto rb4 = Block4->Add<RigidBody>();
			auto rb5 = Block5->Add<RigidBody>();
			auto rb6 = Block6->Add<RigidBody>();
			auto rb7 = Block7->Add<RigidBody>();
			auto rb8 = Block8->Add<RigidBody>();
			auto rbt = TutorialBlock->Add<RigidBody>();
		}

		//This is a game object built purely to manage game systems i.e. Scene Swaps
		//Level Spawning, Score Counting, and a few miscellaneous systems
		GameObject::Sptr GameManager = scene->CreateGameObject("GameManager");
		{
			//Pos-Rot-Scale Doesn't matter

			//ScoreComponent
			//LevelSpawningComponent
			//Scene Swapper

		}
	

		// Tutorial
		
		// Background and forground vehicles\\
		Give these Parents for Foreground/Background Blocks if we have enough objects to do that with!
		SpawnBackGroundCar(Car1Mesh, Car1Material, "Car1", glm::vec3(14.870f, 7.80f, 2.7f), glm::vec3(90.0f, 0.0f, -90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnBackGroundCar(SemiTruckMesh, SemiTruckMaterial, "Semi1", glm::vec3(28.870f, 7.80f, 2.7f), glm::vec3(90.0f, 0.0f, -90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnForeGroundCar(Car1Mesh, Car1Material, "Car2", glm::vec3(-9.970f, 0.470f, -1.90f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnForeGroundCar(PickupTruckMesh, PickupTruckMaterial, "Pickup1", glm::vec3(-18.970f, 0.470f, -1.90f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnBackGroundBuilding(KBuilding1Mesh, KBuildingMaterial, "KBuilding1", glm::vec3(0.0f, 21.880f, -36.040f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.780f, 1.470f, 1.0f));

		//Give those tutorial Signs Textures
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-9.820f, 5.610f, -4.450), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), TutorialBlock);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform 1", glm::vec3(-6.070f, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), TutorialBlock);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform 2", glm::vec3(-3.320f, 5.610f, -2.200f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), TutorialBlock);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform 3", glm::vec3(-0.400f, 5.610f, -4.040f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), TutorialBlock);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform 4", glm::vec3(-0.060f, 5.610f, 4.450f),  glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), TutorialBlock);
		//SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump 1", glm::vec3(1.680f, 5.610f, 2.610f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), TutorialBlock);
		//SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump 2", glm::vec3(4.350f, 5.610f, 3.930f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), TutorialBlock);
		//SpawnGem(BeatGem, BeatGemMaterial, "BeatGem", glm::vec3(2.410f, 5.610f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f), TutorialBlock);
		//SpawnCollectable(Vinyl, VinylMaterial, "Vinyl", glm::vec3(-0.040f, 5.610f, 5.120f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), TutorialBlock);
		////SpawnObj(TutorialSign, TutorialSignMaterial, "Tutorial Sign 1", glm::vec3(-9.770f, 5.690f, -3.890f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.310f, 0.310f, 0.310f));
		////SpawnObj(TutorialSign, TutorialSignMaterial, "Tutorial Sign 2", glm::vec3(-0.390f, 5.690f, -3.440f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.310f, 0.310f, 0.310f));
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(6.360f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), TutorialBlock);
		//SpawnBackGroundCar(Car1Mesh, Car1Material, "Car1", glm::vec3(14.870f, 9.80f, 2.7f), glm::vec3(90.0f, 0.0f, -90.0f), glm::vec3(0.250f, 0.250f, 0.250f), TutorialBlock);
		//SpawnBackGroundCar(SemiTruckMesh, SemiTruckMaterial, "Semi1", glm::vec3(28.870f, 9.80f, 2.7f), glm::vec3(90.0f, 0.0f, -90.0f), glm::vec3(0.250f, 0.250f, 0.250f), TutorialBlock);
		//SpawnForeGroundCar(Car1Mesh, Car1Material, "Car2", glm::vec3(-9.970f, 0.470f, -1.90f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.250f, 0.250f, 0.250f), TutorialBlock);
		//SpawnForeGroundCar(PickupTruckMesh, PickupTruckMaterial, "Pickup1", glm::vec3(-18.970f, 0.470f, -1.90f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.250f, 0.250f, 0.250f), TutorialBlock);
		

		// 1st Block
		
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform Block1", glm::vec3(-9.820f, 5.610f, -4.450), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block1);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block1 1", glm::vec3(-6.070f, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block1);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block1 2", glm::vec3(-2.840f, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block1);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block1 3", glm::vec3(2.760f, 5.610f, -1.770f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block1);
		//SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block1", glm::vec3(0.120f, 5.610f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f), Block1);
		//SpawnCollectable(Vinyl, VinylMaterial, "Vinyl Block1", glm::vec3(5.640f, 5.610f, 0.080f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block1);
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform Block1", glm::vec3(6.360f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block1);
		//
		//// CDs for Block 1
		//SpawnCD(CD, CDMaterial, "CD Block1 1", glm::vec3(-6.030f, 5.610f, -3.220f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block1);
		//SpawnCD(CD, CDMaterial, "CD Block1 2", glm::vec3(-2.710f, 5.610f, -3.190f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block1);
		//SpawnCD(CD, CDMaterial, "CD Block1 3", glm::vec3(0.170f, 5.610f, -2.380f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block1);
		//SpawnCD(CD, CDMaterial, "CD Block1 4", glm::vec3(2.640f, 5.610f, -0.770f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block1);
		
		
		// 2nd Block
		
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform Block2", glm::vec3(-9.820f, 5.610f, -4.450), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block2);
		//SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump Block2 1", glm::vec3(-8.590f, 5.610f, 3.210f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.040f, 1.500f), Block2);
		//SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump Block2 2", glm::vec3(-6.660f, 5.610f, 2.000f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.040f, 1.500f), Block2);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block2 1", glm::vec3(-4.400f, 5.610f, 4.000f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block2);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block2 2", glm::vec3(1.940f, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block2);
		//SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block2", glm::vec3(-1.340f, 5.610f, 0.500f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f), Block2);
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform Block2", glm::vec3(6.360f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block2);
		//
		//// CDs for Block 2
		//SpawnCD(CD, CDMaterial, "CD Block2 1", glm::vec3(-7.720f, 5.610f, -0.030f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block2);
		//SpawnCD(CD, CDMaterial, "CD Block2 2", glm::vec3(-7.720f, 5.610f, 2.130f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block2);
		//SpawnCD(CD, CDMaterial, "CD Block2 3", glm::vec3(-7.720f, 5.610f, 4.610f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block2);
		//SpawnCD(CD, CDMaterial, "CD Block2 4", glm::vec3(-4.420f, 5.610f, 4.750f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block2);
		//SpawnCD(CD, CDMaterial, "CD Block2 5", glm::vec3(-1.340f, 5.610f, 0.810f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block2);
		//SpawnCD(CD, CDMaterial, "CD Block2 6", glm::vec3(1.920f, 5.610f, -3.610f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block2);
		

		// 3rd Block
		
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform Block3", glm::vec3(-9.820f, 5.610f, -4.450), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block3);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block3 1", glm::vec3(-4.360f, 5.610f, -0.290f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block2);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block3 2", glm::vec3(0.350f, 5.610f, -0.290f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block2);
		//SpawnObj(FallingPlat, PianoMaterial, "Falling Platform", glm::vec3(0.390f, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block3);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block3 4", glm::vec3(3.220f, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block3);
		//SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block3 1", glm::vec3(-6.870f, 5.610f, -1.970f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f), Block3);
		//SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block3 2", glm::vec3(-1.870f, 5.610f, -1.970f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f), Block3);
		//SpawnCollectable(Vinyl, VinylMaterial, "Vinyl Block3", glm::vec3(0.370f, 5.610f, -2.830f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block3);
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform Block3", glm::vec3(6.360f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block3);
		//
		//// CDs for Block 3
		//SpawnCD(CD, CDMaterial, "CD Block3 1", glm::vec3(-4.390f, 5.610f, 0.440f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block3);
		//SpawnCD(CD, CDMaterial, "CD Block3 2", glm::vec3(0.350f, 5.610f, 0.290f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block3);
		//SpawnCD(CD, CDMaterial, "CD Block3 3", glm::vec3(3.260f, 5.610f, -3.210f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block3);
		//SpawnCD(CD, CDMaterial, "CD Block3 4", glm::vec3(-6.690f, 5.610f, -1.180f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block3);
		

		// 4th Block
		
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform Block4", glm::vec3(-9.820f, 5.610f, -4.450), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block4);
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block4 1", glm::vec3(-6.540f, 5.610f, -4.220f),  glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block4);
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block4 2", glm::vec3(-3.640f, 5.610f, -4.220f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block4);
		SpawnObj(HalfCirclePlat, HalfCirclePlatMaterial, "Half Circle Platform", glm::vec3(-0.720f, 5.610f, -4.220f),  glm::vec3(-90.000f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f), Block4);
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block4 4", glm::vec3(2.290f, 5.610f, 4.700f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block4);
		SpawnBuilding(Building, BuildingMaterial, "Building Block4", glm::vec3(4.150f, 5.610f, -7.110f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block4);
		SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump Block4 1", glm::vec3(-1.590f, 5.610f, 2.650f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block4);
		SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump Block4 2", glm::vec3(0.460f, 5.610f, 1.610), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block4);
		SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block4 2", glm::vec3(1.770f, 5.610f, -3.520f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f), Block4);
		SpawnCollectable(Vinyl, VinylMaterial, "Vinyl Block4", glm::vec3(2.190f, 5.610f, 5.390f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block4);
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform Block3", glm::vec3(6.840f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block4);
		

		// 5th Block
		
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform Block5", glm::vec3(-9.820f, 5.610f, -4.450), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block5);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block5 1", glm::vec3(-6.540f, 5.610f, -4.220f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block5);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block5 2", glm::vec3(-5.000f, 5.610f, -2.830f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block5);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block5 3", glm::vec3(-3.550f, 5.610f, -1.410f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block5);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block5 4", glm::vec3(-6.450f, 5.610f, -0.730f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block5);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block5 5", glm::vec3(-3.330f, 5.610f, 5.950f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block5);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block5 6", glm::vec3(2.280f, 5.610f, 4.180f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block5);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block5 7", glm::vec3(2.280f, 5.610f, -4.010f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block5);
		//SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump Block5 1", glm::vec3(-6.730f, 5.610f, 4.460f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block5);
		//SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump Block5 2", glm::vec3(-5.030f, 5.610f, 4.110f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block5);
		//SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump Block5 3", glm::vec3(-1.590f, 5.610f, 2.650f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block5);
		//SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump Block5 4", glm::vec3(0.460f, 5.610f, 1.610f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block5);
		//SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block5 1", glm::vec3(-0.580f, 5.610f, -1.970f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f), Block5);
		//SpawnCollectable(Vinyl, VinylMaterial, "Vinyl Block5", glm::vec3(2.190f, 5.610f, 5.390f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block5);
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform Block5", glm::vec3(6.840f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block5);
		//
		//
		//// 6th Block
		//
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform Block6", glm::vec3(-9.820f, 5.610f, -4.450), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block6);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block6 1", glm::vec3(-6.540f, 5.610f, -4.220f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block6);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block6 2", glm::vec3(-5.000f, 5.610f, -2.830f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block6);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block6 3", glm::vec3(-3.550f, 5.610f, -1.410f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block6);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block6 4", glm::vec3(4.150f, 5.610f, 2.610f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block6);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block6 5", glm::vec3(6.340f, 5.610f, 0.720f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block6);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block6 6", glm::vec3(4.220f, 5.610f, -1.110f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block6);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block6 7", glm::vec3(6.810f, 5.610f, -2.630f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block6);
		//SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump Block6 1", glm::vec3(-2.600f, 5.610f, 5.940f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block6);
		//SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump Block6 2", glm::vec3(-1.170f, 5.610f, 6.950f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block6);
		//SpawnBuilding(Building, BuildingMaterial, "Building Block6 1", glm::vec3(-1.830f, 5.610f, -5.420f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block6);
		//SpawnBuilding(Building, BuildingMaterial, "Building Block6 2", glm::vec3(2.270f, 5.610f, -3.810f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block6); // Make this building bigger
		//SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block6 1", glm::vec3(-3.380f, 5.610f, 0.630f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f), Block6);
		//SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block6 2", glm::vec3(0.030f, 5.610f, 2.410f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f), Block6);
		//SpawnCollectable(Vinyl, VinylMaterial, "Vinyl Block6", glm::vec3(-1.890f, 5.610f, 5.390f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block6);
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform Block6", glm::vec3(6.840f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block6);
		//
		//
		//// 7th Block
		//
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform Block7", glm::vec3(-9.820f, 5.610f, -4.450), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block7);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block7 1", glm::vec3(-4.170f, 5.610f, 2.210f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block7);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block7 2", glm::vec3(-0.810f, 5.610f, 2.270f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block7);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block7 3", glm::vec3(2.440f, 5.610f, 2.260f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block7);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block7 4", glm::vec3(-2.610f, 5.610f, -0.240f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block7);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block7 5", glm::vec3(-0.100f, 5.610f, -1.110f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block7);
		//SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump Block7 1", glm::vec3(-8.210f, 5.610f, 2.050f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block7);
		//SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump Block7 2", glm::vec3(-5.780f, 5.610f, 0.380f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block7);
		//SpawnBuilding(Building, BuildingMaterial, "Building Block7 1", glm::vec3(4.130f, 5.610f, -3.610f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f)); // Make this building bigger
		//SpawnGem(BeatGem, BeatGemMaterial, "BeatGem Block7 1", glm::vec3(0.860f, 5.610f, 0.630f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f), Block7);
		//SpawnCollectable(Vinyl, VinylMaterial, "Vinyl Block7", glm::vec3(-0.180f, 5.610f, -0.330f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f), Block7);
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform Block7", glm::vec3(6.840f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block7);
		
		
		// 8th Block
		
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform Block8", glm::vec3(-9.820f, 5.610f, -4.450), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block8);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block8 1", glm::vec3(-6.640f, 5.610f, -4.140f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block8);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block8 2", glm::vec3(-4.430f, 5.610f, -3.310f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block8);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block8 3", glm::vec3(-0.540f, 5.610f, -6.090f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block8);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block8 4", glm::vec3(1.540f, 5.610f, -5.310f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block8);
		//SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform Block8 5", glm::vec3(4.320f, 5.610f, 0.860f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block8);
		//SpawnBuilding(Building, BuildingMaterial, "Building Block8 1", glm::vec3(4.110f, 5.610f, -6.630f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block8);
		//SpawnBuilding2(Building2, BuildingMaterial, "Building2 Block8 1", glm::vec3(-4.050f, 5.610f, 0.900f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block8);
		//SpawnBuilding3(Building3, BuildingMaterial, "Building3 Block8 1", glm::vec3(-0.810f, 5.610f, -1.250f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.290f, 0.290f, 0.290f), Block8);
		//SpawnGem(BeatGem, BeatGemMaterial, "BeatGem", glm::vec3(-2.630f, 5.610f, -4.550f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f), Block8);
		//SpawnSuperSmallWallJump(SuperSmallWallJump, SuperSmallWallJumpMaterial, "Super Small Wall Jump Block8 1", glm::vec3(2.850f, 5.610f, -3.180f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block8);
		//SpawnSuperSmallWallJump(SuperSmallWallJump, SuperSmallWallJumpMaterial, "Super Small Wall Jump Block8 2", glm::vec3(1.520f, 5.610f, -1.510f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block8);
		//SpawnSuperSmallWallJump(SuperSmallWallJump, SuperSmallWallJumpMaterial, "Super Small Wall Jump Block8 3", glm::vec3(2.860f, 5.610f, -0.590f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block8);
		//SpawnSuperSmallWallJump(SuperSmallWallJump, SuperSmallWallJumpMaterial, "Super Small Wall Jump Block8 4", glm::vec3(1.520f, 5.610f, 0.700f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f), Block8);
		//SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform Block8", glm::vec3(6.840f, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f), Block8);
		

		// Player:
		GameObject::Sptr character = scene->CreateGameObject("Character/Player");
		{
			// Set position in the scene
			character->SetPostion(glm::vec3(-10.270f, 5.710f, -3.800f));
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

			TriggerVolume::Sptr volume = character->Add<TriggerVolume>();
			volume->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);

			BoxCollider::Sptr collider = BoxCollider::Create(glm::vec3(0.3f, 0.3f, 0.3f));
			collider->SetPosition(glm::vec3(0.f, 0.25f, 0.f));
			volume->AddCollider(collider);
		}

		GameObject::Sptr DiscoBall = scene->CreateGameObject("DiscoBall"); 
		{
			DiscoBall->SetPostion(glm::vec3(-10.270f, 5.710f, -1.0f));
			DiscoBall->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
			DiscoBall->SetScale(glm::vec3(0.2f, 0.2f, 0.2f));

			RenderComponent::Sptr renderer = DiscoBall->Add<RenderComponent>();
			renderer->SetMesh(DiscoBallMesh);
			renderer->SetMaterial(DiscoBallMaterial);

		SeekBehaviour::Sptr seeking = DiscoBall->Add<SeekBehaviour>();
		seeking->seekTo(character);

		RigidBody::Sptr ballphysics = DiscoBall->Add<RigidBody>(RigidBodyType::Dynamic);
		}

		/////////////////////////// UI //////////////////////////////
		GameObject::Sptr canvas = scene->CreateGameObject("UI Canvas");
		{
			RectTransform::Sptr transform = canvas->Add<RectTransform>();
			transform->SetMin({ 16, 16 });
			transform->SetMax({ 256, 256 });

			GuiPanel::Sptr canPanel = canvas->Add<GuiPanel>();

			GameObject::Sptr subPanel = scene->CreateGameObject("Sub Item");
			{
				RectTransform::Sptr transform = subPanel->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 128, 128 });

				GuiPanel::Sptr panel = subPanel->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 16.0f);
				font->Bake();

				GuiText::Sptr text = subPanel->Add<GuiText>();
				text->SetText("Hello world!");
				text->SetFont(font);
			}

			canvas->AddChild(subPanel);
		}

		GuiBatcher::SetDefaultTexture(ResourceManager::CreateAsset<Texture2D>("textures/ui-sprite.png"));
		GuiBatcher::SetDefaultBorderRadius(8);

		// Call scene awake to start up all of our components
		scene->Window = window;
		scene->Awake();

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");

	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		scene->IsPlaying = !scene->IsPlaying;
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
	//ComponentManager::RegisterType<MouseController>();
	ComponentManager::RegisterType<SeekBehaviour>();
	ComponentManager::RegisterType<RotatingBehaviour>();
	ComponentManager::RegisterType<RotatingBehaviourCD>();
	ComponentManager::RegisterType<CharacterController>();
	ComponentManager::RegisterType<JumpBehaviour>();
	ComponentManager::RegisterType<ScoreComponent>();
	ComponentManager::RegisterType<MaterialSwapBehaviour>();
	ComponentManager::RegisterType<LevelMover>();
	ComponentManager::RegisterType<BackgroundMover>();
	ComponentManager::RegisterType<BackgroundBuildingMover>();
	ComponentManager::RegisterType<VinylAnim>();
	ComponentManager::RegisterType<ForeGroundMover>();
	ComponentManager::RegisterType<RectTransform>();
	ComponentManager::RegisterType<GuiPanel>();
	ComponentManager::RegisterType<GuiText>();


	// GL states, we'll enable depth testing and backface fulling
	// GL states, we'll enable depth testing and backface fulling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	// Structure for our frame-level uniforms, matches layout from
	// fragments/frame_uniforms.glsl
	// For use with a UBO.
	struct FrameLevelUniforms {
		// The camera's view matrix
		glm::mat4 u_View;
		// The camera's projection matrix
		glm::mat4 u_Projection;
		// The combined viewProject matrix
		glm::mat4 u_ViewProjection;
		// The camera's position in world space
		glm::vec4 u_CameraPos;
		// The time in seconds since the start of the application
		float u_Time;
	};
	// This uniform buffer will hold all our frame level uniforms, to be shared between shaders
	UniformBuffer<FrameLevelUniforms>::Sptr frameUniforms = std::make_shared<UniformBuffer<FrameLevelUniforms>>(BufferUsage::DynamicDraw);
	// The slot that we'll bind our frame level UBO to
	const int FRAME_UBO_BINDING = 0;

	// Structure for our isntance-level uniforms, matches layout from
	// fragments/frame_uniforms.glsl
	// For use with a UBO.
	struct InstanceLevelUniforms {
		// Complete MVP
		glm::mat4 u_ModelViewProjection;
		// Just the model transform, we'll do worldspace lighting
		glm::mat4 u_Model;
		// Normal Matrix for transforming normals
		glm::mat4 u_NormalMatrix;
	};

	// This uniform buffer will hold all our instance level uniforms, to be shared between shaders
	UniformBuffer<InstanceLevelUniforms>::Sptr instanceUniforms = std::make_shared<UniformBuffer<InstanceLevelUniforms>>(BufferUsage::DynamicDraw);

	// The slot that we'll bind our instance level UBO to
	const int INSTANCE_UBO_BINDING = 1;

	////////////////////////////////
	///// SCENE CREATION MOVED /////
	////////////////////////////////
	CreateScene();

	// We'll use this to allow editing the save/load path
	// via ImGui, note the reserve to allocate extra space
	// for input!
	std::string scenePath = "scene.json";
	scenePath.reserve(256);
	scene->SetAmbientLight(glm::vec3(0.25f));

	// Our high-precision timer
	double lastFrame = glfwGetTime();

	BulletDebugMode physicsDebugMode = BulletDebugMode::None;
	float playbackSpeed = 1.0f;

	nlohmann::json editorSceneState;
	int _Pause;
	
	ToneFire::FMODStudio studio;

	studio.LoadBank("Master.bank");
	studio.LoadBank("Master.strings.bank");
	studio.LoadBank("Level1.bank");




	ToneFire::StudioSound test;
	test.LoadEvent("event:/Music");
	test.SetEventPosition("event:/Music", FMOD_VECTOR{ -10.270f, 5.710f, -3.800f });
	//test.PlayEvent("event:/Music");
	//test.SetEventParameter("event:/Music", "Volume", 0.5f);
	
	///// Game loop /////
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		ImGuiHelper::StartFrame();
		studio.Update();

		// Calculate the time since our last frame (dt)
		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);

		// Draw our material properties window!
		DrawMaterialsWindow();

		//Player Quick Pause Functionality
		glfwSetKeyCallback(window, key_callback);

		// Showcasing how to use the imGui library!
		bool isDebugWindowOpen = ImGui::Begin("Debugging");
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

		// Bind the skybox texture to a reserved texture slot
		// See Material.h and Material.cpp for how we're reserving texture slots
		TextureCube::Sptr environment = scene->GetSkyboxTexture();
		if (environment) environment->Bind(0);

		// Here we'll bind all the UBOs to their corresponding slots
		scene->PreRender();
		frameUniforms->Bind(FRAME_UBO_BINDING);
		instanceUniforms->Bind(INSTANCE_UBO_BINDING);

		// Upload frame level uniforms
		auto& frameData = frameUniforms->GetData();
		frameData.u_Projection = camera->GetProjection();
		frameData.u_View = camera->GetView();
		frameData.u_ViewProjection = camera->GetViewProjection();
		frameData.u_CameraPos = glm::vec4(camera->GetGameObject()->GetPosition(), 1.0f);
		frameData.u_Time = static_cast<float>(thisFrame);
		frameUniforms->Update();

		// Render all our objects
		ComponentManager::Each<RenderComponent>([&](const RenderComponent::Sptr& renderable) {
			// Early bail if mesh not set
			if (renderable->GetMesh() == nullptr) {
				return;
			}

			// If we don't have a material, try getting the scene's fallback material
			// If none exists, do not draw anything
			if (renderable->GetMaterial() == nullptr) {
				if (scene->DefaultMaterial != nullptr) {
					renderable->SetMaterial(scene->DefaultMaterial);
				}
				else {
					return;
				}
			}

			// If the material has changed, we need to bind the new shader and set up our material and frame data
			// Note: This is a good reason why we should be sorting the render components in ComponentManager
			if (renderable->GetMaterial() != currentMat) {
				currentMat = renderable->GetMaterial();
				shader = currentMat->GetShader();

				shader->Bind();
				currentMat->Apply();
			}

			// Grab the game object so we can do some stuff with it
			GameObject* object = renderable->GetGameObject();

			// Use our uniform buffer for our instance level uniforms
			auto& instanceData = instanceUniforms->GetData();
			instanceData.u_Model = object->GetTransform();
			instanceData.u_ModelViewProjection = viewProj * object->GetTransform();
			instanceData.u_NormalMatrix = glm::mat3(glm::transpose(glm::inverse(object->GetTransform())));
			instanceUniforms->Update();

			// Draw the object
			renderable->GetMesh()->Draw();
			});

		// Use our cubemap to draw our skybox
		scene->DrawSkybox();
		
		// Disable culling
		glDisable(GL_CULL_FACE);

		// Disable depth testing, we're going to use order-dependant layering
		//glDisable(GL_DEPTH_TEST);
		// Disable depth writing
		//glDepthMask(GL_FALSE);
		
		// Enable alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		// Enable the scissor test;
		glEnable(GL_SCISSOR_TEST);
		
		// Our projection matrix will be our entire window for now
			glm::mat4 proj = glm::ortho(0.0f, (float)windowSize.x, (float)windowSize.y, 0.0f, -1.0f, 1.0f);
		GuiBatcher::SetProjection(proj);

		scene->RenderGUI();

		// Flush the Gui Batch renderer
		GuiBatcher::Flush();

		// Disable alpha blending
		glDisable(GL_BLEND);
		// Disable scissor testing
		glDisable(GL_SCISSOR_TEST);
		// Re-enable depth writing
		glDepthMask(GL_TRUE);

		// End our ImGui window
		ImGui::End();

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
