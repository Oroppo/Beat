

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
glm::ivec2 windowSize = glm::ivec2(800, 800);
// The title of our GLFW window
std::string windowTitle = "INFR-1350U";

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

		MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("Monkey.obj");
		MeshResource::Sptr tableMesh = ResourceManager::CreateAsset<MeshResource>("table.obj");
		MeshResource::Sptr puckMesh = ResourceManager::CreateAsset<MeshResource>("puck.obj");
		MeshResource::Sptr paddleMesh = ResourceManager::CreateAsset<MeshResource>("paddle.obj");
		MeshResource::Sptr ScoreBoard = ResourceManager::CreateAsset<MeshResource>("ScoreBoard.obj");

		Texture2D::Sptr    boxTexture = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		Texture2D::Sptr    monkeyTex  = ResourceManager::CreateAsset<Texture2D>("textures/monkey-uvMap.png");
		Texture2D::Sptr    tableTex = ResourceManager::CreateAsset<Texture2D>("textures/HockeyRink.png");
		Texture2D::Sptr    puckTex = ResourceManager::CreateAsset<Texture2D>("textures/puck.png");
		Texture2D::Sptr    paddleRedTex = ResourceManager::CreateAsset<Texture2D>("textures/HockeyStickRed.png");
		Texture2D::Sptr    paddleBlueTex = ResourceManager::CreateAsset<Texture2D>("textures/HockeyStickBlue.png");

		Texture2D::Sptr    R0_Tex = ResourceManager::CreateAsset<Texture2D>("textures/R0.png");
		Texture2D::Sptr    R1_Tex = ResourceManager::CreateAsset<Texture2D>("textures/R1.png");
		Texture2D::Sptr    R2_Tex = ResourceManager::CreateAsset<Texture2D>("textures/R2.png");
		Texture2D::Sptr    R3_Tex = ResourceManager::CreateAsset<Texture2D>("textures/R3.png");
		Texture2D::Sptr    R4_Tex = ResourceManager::CreateAsset<Texture2D>("textures/R4.png");
		Texture2D::Sptr    R5_Tex = ResourceManager::CreateAsset<Texture2D>("textures/R5.png");
		Texture2D::Sptr    R6_Tex = ResourceManager::CreateAsset<Texture2D>("textures/R6.png");
		Texture2D::Sptr    R7_Tex = ResourceManager::CreateAsset<Texture2D>("textures/R7.png");
		Texture2D::Sptr    R8_Tex = ResourceManager::CreateAsset<Texture2D>("textures/R8.png");
		Texture2D::Sptr    R9_Tex = ResourceManager::CreateAsset<Texture2D>("textures/R9.png");
		// Create an empty scene
		scene = std::make_shared<Scene>();

		// I hate this
		scene->BaseShader = uboShader;

		// Create our materials
		Material::Sptr boxMaterial = ResourceManager::CreateAsset<Material>();
		{
			boxMaterial->Name = "Box";
			boxMaterial->MatShader = scene->BaseShader;
			boxMaterial->Texture = boxTexture;
			boxMaterial->Shininess = 2.0f;
		}	

		Material::Sptr monkeyMaterial = ResourceManager::CreateAsset<Material>();
		{
			monkeyMaterial->Name = "Monkey";
			monkeyMaterial->MatShader = scene->BaseShader;
			monkeyMaterial->Texture = monkeyTex;
			monkeyMaterial->Shininess = 256.0f;
		}
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
		Material::Sptr R0_mat = ResourceManager::CreateAsset<Material>();
		{
			R0_mat->Name = "R0";
			R0_mat->MatShader = scene->BaseShader;
			R0_mat->Texture = R0_Tex;
			R0_mat->Shininess = 0.0f;
		}
		Material::Sptr R1_mat = ResourceManager::CreateAsset<Material>();
		{
			R1_mat->Name = "R1";
			R1_mat->MatShader = scene->BaseShader;
			R1_mat->Texture = R1_Tex;
			R1_mat->Shininess = 0.0f;
		}
		Material::Sptr R2_mat = ResourceManager::CreateAsset<Material>();
		{
			R2_mat->Name = "R2";
			R2_mat->MatShader = scene->BaseShader;
			R2_mat->Texture = R2_Tex;
			R2_mat->Shininess = 0.0f;
		}
		Material::Sptr R3_mat = ResourceManager::CreateAsset<Material>();
		{
			R3_mat->Name = "R3";
			R3_mat->MatShader = scene->BaseShader;
			R3_mat->Texture = R3_Tex;
			R3_mat->Shininess = 1.0f;
		}
		Material::Sptr R4_mat = ResourceManager::CreateAsset<Material>();
		{
			R4_mat->Name = "R4";
			R4_mat->MatShader = scene->BaseShader;
			R4_mat->Texture = R4_Tex;
			R4_mat->Shininess = 1.0f;
		}
		Material::Sptr R5_mat = ResourceManager::CreateAsset<Material>();
		{
			R5_mat->Name = "R5";
			R5_mat->MatShader = scene->BaseShader;
			R5_mat->Texture = R5_Tex;
			R5_mat->Shininess = 1.0f;
		}
		Material::Sptr R6_mat = ResourceManager::CreateAsset<Material>();
		{
			R6_mat->Name = "R6";
			R6_mat->MatShader = scene->BaseShader;
			R6_mat->Texture = R6_Tex;
			R6_mat->Shininess = 1.0f;
		}
		Material::Sptr R7_mat = ResourceManager::CreateAsset<Material>();
		{
			R7_mat->Name = "R7";
			R7_mat->MatShader = scene->BaseShader;
			R7_mat->Texture = R0_Tex;
			R7_mat->Shininess = 1.0f;
		}
		Material::Sptr R8_mat = ResourceManager::CreateAsset<Material>();
		{
			R8_mat->Name = "R8";
			R8_mat->MatShader = scene->BaseShader;
			R8_mat->Texture = R8_Tex;
			R8_mat->Shininess = 1.0f;
		}
		Material::Sptr R9_mat = ResourceManager::CreateAsset<Material>();
		{
			R9_mat->Name = "R9";
			R9_mat->MatShader = scene->BaseShader;
			R9_mat->Texture = R9_Tex;
			R9_mat->Shininess = 1.0f;
		}

		// Create some lights for our scene
		scene->Lights.resize(3);
		scene->Lights[0].Position = glm::vec3(0.0f, 1.0f, 3.0f);
		scene->Lights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[0].Range = 0.1f;

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
			camera->SetPostion(glm::vec3(1.5, -0.4, 2.45));			
			camera->LookAt(glm::vec3(0.0f));
			camera->SetRotation(glm::vec3(-30, 0, 180));

			Camera::Sptr cam = camera->Add<Camera>();
			cam->SetFovRadians(105.f);
			cam->SetNearPlane(0.3);

			// Make sure that the camera is set as the scene's main camera!
			scene->MainCamera = cam;
		}

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

		//Create a trigger volume for testing how we can detect collisions with objects!
		GameObject::Sptr trigger = scene->CreateGameObject("Trigger");
		{
			TriggerVolume::Sptr volume = trigger->Add<TriggerVolume>();
			BoxCollider::Sptr collider = BoxCollider::Create(glm::vec3(0.5f, 0.5f, 0.5f));
			collider->SetPosition(glm::vec3(0.0f, 0.0f, 0.5f));
			volume->AddCollider(collider);
		}

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

		// Showcasing how to use the imGui library!
		//bool isDebugWindowOpen = ImGui::Begin("Debugging");
		bool isDebugWindowOpen = false;
		scene->IsPlaying = true;
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