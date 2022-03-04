

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
#include "Gameplay/Components/BeatTimer.h"
#include "Gameplay/Components/BuildingAnim.h"
#include "Gameplay/Components/MaterialSwap.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

// Animations
#include "Gameplay/Animation/MorphAnimator.h"
#include "Gameplay/Animation/MorphMeshRenderer.h"
//Sound
#include "Sound/AudioEngine.h"
#include "Fmod.h"
#include "FMOD/ToneFire.h"
#include "FMOD/SoundEffects.h"

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
glm::ivec2 windowSize = glm::ivec2(1920, 1080);
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
void SpawnObj(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "Plat", glm::vec3 pos = glm::vec3(-10.900f, 5.610f, -4.920f), 
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
		physics->AddCollider(BoxCollider::Create(glm::vec3(1.8f, 5.2f, 1.0f)));

		if (parent != nullptr) {
			parent->AddChild(Startplatform);
		}

	}
}
// for spawning Beat Gems
void SpawnGem(MeshResource::Sptr Mesh, Material::Sptr MaterialOn, Material::Sptr MaterialOff, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
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
		Startplatform->Add<MaterialSwap>();

		
		
		// Create and attach a renderer for the Object
		RenderComponent::Sptr renderer = Startplatform->Add<RenderComponent>();
		renderer->SetMesh(Mesh);
		renderer->SetMaterial(MaterialOff);

		TriggerVolume::Sptr volume = Startplatform->Add<TriggerVolume>();
		volume->AddCollider(BoxCollider::Create(glm::vec3(0.5f, 0.5f, 0.5f)));
		//volume->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);

		// Add a dynamic rigid body to this object
		//RigidBody::Sptr physics = Startplatform->Add<RigidBody>(RigidBodyType::Kinematic);
		//physics->AddCollider(BoxCollider::Create(glm::vec3(0.5f, 0.5f, 0.5f)));
		// For Gem Colliders X = left/right Y = Up/Down Z = Towards/Away
		


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
		//RigidBody::Sptr physics = Startplatform->Add<RigidBody>(RigidBodyType::Kinematic);
		// For Colliders X is towards Cam, Y is up/down , Z is Left and Right
		//ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(0.5f, 0.5f, 0.5f)));
		
		TriggerVolume::Sptr volume = Startplatform->Add<TriggerVolume>();
		volume->AddCollider(BoxCollider::Create(glm::vec3(0.5f, 0.5f, 0.5f)));
		//volume->SetPostion(glm::vec3(0.0f, 0.5f, 0.0f));

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

		//RigidBody::Sptr physics = Startplatform->Add<RigidBody>(RigidBodyType::Kinematic);
		//physics->AddCollider(BoxCollider::Create(glm::vec3(0.5f, 0.5f, 0.5f)));

		TriggerVolume::Sptr volume = Startplatform->Add<TriggerVolume>();
		volume->AddCollider(BoxCollider::Create(glm::vec3(0.5f, 0.5f, 0.5f)));

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
		ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(1.590f, 5.000f, 2.000f)));
		CollectCollider->SetPosition(glm::vec3(-0.190f, 1.280f, -0.080f));

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
		ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(1.700f, 3.500f, 2.000f)));
		CollectCollider->SetPosition(glm::vec3(0.570f, -3.230f, 1.150f));

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
		ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(1.500f, 4.800f, 1.700f)));
		CollectCollider->SetPosition(glm::vec3(-0.040f, -0.990f, 0.000f));

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

void SpawnStairsRight(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
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
		CollectCollider->SetPosition(glm::vec3(-0.120f, 0.460f, 1.030f));
		CollectCollider->SetScale(glm::vec3(13.138, 7.218, 0.5));
		CollectCollider->SetRotation(glm::vec3(68.0f, 0.000f, 0.00f));

		if (parent != nullptr) {
			parent->AddChild(SuperSmallWallJump);

		}

	}
}

void SpawnStairsLeft(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
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
		CollectCollider->SetPosition(glm::vec3(-0.120f, 0.460f, -1.030f));
		CollectCollider->SetScale(glm::vec3(13.138, 7.218, 0.5));
		CollectCollider->SetRotation(glm::vec3(-68.0f, 0.000f, 0.00f));

		if (parent != nullptr) {
			parent->AddChild(SuperSmallWallJump);

		}

	}
}

void SpawnSpeaker(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
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

		if (parent != nullptr) {
			parent->AddChild(SuperSmallWallJump);

		}

	}
}

void SpawnSquarePlat(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
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
		ICollider::Sptr CollectCollider = physics->AddCollider(BoxCollider::Create(glm::vec3(0.5f, 0.3f, 0.4f)));
		CollectCollider->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

		if (parent != nullptr) {
			parent->AddChild(SuperSmallWallJump);

		}

	}
}

void SpawnFloatingLights(MeshResource::Sptr Mesh, Material::Sptr Material, std::string ObjName = "DeezNuts", glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
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
		KBuilding->Add<BuildingAnim>();

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
	
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	nlohmann::json editorSceneState;

	//if any key is pressed will disable these booleans. only happens once per execution.
	if (action == GLFW_PRESS) {
		scene->FindObjectByName("Movement Tutorial")->Get<GuiPanel>()->IsEnabled =false;
		scene->FindObjectByName("Vinyl Tutorial")->Get<GuiPanel>()->IsEnabled = false;
		scene->FindObjectByName("Wall Jump Tutorial")->Get<GuiPanel>()->IsEnabled = false;
		scene->FindObjectByName("Beat Gem Tutorial")->Get<GuiPanel>()->IsEnabled = false;
		scene->FindObjectByName("HUD Score Display")->Get<GuiPanel>()->IsEnabled = true;
		scene->FindObjectByName("GameOver Score Text")->Get<GuiPanel>()->IsEnabled = true;
		scene->FindObjectByName("GameOver Score Text")->Get<GuiText>()->IsEnabled = true;
	}
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		scene = Scene::FromJson(editorSceneState);
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		//toggle each of these booleans
		scene->IsPlaying = !scene->IsPlaying;
		scene->FindObjectByName("PauseMenu Dimmed Background")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("PauseMenu Dimmed Background")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("PauseMenu Background")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("PauseMenu Background")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("PauseMenu Resume Button")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("PauseMenu Resume Button")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("PauseMenu Options Button")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("PauseMenu Options Button")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("PauseMenu Quit Button")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("PauseMenu Quit Button")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("PauseMenu Resync Button")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("PauseMenu Resync Button")->Get<GuiPanel>()->IsEnabled;
	}

	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{

		//toggle each of these booleans
		scene->FindObjectByName("MainMenu Logo")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("MainMenu Logo")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("MainMenu Play Button")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("MainMenu Play Button")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("MainMenu Options Button")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("MainMenu Options Button")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("MainMenu Music Button")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("MainMenu Music Button")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("MainMenu Credits Button")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("MainMenu Credits Button")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("MainMenu Quit Button")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("MainMenu Quit Button")->Get<GuiPanel>()->IsEnabled;
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{

		//toggle each of these booleans
		scene->FindObjectByName("GameOver Dimmed Background")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("GameOver Dimmed Background")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("GameOver Text")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("GameOver Text")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("GameOver Score Breakdown")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("GameOver Score Breakdown")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("GameOver Quit Button")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("GameOver Quit Button")->Get<GuiPanel>()->IsEnabled;
		scene->FindObjectByName("GameOver Continue Button")->Get<GuiPanel>()->IsEnabled = !scene->FindObjectByName("GameOver Continue Button")->Get<GuiPanel>()->IsEnabled;
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
	ComponentManager::RegisterType<BeatTimer>();
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
	ComponentManager::RegisterType<BuildingAnim>();
	ComponentManager::RegisterType<MaterialSwap>();
	ComponentManager::RegisterType<Morphanimator>();
	ComponentManager::RegisterType<MorphMeshRenderer>();

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
	
	//CreateScene();

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
	MeshResource::Sptr Building = ResourceManager::CreateAsset<MeshResource>("RBuilding.obj");
	MeshResource::Sptr KBuilding1Mesh = ResourceManager::CreateAsset<MeshResource>("KBuilding.obj");
	MeshResource::Sptr KBuilding2Mesh = ResourceManager::CreateAsset<MeshResource>("KBuilding2.obj");
	MeshResource::Sptr KBuilding3Mesh = ResourceManager::CreateAsset<MeshResource>("KBuilding3.obj");
	MeshResource::Sptr OvalBuilding = ResourceManager::CreateAsset<MeshResource>("OvalBuilding.obj");
	MeshResource::Sptr CharacterMesh = ResourceManager::CreateAsset<MeshResource>("discobot.obj");
	MeshResource::Sptr DiscoBallMesh = ResourceManager::CreateAsset<MeshResource>("DiscoBall2.obj");
	MeshResource::Sptr StartPlatform = ResourceManager::CreateAsset<MeshResource>("LStartPlatform.obj");
	MeshResource::Sptr Car1Mesh = ResourceManager::CreateAsset<MeshResource>("FutureCar1.obj");
	MeshResource::Sptr SemiTruckMesh = ResourceManager::CreateAsset<MeshResource>("Semitruck.obj");
	MeshResource::Sptr PickupTruckMesh = ResourceManager::CreateAsset<MeshResource>("FuturePickup.obj");
	MeshResource::Sptr SmallWallJump = ResourceManager::CreateAsset<MeshResource>("SmallWallJump.obj");
	MeshResource::Sptr SuperSmallWallJump = ResourceManager::CreateAsset<MeshResource>("SuperSmallWallJump.obj");
	MeshResource::Sptr FallingPlat = ResourceManager::CreateAsset<MeshResource>("pianoplatform.obj");
	MeshResource::Sptr HalfCirclePlat = ResourceManager::CreateAsset<MeshResource>("HalfCriclePlat.obj");
	MeshResource::Sptr StairsRight = ResourceManager::CreateAsset<MeshResource>("StairCaseR.obj");
	MeshResource::Sptr StairsLeft = ResourceManager::CreateAsset<MeshResource>("StairCaseL.obj");
	MeshResource::Sptr Speaker = ResourceManager::CreateAsset<MeshResource>("speaker.obj");
	MeshResource::Sptr SquarePlat = ResourceManager::CreateAsset<MeshResource>("SquarePlatform.obj");
	MeshResource::Sptr FloatingLight = ResourceManager::CreateAsset<MeshResource>("FloatingStreetLight.obj");
	MeshResource::Sptr DiscoBotMesh1 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run1.obj");
	MeshResource::Sptr DiscoBotMesh2 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run2.obj");
	MeshResource::Sptr DiscoBotMesh3 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run3.obj");
	MeshResource::Sptr DiscoBotMesh4 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run4.obj");
	MeshResource::Sptr DiscoBotMesh5 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run5.obj");
	MeshResource::Sptr DiscoBotMesh6 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run6.obj");
	MeshResource::Sptr DiscoBotMesh7 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run7.obj");
	MeshResource::Sptr DiscoBotMesh8 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run8.obj");
	MeshResource::Sptr DiscoBotMesh9 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run9.obj");
	MeshResource::Sptr BotJump1= ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump1.obj");
	MeshResource::Sptr BotJump2 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump2.obj");
	MeshResource::Sptr BotJump3 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump3.obj");
	MeshResource::Sptr BotJump4 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump4.obj");
	MeshResource::Sptr BotJump5 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump5.obj");
	MeshResource::Sptr BotJump6 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump6.obj");
	MeshResource::Sptr BotJump7 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump7.obj");
	//Textures
	Texture2D::Sptr StartTex = ResourceManager::CreateAsset<Texture2D>("textures/LStartPlatformTex.png");
	Texture2D::Sptr SmallTex = ResourceManager::CreateAsset<Texture2D>("textures/DanceFloorTex2.png");
	Texture2D::Sptr VinylTex = ResourceManager::CreateAsset<Texture2D>("textures/VinylTex.png");
	Texture2D::Sptr CDTex = ResourceManager::CreateAsset<Texture2D>("textures/CDTex.png");
	Texture2D::Sptr GemTex = ResourceManager::CreateAsset<Texture2D>("textures/Gem.png");
	Texture2D::Sptr GemOff = ResourceManager::CreateAsset<Texture2D>("textures/GemOff.png");
	Texture2D::Sptr CharacterTex = ResourceManager::CreateAsset<Texture2D>("textures/shirt.png");
	Texture2D::Sptr LoseScreenTex = ResourceManager::CreateAsset<Texture2D>("textures/Game_Over_Screen.png");
	Texture2D::Sptr SmallWallJumpTex = ResourceManager::CreateAsset<Texture2D>("textures/SmallWallJumpTexBlue.png");
	Texture2D::Sptr SuperSmallWallJumpTex = ResourceManager::CreateAsset<Texture2D>("textures/SmallWallJumpTexRed.png");
	Texture2D::Sptr WallJumpTex = ResourceManager::CreateAsset<Texture2D>("textures/WallJumpTex.png");
	Texture2D::Sptr Car1Tex = ResourceManager::CreateAsset<Texture2D>("textures/FutureCarTex.png");
	Texture2D::Sptr SemiTruckTex = ResourceManager::CreateAsset<Texture2D>("textures/SemiTruckTexV2.png");
	Texture2D::Sptr PickupTruckTex = ResourceManager::CreateAsset<Texture2D>("textures/PickupTruckTex.png");
	Texture2D::Sptr KBuilding1Tex = ResourceManager::CreateAsset<Texture2D>("textures/KBuildingTex.png");
	Texture2D::Sptr KBuilding2Tex = ResourceManager::CreateAsset<Texture2D>("textures/KBuilding2Tex.png");
	Texture2D::Sptr KBuilding3Tex = ResourceManager::CreateAsset<Texture2D>("textures/KBuilding3Tex.png");
	Texture2D::Sptr BuildingTex = ResourceManager::CreateAsset<Texture2D>("textures/RBuildingTex.png");
	Texture2D::Sptr OvalBuildingTex = ResourceManager::CreateAsset<Texture2D>("textures/OvalBuildingTex.png");
	Texture2D::Sptr DiscoBallTex = ResourceManager::CreateAsset<Texture2D>("textures/DiscoBallTexV2.png");
	Texture2D::Sptr FallingPlatTex = ResourceManager::CreateAsset<Texture2D>("textures/pianotex.png");
	Texture2D::Sptr HalfCirclePlatTex = ResourceManager::CreateAsset<Texture2D>("textures/halfCircleTex.png");
	Texture2D::Sptr TexBeatLogo = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BeatLogo.png");
	Texture2D::Sptr TexPlayButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BPlay.png");
	Texture2D::Sptr TexOptionsButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BOptions.png");
	Texture2D::Sptr TexMusicButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BMusic.png");
	Texture2D::Sptr TexCreditsButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BCredits.png");
	Texture2D::Sptr TexQuitButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BQuit.png");
	Texture2D::Sptr TexResumeButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BResume.png");
	Texture2D::Sptr TexResyncButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BResync.png");
	Texture2D::Sptr TexContinueButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BContinue.png");
	Texture2D::Sptr TexPauseMenu = ResourceManager::CreateAsset<Texture2D>("textures/GUI/PauseMenuBG.png");
	Texture2D::Sptr TexDimmedBG = ResourceManager::CreateAsset<Texture2D>("textures/GUI/DimBG.png");
	Texture2D::Sptr TexScoreBreakdown = ResourceManager::CreateAsset<Texture2D>("textures/GUI/ScoreBreakdown.png");
	Texture2D::Sptr TexGameOverText = ResourceManager::CreateAsset<Texture2D>("textures/GUI/GameOverText.png");
	Texture2D::Sptr TexMovementTutorial = ResourceManager::CreateAsset<Texture2D>("textures/GUI/Movement.png");
	Texture2D::Sptr TexWallJumpTutorial = ResourceManager::CreateAsset<Texture2D>("textures/GUI/WallJump.png");
	Texture2D::Sptr TexBeatGemTutorial = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BeatGems.png");
	Texture2D::Sptr TexVinylsTutorial = ResourceManager::CreateAsset<Texture2D>("textures/GUI/Vinyls.png");
	Texture2D::Sptr StairsRightTex = ResourceManager::CreateAsset<Texture2D>("textures/StairCaseRTex.png");
	Texture2D::Sptr StairsLeftTex = ResourceManager::CreateAsset<Texture2D>("textures/StarTexL.png");
	Texture2D::Sptr SpeakerTex = ResourceManager::CreateAsset<Texture2D>("textures/speakertex.png");
	Texture2D::Sptr SquarePlatTex = ResourceManager::CreateAsset<Texture2D>("textures/SquarePlatformTex.png");
	Texture2D::Sptr FloatingLightTex = ResourceManager::CreateAsset<Texture2D>("textures/StreetLightTex.png");
	Texture2D::Sptr TexBeatBar = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BeatBar.png");
	Texture2D::Sptr TexBeatBarTick = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BeatBarTick.png");
	Texture2D::Sptr TexScoreDisplay = ResourceManager::CreateAsset<Texture2D>("textures/GUI/ScoreDisplay.png");

	Font::Sptr FontVCR = ResourceManager::CreateAsset<Font>("fonts/VCR.ttf", 16.f);
	FontVCR->Bake();

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

	Material::Sptr BeatGemMaterial = ResourceManager::CreateAsset<Material>(basicShader);
	{
		BeatGemMaterial->Name = "BeatGem";
		BeatGemMaterial->Set("u_Material.Diffuse", GemTex);
		BeatGemMaterial->Set("u_Material.Shininess", 0.1f);
	}

	Material::Sptr BeatGemOffMaterial = ResourceManager::CreateAsset<Material>(basicShader);
	{
		BeatGemOffMaterial->Name = "BeatGem";
		BeatGemOffMaterial->Set("u_Material.Diffuse", GemOff);
		BeatGemOffMaterial->Set("u_Material.Shininess", 0.1f);
	}


	Material::Sptr VinylMaterial = ResourceManager::CreateAsset<Material>(toonShader);
	{
		VinylMaterial->Name = "Vinyl";
		VinylMaterial->Set("u_Material.Diffuse", VinylTex);
		VinylMaterial->Set("u_Material.Shininess", 0.1f);
		VinylMaterial->Set("u_Material.Steps", 4);
	}

	Material::Sptr CDMaterial = ResourceManager::CreateAsset<Material>(toonShader);
	{
		CDMaterial->Name = "CD";
		CDMaterial->Set("u_Material.Diffuse", CDTex);
		CDMaterial->Set("u_Material.Shininess", 0.1f);
		CDMaterial->Set("u_Material.Steps", 4);
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

	Material::Sptr KBuilding2Material = ResourceManager::CreateAsset<Material>(basicShader);
	{
		KBuilding2Material->Name = "KBuilding2";
		KBuilding2Material->Set("u_Material.Diffuse", KBuilding2Tex);
		KBuilding2Material->Set("u_Material.Shininess", 0.1f);
	}

	Material::Sptr KBuilding3Material = ResourceManager::CreateAsset<Material>(basicShader);
	{
		KBuilding3Material->Name = "KBuilding3";
		KBuilding3Material->Set("u_Material.Diffuse", KBuilding3Tex);
		KBuilding3Material->Set("u_Material.Shininess", 0.1f);
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

	Material::Sptr StairsRightMaterial = ResourceManager::CreateAsset<Material>(basicShader);
	{
		StairsRightMaterial->Name = "Stairs Right";
		StairsRightMaterial->Set("u_Material.Diffuse", StairsRightTex);
		StairsRightMaterial->Set("u_Material.Shininess", 0.1f);
	}

	Material::Sptr StairsLeftMaterial = ResourceManager::CreateAsset<Material>(basicShader);
	{
		StairsLeftMaterial->Name = "Stairs Left";
		StairsLeftMaterial->Set("u_Material.Diffuse", StairsLeftTex);
		StairsLeftMaterial->Set("u_Material.Shininess", 0.1f);
	}

	Material::Sptr SpeakerMaterial = ResourceManager::CreateAsset<Material>(basicShader);
	{
		SpeakerMaterial->Name = "Speaker Material";
		SpeakerMaterial->Set("u_Material.Diffuse", SpeakerTex);
		SpeakerMaterial->Set("u_Material.Shininess", 0.1f);
	}

	Material::Sptr SquarePlatMaterial = ResourceManager::CreateAsset<Material>(basicShader);
	{
		SquarePlatMaterial->Name = "Square Platform";
		SquarePlatMaterial->Set("u_Material.Diffuse", SquarePlatTex);
		SquarePlatMaterial->Set("u_Material.Shininess", 0.1f);
	}

	Material::Sptr FloatingLightMaterial = ResourceManager::CreateAsset<Material>(basicShader);
	{
		FloatingLightMaterial->Name = "FLoating Light";
		FloatingLightMaterial->Set("u_Material.Diffuse", FloatingLightTex);
		FloatingLightMaterial->Set("u_Material.Shininess", 0.1f);
	}

	Material::Sptr OvalBuildingMaterial = ResourceManager::CreateAsset<Material>(basicShader);
	{
		OvalBuildingMaterial->Name = "Oval Building";
		OvalBuildingMaterial->Set("u_Material.Diffuse", OvalBuildingTex);
		OvalBuildingMaterial->Set("u_Material.Shininess", 0.1f);
	}

	bool loadScene = false;
	float const distanceFromBlock = 50.f;

	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene) {
		ResourceManager::LoadManifest("manifest.json");
		scene = Scene::Load("scene.json");

		// Call scene awake to start up all of our components
		scene->Window = window;
		scene->Awake();
	}
	else {

		// Create some lights for our scene
		scene->Lights.resize(2);
		scene->Lights[0].Position = glm::vec3(0.0f, 1.0f, 3.0f);
		scene->Lights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[0].Range = 50.0f;
	/*
		// Create some lights for our scene
		scene->Lights.resize(6);
		//scene->Lights[0].Position = glm::vec3(0.0f, 1.0f, 3.0f);
		scene->Lights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[0].Range = 100.0f;

		scene->Lights[2].Position = glm::vec3(-1.380f, 17.460f, -5.710f);
		scene->Lights[2].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[2].Range = 50;

		scene->Lights[3].Position = glm::vec3(-25.380f, 14.060f, -14.020f);
		scene->Lights[3].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[3].Range = 187;

		scene->Lights[4].Position = glm::vec3(25.380f, 14.060f, -14.020f);
		scene->Lights[4].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[4].Range = 187;

		scene->Lights[5].Color = glm::vec3(1.0f, 1.0f, 1.0f);

		*/
		// Red/Green light
		scene->Lights[1].Position = glm::vec3(6.840f, 5.610f, 3.0f);
		scene->Lights[1].Color = glm::vec3((1.0f, 0.99f, 0.99f));
		scene->Lights[1].Range = 50.0f;
	


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

		//This is a game object built purely to manage game systems i.e. Scene Swaps
		//Level Spawning, Score Counting, and a few miscellaneous systems
		GameObject::Sptr GameManager = scene->CreateGameObject("GameManager");
		{
			//Pos-Rot-Scale Doesn't matter
			RigidBody::Sptr physics = GameManager->Add<RigidBody>(RigidBodyType::Kinematic);
			GameManager->Add <BeatTimer>();
			//ScoreComponent
			//LevelSpawningComponent
			//Scene Swapper

		}


		//float const distanceFromBlock = 20.f;

		// Background and forground vehicles\\
		Give these Parents for Foreground/Background Blocks if we have enough objects to do that with!
		SpawnBackGroundCar(Car1Mesh, Car1Material, "Car1", glm::vec3(14.870f, 7.80f, 2.7f), glm::vec3(90.0f, 0.0f, -90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnBackGroundCar(SemiTruckMesh, SemiTruckMaterial, "Semi1", glm::vec3(28.870f, 7.80f, 2.7f), glm::vec3(90.0f, 0.0f, -90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnForeGroundCar(Car1Mesh, Car1Material, "Car2", glm::vec3(-9.970f, 0.470f, -1.90f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnForeGroundCar(PickupTruckMesh, PickupTruckMaterial, "Pickup1", glm::vec3(-18.970f, 0.470f, -1.90f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnBackGroundBuilding(KBuilding1Mesh, KBuildingMaterial, "KBuilding1", glm::vec3(-1.0f, 21.880f, -46.040f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.780f, 1.470f, 1.0f));
		SpawnBackGroundBuilding(KBuilding2Mesh, KBuilding2Material, "KBuilding2", glm::vec3(25.670, 21.880f, -46.040f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.780f, 1.470f, 1.0f));
		SpawnBackGroundBuilding(KBuilding3Mesh, KBuilding3Material, "KBuilding3", glm::vec3(-30.530f, 21.880f, -46.040f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.780f, 1.470f, 1.0f));
		SpawnBackGroundBuilding(OvalBuilding, OvalBuildingMaterial, "OvalBuilding", glm::vec3(13.730f, 27.510f, -46.040f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.720f, 3.500f, 1.0f));
		
		// Tutorial
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-15.820f, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-11.640f, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-8.620f, 5.610f, -2.200f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-5.290f, 5.610f, -4.040f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(0.340f, 5.610f, 5.440f),  glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(1.150f, 5.610f, -4.850f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(4.360f, 5.610f, -4.850f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump", glm::vec3(-3.270f, 5.610f, 3.070f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
		SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump", glm::vec3(-1.100f, 5.610f, 2.010f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
		SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(-2.280f, 5.610f, -2.450f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnCollectable(Vinyl, VinylMaterial, "Vinyl", glm::vec3(0.180f, 5.610f, 6.010f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(9.180f, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnBackGroundCar(Car1Mesh, Car1Material, "Car1", glm::vec3(14.870f, 9.80f, 2.7f), glm::vec3(90.0f, 0.0f, -90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnBackGroundCar(SemiTruckMesh, SemiTruckMaterial, "Semi1", glm::vec3(28.870f, 9.80f, 2.7f), glm::vec3(90.0f, 0.0f, -90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnForeGroundCar(Car1Mesh, Car1Material, "Car2", glm::vec3(-9.970f, 0.470f, -1.90f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnForeGroundCar(PickupTruckMesh, PickupTruckMaterial, "Pickup1", glm::vec3(-18.970f, 0.470f, -1.90f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		float adjustment = 30;
		// 1st Block		
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-15.820f + adjustment, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-10.460f + adjustment, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-3.940f + adjustment, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(2.200f + adjustment, 5.610f, -1.770f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(6.500f + adjustment, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(-0.520f + adjustment, 5.610f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(-6.940f + adjustment, 5.610f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnCollectable(Vinyl, VinylMaterial, "Vinyl",						glm::vec3(5.210f + adjustment, 5.610f, 0.080f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(9.180f + adjustment, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		
		// CDs for Block 1
		SpawnCD(CD, CDMaterial, "CD", glm::vec3(-10.220f + 22, 5.610f, -3.220f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		SpawnCD(CD, CDMaterial, "CD", glm::vec3(-6.970f + 22, 5.610f, -2.810f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		SpawnCD(CD, CDMaterial, "CD", glm::vec3(-3.770f + 22, 5.610f, -3.190f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		SpawnCD(CD, CDMaterial, "CD", glm::vec3(-0.620f + 22, 5.610f, -2.380f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		SpawnCD(CD, CDMaterial, "CD", glm::vec3(2.140f + 22, 5.610f, -0.770f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		
		//trust me if I could find a better way i'd have done it but rn we can't pass materials to 
		//the Material Swap Component so we instead add Dummy Children Objects to hold the Materials instead.
		{
			GameObject::Sptr MaterialDummyOn = scene->CreateGameObject("Material Dummy On");
			RenderComponent::Sptr renderer = MaterialDummyOn->Add<RenderComponent>();
			renderer->SetMesh(BeatGem);
			renderer->SetMaterial(BeatGemMaterial);
			renderer->IsEnabled = false;
		}
		{
			GameObject::Sptr MaterialDummyOff = scene->CreateGameObject("Material Dummy Off");
			RenderComponent::Sptr renderer = MaterialDummyOff->Add<RenderComponent>();
			renderer->SetMesh(BeatGem);
			renderer->SetMaterial(BeatGemOffMaterial);
			renderer->IsEnabled = false;
		}

		// Player:
		GameObject::Sptr character = scene->CreateGameObject("Character/Player");
		{
			// Set position in the scene
			character->SetPostion(glm::vec3(-15.820f, 5.710f, -3.800f));
			character->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
			character->SetScale(glm::vec3(0.7f, 0.7f, 0.7f));

			// Add some behaviour that relies on the physics body
			//character->Add<JumpBehaviour>();
			character->Add<CharacterController>();

			// Create and attach a renderer for the paddle
			RenderComponent::Sptr renderer = character->Add<RenderComponent>();
			renderer->SetMesh(CharacterMesh);
			renderer->SetMaterial(CharacterMaterial);

			// Add a kinematic rigid body to the paddle
			RigidBody::Sptr physics = character->Add<RigidBody>(RigidBodyType::Dynamic);
			auto rb = physics->AddCollider(BoxCollider::Create(glm::vec3(0.2, 0.85, 0.15)));
			rb->SetPosition(glm::vec3(0.0, 0.75, 0.0));

			TriggerVolume::Sptr volume = character->Add<TriggerVolume>();
			volume->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);

			BoxCollider::Sptr collider = BoxCollider::Create(glm::vec3(0.3f, 0.3f, 0.3f));
			collider->SetPosition(glm::vec3(0.f, 0.25f, 0.f));
			volume->AddCollider(collider);

			MorphMeshRenderer::Sptr morph1 = character->Add<MorphMeshRenderer>();
			morph1->SetMorphMeshRenderer(DiscoBotMesh1, CharacterMaterial);
			Morphanimator::Sptr Animator = character->Add<Morphanimator>();


			std::vector<MeshResource::Sptr> RunAnim;
			std::vector<MeshResource::Sptr> JumpAnim;

			RunAnim.push_back(DiscoBotMesh2);
			RunAnim.push_back(DiscoBotMesh3);
			RunAnim.push_back(DiscoBotMesh4);
			RunAnim.push_back(DiscoBotMesh5);
			RunAnim.push_back(DiscoBotMesh6);
			RunAnim.push_back(DiscoBotMesh7);
			RunAnim.push_back(DiscoBotMesh8);
			RunAnim.push_back(DiscoBotMesh9);

			JumpAnim.push_back(BotJump2);
			JumpAnim.push_back(BotJump3);
			JumpAnim.push_back(BotJump4);
			JumpAnim.push_back(BotJump5);
			JumpAnim.push_back(BotJump6);
			JumpAnim.push_back(BotJump7);

			Animator->SetInitial();
			Animator->SetFrameTime(0.1f);

			Animator->SetFrames(RunAnim);
		
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

		{//Main Menu Block

				{//Logo
					GameObject::Sptr logo = scene->CreateGameObject("MainMenu Logo");

					RectTransform::Sptr transform = logo->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 750, 750 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 750, 750 });

					GuiPanel::Sptr logoPanel = logo->Add<GuiPanel>();
					logoPanel->SetTexture(TexBeatLogo);
					logoPanel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					logoPanel->SetBorderRadius(0);
					logoPanel->IsEnabled = false;



					transform->SetPosition({ (float)windowSize.x * 0.5, 300 });

				}

				{//Play Button
					GameObject::Sptr button = scene->CreateGameObject("MainMenu Play Button");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 200, 100 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 200, 100 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexPlayButton);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					panel->SetBorderRadius(0);
					panel->IsEnabled = false;


					transform->SetPosition({ (float)windowSize.x * 0.20, (float)windowSize.y * 0.8});

				}

				{//Options Button
					GameObject::Sptr button = scene->CreateGameObject("MainMenu Options Button");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 200, 100 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 200, 100 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexOptionsButton);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.6f));
					panel->SetBorderRadius(0);
					panel->IsEnabled = false;


					transform->SetPosition({ (float)windowSize.x * 0.35, (float)windowSize.y * 0.8 });

				}

				{//Music Button
					GameObject::Sptr button = scene->CreateGameObject("MainMenu Music Button");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 200, 100 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 200, 100 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexMusicButton);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.6f));
					panel->SetBorderRadius(0);
					panel->IsEnabled = false;


					transform->SetPosition({ (float)windowSize.x * 0.5, (float)windowSize.y * 0.8 });

				}

				{//Credits Button
					GameObject::Sptr button = scene->CreateGameObject("MainMenu Credits Button");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 200, 100 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 200, 100 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexCreditsButton);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.6f));
					panel->SetBorderRadius(0);
					panel->IsEnabled = false;



					transform->SetPosition({ (float)windowSize.x * 0.65, (float)windowSize.y * 0.8 });

				}

				{//Quit Button
					GameObject::Sptr button = scene->CreateGameObject("MainMenu Quit Button");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 200, 100 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 200, 100 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexQuitButton);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					panel->SetBorderRadius(0);
					panel->IsEnabled = false;



					transform->SetPosition({ (float)windowSize.x * 0.8, (float)windowSize.y * 0.8 });

				}

			}


		{//Pause Menu Block

			{//Dim BG
				GameObject::Sptr background = scene->CreateGameObject("PauseMenu Dimmed Background");

				RectTransform::Sptr transform = background->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetSize({ 1920, 1080 });
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 1920, 1080 });

				GuiPanel::Sptr panel = background->Add<GuiPanel>();
				panel->SetTexture(TexDimmedBG);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;


				transform->SetPosition({ (float)windowSize.x * 0.5, (float)windowSize.y * 0.5 });

			}

			{//Background
				GameObject::Sptr background = scene->CreateGameObject("PauseMenu Background");

				RectTransform::Sptr transform = background->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetSize({ 400, 750 });
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 400, 750 });

				GuiPanel::Sptr panel = background->Add<GuiPanel>();
				panel->SetTexture(TexPauseMenu);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;


				transform->SetPosition({ (float)windowSize.x * 0.5, (float)windowSize.y * 0.5 });

			}

			{//Resume Button
				GameObject::Sptr button = scene->CreateGameObject("PauseMenu Resume Button");

				RectTransform::Sptr transform = button->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetSize({ 300, 150 });
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 300, 150 });

				GuiPanel::Sptr panel = button->Add<GuiPanel>();
				panel->SetTexture(TexResumeButton);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;


				transform->SetPosition({ (float)windowSize.x * 0.5, (float)windowSize.y * 0.28 });

			}

			{//Options Button
				GameObject::Sptr button = scene->CreateGameObject("PauseMenu Options Button");

				RectTransform::Sptr transform = button->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetSize({ 300, 150 });
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 300, 150 });

				GuiPanel::Sptr panel = button->Add<GuiPanel>();
				panel->SetTexture(TexOptionsButton);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.6f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;

				transform->SetPosition({ (float)windowSize.x * 0.5, (float)windowSize.y * 0.43 });

			}

			{//Resync Button
				GameObject::Sptr button = scene->CreateGameObject("PauseMenu Resync Button");

				RectTransform::Sptr transform = button->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetSize({ 300, 150 });
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 300, 150 });

				GuiPanel::Sptr panel = button->Add<GuiPanel>();
				panel->SetTexture(TexResyncButton);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.6f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;

				transform->SetPosition({ (float)windowSize.x * 0.5, (float)windowSize.y * 0.58 });

			}

			{//Quit Button
				GameObject::Sptr button = scene->CreateGameObject("PauseMenu Quit Button");

				RectTransform::Sptr transform = button->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetSize({ 300, 150 });
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 300, 150 });

				GuiPanel::Sptr panel = button->Add<GuiPanel>();
				panel->SetTexture(TexQuitButton);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;


				transform->SetPosition({ (float)windowSize.x * 0.5, (float)windowSize.y * 0.73 });

			}
		}

		{//Game Over Block

				{//Dim BG
					GameObject::Sptr background = scene->CreateGameObject("GameOver Dimmed Background");

					RectTransform::Sptr transform = background->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 1920, 1080 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 1920, 1080 });

					GuiPanel::Sptr panel = background->Add<GuiPanel>();
					panel->SetTexture(TexDimmedBG);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					panel->SetBorderRadius(0);
					panel->IsEnabled = false;


					transform->SetPosition({ (float)windowSize.x * 0.5, (float)windowSize.y * 0.5 });

				}

				{//Game Over Text
					GameObject::Sptr button = scene->CreateGameObject("GameOver Text");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 809, 249 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 809, 249 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexGameOverText);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					panel->SetBorderRadius(0);
					panel->IsEnabled = false;


					transform->SetPosition({ (float)windowSize.x * 0.5, (float)windowSize.y * 0.2});

				}

				{//Score breakdown
					GameObject::Sptr button = scene->CreateGameObject("GameOver Score Breakdown");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 504 * 0.75, 475 * 0.75 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 504 *0.75, 475 * 0.75 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexScoreBreakdown);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					panel->SetBorderRadius(0);
					panel->IsEnabled = false;


					transform->SetPosition({ (float)windowSize.x * 0.4, (float)windowSize.y * 0.5 });

				}


				{//Quit
					GameObject::Sptr button = scene->CreateGameObject("GameOver Quit Button");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 300, 150 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 300, 150 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexQuitButton);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					panel->SetBorderRadius(0);
					panel->IsEnabled = false;

					transform->SetPosition({ (float)windowSize.x * 0.35, (float)windowSize.y * 0.8 });

				}

				{//Continue Button
					GameObject::Sptr button = scene->CreateGameObject("GameOver Continue Button");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 300, 150 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 300, 150 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexContinueButton);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					panel->SetBorderRadius(0);
					panel->IsEnabled = false;


					transform->SetPosition({ (float)windowSize.x * 0.65, (float)windowSize.y * 0.8 });

				}
		}
		
		{//Tutorial Blocks

				{//Movement
					GameObject::Sptr button = scene->CreateGameObject("Movement Tutorial");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 700 *0.75, 500 * 0.75 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 700 * 0.75, 500 * 0.75 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexMovementTutorial);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					panel->SetBorderRadius(0);


					transform->SetPosition({ (float)windowSize.x * 0.2, (float)windowSize.y * 0.2 });

				}

				{//Wall Jump
					GameObject::Sptr button = scene->CreateGameObject("Wall Jump Tutorial");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 700 * 0.75, 500 * 0.75 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 700 * 0.75, 500 * 0.75 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexWallJumpTutorial);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					panel->SetBorderRadius(0);


					transform->SetPosition({ (float)windowSize.x * 0.2, (float)windowSize.y * 0.6 });

				}

				{//Beat Gem
					GameObject::Sptr button = scene->CreateGameObject("Beat Gem Tutorial");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 700 * 0.75, 500 * 0.75 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 700 * 0.75, 500 * 0.75 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexBeatGemTutorial);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					panel->SetBorderRadius(0);


					transform->SetPosition({ (float)windowSize.x * 0.6, (float)windowSize.y * 0.2 });

				}

				{//Vinyls
					GameObject::Sptr button = scene->CreateGameObject("Vinyl Tutorial");

					RectTransform::Sptr transform = button->Add<RectTransform>();
					transform->SetPosition({ 0, 0 });
					transform->SetRotationDeg(0);
					transform->SetSize({ 700 * 0.75, 500 * 0.75 });
					transform->SetMin({ 0, 0 });
					transform->SetMax({ 700 * 0.75, 500 * 0.75 });

					GuiPanel::Sptr panel = button->Add<GuiPanel>();
					panel->SetTexture(TexVinylsTutorial);
					panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					panel->SetBorderRadius(0);


					transform->SetPosition({ (float)windowSize.x * 0.6, (float)windowSize.y * 0.6 });

				}
		}
		

		{//HUD
			{//Beat Bar
				GameObject::Sptr button = scene->CreateGameObject("HUD Score Display");

				RectTransform::Sptr transform = button->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetSize({ 500, 100});
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 500, 100});

				GuiPanel::Sptr panel = button->Add<GuiPanel>();
				panel->SetTexture(TexScoreDisplay);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;



				transform->SetPosition({250, 50});

			}

			{//Score Text
				GameObject::Sptr button = scene->CreateGameObject("GameOver Score Text");

				RectTransform::Sptr transform = button->Add<RectTransform>();
				transform->SetPosition({ 0,0});
				transform->SetRotationDeg(0);
				transform->SetSize({ 504, 475});
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 504, 475});

				transform->SetPosition({ 450 , 80 });


				GuiPanel::Sptr panel = button->Add<GuiPanel>();
				panel->SetTexture(TexScoreBreakdown);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;

				GuiText::Sptr text = button->Add<GuiText>();
				text->SetColor(glm::vec4(0.f));
				text->SetFont(FontVCR);
				text->SetText("0");
				text->IsEnabled = false;

				text->SetTextScale(4.0f);

			}
		/*
			{//Beat Bar
				GameObject::Sptr button = scene->CreateGameObject("HUD Beat Bar");

				RectTransform::Sptr transform = button->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetSize({ 800 * 0.75, 300 * 0.75 });
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 800 * 0.75, 300 * 0.75 });

				GuiPanel::Sptr panel = button->Add<GuiPanel>();
				panel->SetTexture(TexBeatBar);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);


				transform->SetPosition({ (float)windowSize.x * 0.5, (float)windowSize.y * 0.9 });

			}

			{//Beat Tick
				GameObject::Sptr button = scene->CreateGameObject("HUD Beat Tick");

				RectTransform::Sptr transform = button->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetSize({ 50 * 0.75, 170 * 0.75 });
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 50 * 0.75, 170 * 0.75 });

				GuiPanel::Sptr panel = button->Add<GuiPanel>();
				panel->SetTexture(TexBeatBarTick);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);


				transform->SetPosition({ (float)windowSize.x * 0.4, (float)windowSize.y * 0.9 });

			}
		*/
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

	// We'll use this to allow editing the save/load path
	// via ImGui, note the reserve to allocate extra space
	// for input!
	std::string scenePath = "scene.json";
	scenePath.reserve(256);
	// doesn't seem to do anything besides on/off
	scene->SetAmbientLight(glm::vec3(1.0f));

	// Our high-precision timer
	double lastFrame = glfwGetTime();

	BulletDebugMode physicsDebugMode = BulletDebugMode::None;
	float playbackSpeed = 1.0f;

	nlohmann::json editorSceneState;
	int _Pause;
	
	srand(time(0));

	//Give these an application layer for the final product
	SoundEffects Sound;
	Sound.init();

	ToneFire::FMODStudio Studio = Sound.GetContext();
	ToneFire::StudioSound Test = Sound.GetContextSound();

	Test.LoadEvent("event:/Music");
	Test.SetEventPosition("event:/Music", FMOD_VECTOR{ -10.270f, 5.710f, -3.800f });
	Test.SetVolume("event:/Music", 0.5f);

	
	//test.SetEventParameter("event:/Music", "Volume", 0.5f);
	float SpawnTimer = 20;
	///// Game loop /////
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		ImGuiHelper::StartFrame();
		Studio.Update();

		//Idk wtf is going on here it shouldn't be checking if not true but it DOES work so I'm leaving it alone
		if (!scene->IsPlaying) {

			std::cout << "Is this john working???" << std::endl;
			Test.PlayEvent("event:/Music");

		}


		glm::vec3 playerPos = scene->FindObjectByName("DiscoBall")->GetPosition();
		float playerPosX = playerPos.x;
		float playerPosY = playerPos.y;
		glm::vec3 lightFollowPos = glm::vec3(playerPosX, playerPosY, (playerPos.z + 3));

		///////////// For moving Lights will be expanded upon later ///////////////////////////////////
		// Light 0 follows the discoball
		// light 1 changes color when player is at the right most part of the screen
		if (playerPos.x < 2.0f)
		{
			scene->Lights[1].Color = glm::vec3(0.98f, 0.10f, 0.10f);
		}

		if (playerPos.x >= 2.0f)
		{
			scene->Lights[1].Color = glm::vec3(0.011f, 0.70f, 0.20f);
		}
		//scene->Lights[1].Position = glm::vec3(6.840f, 5.610f, 3.0f);
		scene->Lights[0].Position = glm::vec3(lightFollowPos);
		//scene->Lights[1].Range = 50;
		//scene->Lights[0].Range = 50;
		
		scene->SetupShaderAndLights();
		// Calculate the time since our last frame (dt)
		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);

		if (scene->IsPlaying) {

			SpawnTimer += dt;
		}
		// Draw our material properties window!
		DrawMaterialsWindow();

		//Player Quick Pause Functionality
		glfwSetKeyCallback(window, key_callback);

		// Showcasing how to use the imGui library!
		bool isDebugWindowOpen = ImGui::Begin("Debugging");
		//isDebugWindowOpen = false;
		//scene->IsPlaying = true;
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
		
		LOG_INFO(SpawnTimer);

		if (SpawnTimer >= 30) {
			SpawnTimer -= 30;
			
			
			// can use to spawn in the tutorial level when the character dies
			// besides this u could reset player position in character controll and reset score
			// can't use right now because this call to function needs to realative
			// to an object
			/*if (CharacterController::GetCharacterBool() == 1)
			{
				int blockToSpawn = 0;
			}
			else
			{
				// put rest of block spawn code here
			}*/
			int blockToSpawn = rand() % 8;

			switch (blockToSpawn) {
			case 0:
				// 1st Block
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-9.820f + distanceFromBlock, 5.610f, -4.450), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-6.070f + distanceFromBlock, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-2.840f + distanceFromBlock, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(2.760f + distanceFromBlock, 5.610f, -1.770f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(0.120f + distanceFromBlock, 5.610f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnCollectable(Vinyl, VinylMaterial, "Vinyl", glm::vec3(5.640f + distanceFromBlock, 5.610f, 0.080f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(6.360f + distanceFromBlock, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));

				// CDs for Block 1
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-6.030f + distanceFromBlock, 5.610f, -3.220f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-2.710f + distanceFromBlock, 5.610f, -3.190f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(0.170f + distanceFromBlock, 5.610f, -2.380f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(2.640f + distanceFromBlock, 5.610f, -0.770f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));

				break;
			case 1:
				//spawn block 2
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-15.820f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump", glm::vec3(-9.320f + distanceFromBlock, 5.610f, 3.210f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.040f, 1.500f));
				SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump", glm::vec3(-7.320f + distanceFromBlock, 5.610f, 2.000f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.040f, 1.500f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-12.130f + distanceFromBlock, 5.610f, -3.850f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-4.880f + distanceFromBlock, 5.610f, 4.000f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(1.940f + distanceFromBlock, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(6.190f + distanceFromBlock, 5.610f, -4.160f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(-9.190f + distanceFromBlock, 5.610f, -2.060f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(-1.340f + distanceFromBlock, 5.610f, 0.500f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(9.180f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));

				// CDs for Block 2
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-8.380f + distanceFromBlock, 5.610f, -0.030f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-8.380f + distanceFromBlock, 5.610f, 2.130f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-8.380f + distanceFromBlock, 5.610f, 4.610f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-4.910f + distanceFromBlock, 5.610f, 4.750f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-1.340f + distanceFromBlock, 5.610f, 0.810f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(1.920f + distanceFromBlock, 5.610f, -3.610f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));

				break;
			case 2:
				//spawn block 3
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-15.820f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-9.000f + distanceFromBlock, 5.610f, -0.290f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-4.360f + distanceFromBlock, 5.610f, -0.290f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(0.350f + distanceFromBlock, 5.610f, -0.290f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(FallingPlat, PianoMaterial, "Falling Platform", glm::vec3(0.390f + distanceFromBlock, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(3.220f + distanceFromBlock, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(6.490f + distanceFromBlock, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(-11.530f + distanceFromBlock, 5.610f, -1.970f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(-6.870f + distanceFromBlock, 5.610f, -1.970f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(-1.870f + distanceFromBlock, 5.610f, -1.970f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnCollectable(Vinyl, VinylMaterial, "Vinyl", glm::vec3(0.370f + distanceFromBlock, 5.610f, -2.830f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(9.180f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));

				// CDs for Block 3
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-11.530f + distanceFromBlock, 5.610f, -1.360f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-9.000f + distanceFromBlock, 5.610f, 0.440f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-4.390f + distanceFromBlock, 5.610f, 0.440f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(0.350f + distanceFromBlock, 5.610f, 0.290f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(3.260f + distanceFromBlock, 5.610f, -3.210f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-6.690f + distanceFromBlock, 5.610f, -1.180f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				break;
			case 3:
				//spawn block 4
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-15.820f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-12.030f + distanceFromBlock, 5.610f, -4.220f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-8.540f + distanceFromBlock, 5.610f, -4.220f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-4.230f + distanceFromBlock, 5.610f, -4.220f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(2.290f + distanceFromBlock, 5.610f, 4.700f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnSpeaker(Speaker, SpeakerMaterial, "Speaker", glm::vec3(3.400f + distanceFromBlock, 6.410f, -0.090f), glm::vec3(90.0f, 0.0f, -32.000f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnObj(HalfCirclePlat, HalfCirclePlatMaterial, "Half Circle Platform", glm::vec3(-0.720f + distanceFromBlock, 5.610f, -4.220f), glm::vec3(-90.000f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnBuilding(Building, BuildingMaterial, "Building", glm::vec3(4.580f + distanceFromBlock, 5.610f, -7.110f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump", glm::vec3(-1.590f + distanceFromBlock, 5.610f, 2.650f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnWallJump(WallJump, WallJumpMaterial, "Wall Jump", glm::vec3(0.460f + distanceFromBlock, 5.610f, 1.610), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(1.770f + distanceFromBlock, 5.610f, -3.520f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				//SpawnCollectable(Vinyl, VinylMaterial, "Vinyl", glm::vec3(2.190f + distanceFromBlock, 5.610f, 5.390f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(9.180f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));

				// CDs for Block 4
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-11.730f + distanceFromBlock, 5.610f, -3.310f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-8.540f + distanceFromBlock, 5.610f, -3.310f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-4.230f + distanceFromBlock, 5.610f, -3.160f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-0.630f + distanceFromBlock, 5.610f, -0.520f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-0.640f + distanceFromBlock, 5.610f, 2.130f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-0.640f + distanceFromBlock, 5.610f, 4.770f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(2.190f + distanceFromBlock, 5.610f, 5.390f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(1.770f + distanceFromBlock, 5.610f, -1.180f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				break;
			case 4:
				//spawn block 5
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-15.820f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-12.820f + distanceFromBlock, 5.610f, -4.220f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-10.240f + distanceFromBlock, 5.610f, -2.830f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-7.960f + distanceFromBlock, 5.610f, -1.410f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-3.330f + distanceFromBlock, 5.610f, 5.950f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(2.280f + distanceFromBlock, 5.610f, 4.180f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(2.280f + distanceFromBlock, 5.610f, -4.010f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnSquarePlat(SquarePlat, SquarePlatMaterial, "Square Platform", glm::vec3(-6.180f + distanceFromBlock, 5.610f, -0.170f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnSquarePlat(SquarePlat, SquarePlatMaterial, "Square Platform", glm::vec3(4.630f + distanceFromBlock, 5.610f, 2.260f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnSquarePlat(SquarePlat, SquarePlatMaterial, "Square Platform", glm::vec3(5.010f + distanceFromBlock, 5.610f, -3.850f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump", glm::vec3(-6.730f + distanceFromBlock, 5.610f, 4.460f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump", glm::vec3(-5.030f + distanceFromBlock, 5.610f, 4.110f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump", glm::vec3(-1.590f + distanceFromBlock, 5.610f, 2.650f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump", glm::vec3(0.460f + distanceFromBlock, 5.610f, 1.610f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(-0.580f + distanceFromBlock, 5.610f, -1.970f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnCollectable(Vinyl, VinylMaterial, "Vinyl", glm::vec3(2.190f + distanceFromBlock, 5.610f, 5.390f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(9.180f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));

				// CDs for Block 5
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-6.040f + distanceFromBlock, 5.610f, 2.340f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-6.040f + distanceFromBlock, 5.610f, 5.170f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-0.630f + distanceFromBlock, 5.610f, 0.460f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-0.640f + distanceFromBlock, 5.610f, 2.710f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(2.170f + distanceFromBlock, 5.610f, -2.880f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				break;
			case 5:
				//block 6
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-15.820f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-12.110f + distanceFromBlock, 5.610f, -4.220f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-8.870f + distanceFromBlock, 5.610f, -2.830f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-5.920f + distanceFromBlock, 5.610f, -1.410f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-3.530f + distanceFromBlock, 5.610f, 0.030f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(5.770f + distanceFromBlock, 5.610f, -5.060f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnSpeaker(Speaker, SpeakerMaterial, "Speaker", glm::vec3(0.840f + distanceFromBlock, 6.410f, 2.360f), glm::vec3(90.0f, 0.0f, -32.000f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump", glm::vec3(-2.600f + distanceFromBlock, 5.610f, 5.940f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump", glm::vec3(-1.170f + distanceFromBlock, 5.610f, 6.950f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnBuilding(Building, BuildingMaterial, "Building Block6 1", glm::vec3(-1.010f + distanceFromBlock, 5.610f, -4.960f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.310f, 0.310f, 0.310f));
				SpawnBuilding(Building, BuildingMaterial, "Building Block6 2", glm::vec3(2.070f + distanceFromBlock, 5.610f, -3.810f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.310f, 0.310f, 0.310f));
				SpawnCollectable(Vinyl, VinylMaterial, "Vinyl", glm::vec3(-1.890f + distanceFromBlock, 5.610f, 5.390f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(9.180f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));

				// CDs for Block 6
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-11.690f + distanceFromBlock, 5.610f, -3.190f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-8.860f + distanceFromBlock, 5.610f, -1.820f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-5.720f + distanceFromBlock, 5.610f, -0.610f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-3.470f + distanceFromBlock, 5.610f, 0.790f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(5.660f + distanceFromBlock, 5.610f, -4.080f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				break;
			case 6:
				//block 7
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-15.820f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnStairsRight(StairsRight, StairsRightMaterial, "Stairs Right", glm::vec3(-11.200f + distanceFromBlock, 5.610f, -4.200f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-5.130f + distanceFromBlock, 5.610f, 2.210f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-0.810f + distanceFromBlock, 5.610f, 2.270f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-0.100f + distanceFromBlock, 5.610f, -1.110f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnSpeaker(Speaker, SpeakerMaterial, "Speaker", glm::vec3(3.630f + distanceFromBlock, 6.410f, 2.560f), glm::vec3(90.0f, 0.0f, -32.000f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnStairsLeft(StairsLeft, StairsLeftMaterial, "Stairs Left", glm::vec3(-1.250f + distanceFromBlock, 5.610f, -0.920f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump", glm::vec3(-8.890f + distanceFromBlock, 5.610f, 2.050f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnSmallWallJump(SmallWallJump, SmallWallJumpMaterial, "Small Wall Jump", glm::vec3(-6.950f + distanceFromBlock, 5.610f, 0.380f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnBuilding(Building, BuildingMaterial, "Building", glm::vec3(4.730f + distanceFromBlock, 5.610f, -3.610f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.310f, 0.310f, 0.310f));
				SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(1.350f + distanceFromBlock, 5.610f, 1.180f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnCollectable(Vinyl, VinylMaterial, "Vinyl", glm::vec3(-0.740f + distanceFromBlock, 5.610f, 2.750f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(9.180f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));

				// CDs for Block 7
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-7.840f + distanceFromBlock, 5.610f, -0.950f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-7.850f + distanceFromBlock, 5.610f, 2.340f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-5.030f + distanceFromBlock, 5.610f, 2.760f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-0.120f + distanceFromBlock, 5.610f, -0.410f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				break;
			case 7:
				// 8th Block
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-15.820f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-12.960f + distanceFromBlock, 5.610f, -4.910f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-9.270f + distanceFromBlock, 5.610f, -4.140f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-4.890f + distanceFromBlock, 5.610f, -3.310f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnStairsRight(StairsRight, StairsRightMaterial, "Stairs Right", glm::vec3(-0.320f + distanceFromBlock, 5.610f, -5.570f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnSpeaker(Speaker, SpeakerMaterial, "Speaker", glm::vec3(3.410f + distanceFromBlock, 6.410f, -0.350f), glm::vec3(90.0f, 0.0f, -32.000f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnBuilding2(Building, BuildingMaterial, "Building", glm::vec3(-7.960f + distanceFromBlock, 5.610f, 6.710f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnBuilding(Building, BuildingMaterial, "Building", glm::vec3(4.570f + distanceFromBlock, 5.610f, -6.630f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.310f, 0.310f, 0.310f));
				SpawnBuilding2(Building, BuildingMaterial, "Building", glm::vec3(-4.500f + distanceFromBlock, 5.610f, 7.810f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnBuilding3(Building, BuildingMaterial, "Building", glm::vec3(-1.350f + distanceFromBlock, 5.610f, 5.180f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.290f, 0.290f, 0.290f));
				SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(-2.630f + distanceFromBlock, 5.610f, -4.550f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnSuperSmallWallJump(SuperSmallWallJump, SuperSmallWallJumpMaterial, "Super Small Wall Jump", glm::vec3(2.400f + distanceFromBlock, 5.610f, -3.180f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnSuperSmallWallJump(SuperSmallWallJump, SuperSmallWallJumpMaterial, "Super Small Wall Jump", glm::vec3(0.700f + distanceFromBlock, 5.610f, -1.510f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnSuperSmallWallJump(SuperSmallWallJump, SuperSmallWallJumpMaterial, "Super Small Wall Jump", glm::vec3(2.400f + distanceFromBlock, 5.610f, -0.590f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnSuperSmallWallJump(SuperSmallWallJump, SuperSmallWallJumpMaterial, "Super Small Wall Jump", glm::vec3(0.700f + distanceFromBlock, 5.610f, 0.700f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.210f, 1.500f));
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(9.180f + distanceFromBlock, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));

				// CDs for Block 8
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-12.620f + distanceFromBlock, 5.610f, -4.080f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-9.160f + distanceFromBlock, 5.610f, -3.410f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-4.850f + distanceFromBlock, 5.610f, -2.620f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(1.540f + distanceFromBlock, 5.610f, -0.020f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(1.500f + distanceFromBlock, 5.610f, -4.260f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(4.170f + distanceFromBlock, 5.610f, 0.400f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				break;

			default:
				// 1st Block
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-9.820f + distanceFromBlock, 5.610f, -4.450), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-6.070f + distanceFromBlock, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-2.840f + distanceFromBlock, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnObj(SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(2.760f + distanceFromBlock, 5.610f, -1.770f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
				SpawnGem(BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem 4", glm::vec3(0.120f + distanceFromBlock, 5.610f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
				SpawnCollectable(Vinyl, VinylMaterial, "Vinyl", glm::vec3(5.640f + distanceFromBlock, 5.610f, 0.080f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnStartPlat(StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(6.360f + distanceFromBlock, 5.610f, -4.920f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));

				// CDs for Block 1
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-6.030f + distanceFromBlock, 5.610f, -3.220f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(-2.710f + distanceFromBlock, 5.610f, -3.190f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(0.170f + distanceFromBlock, 5.610f, -2.380f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				SpawnCD(CD, CDMaterial, "CD", glm::vec3(2.640f + distanceFromBlock, 5.610f, -0.770f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
				break;
			}
			//we only want to wait 20seconds for the first level after that we simply want to wait 10
			//SpawnTimer = 0.f;
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
		  glDisable(GL_DEPTH_TEST);
		// Disable depth writing
		  glDepthMask(GL_FALSE);

		// Enable alpha blending
		  glEnable(GL_BLEND);
		  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Enable the scissor test;
		//  glEnable(GL_SCISSOR_TEST);

		// Our projection matrix will be our entire window for now
		glm::mat4 proj = glm::ortho(0.0f, (float)windowSize.x, (float)windowSize.y, 0.0f, -1.0f, 1.0f);
		GuiBatcher::SetProjection(proj);

		// Iterate over and render all the GUI objects
		scene->RenderGUI();

		// Flush the Gui Batch renderer
		GuiBatcher::Flush();

		glEnable(GL_DEPTH_TEST);

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
		// InputEngine::EndFrame();
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
