#include <Logging.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <json.hpp>
#include <fstream>
#include <sstream>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

// Graphics
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture2D.h"
#include "Graphics/VertexTypes.h"

#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"

#include "Camera.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Coordinator.h"
#include "ECS.h"

#define LOG_GL_NOTIFICATIONS

//TEST TO SEE IF PUSHING THE REPO WORKS

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
std::string windowTitle = "Amnesia Interactive : Beat!";


//Call this Whenever Window is Resized
void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	windowSize = glm::ivec2(width, height);
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

//////////////////////////////////////////////////////
////////////////// NEW IN WEEK 7 /////////////////////
//////////////////////////////////////////////////////

glm::mat4 MAT4_IDENTITY = glm::mat4(1.0f);
glm::mat3 MAT3_IDENTITY = glm::mat3(1.0f);

glm::vec4 UNIT_X = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
glm::vec4 UNIT_Y = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
glm::vec4 UNIT_Z = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
glm::vec4 UNIT_W = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
glm::vec4 ZERO = glm::vec4(0.0f);
glm::vec4 ONE = glm::vec4(1.0f);

// Helper structure for material parameters
// to our shader
struct MaterialInfo : IResource {
	typedef std::shared_ptr<MaterialInfo> Sptr;
	// A human readable name for the material
	std::string     Name;
	// The shader that the material is using
	Shader::Sptr    Shader;

	// Material shader parameters
	Texture2D::Sptr Texture;
	float           Shininess;

	/// <summary>
	/// Handles applying this material's state to the OpenGL pipeline
	/// Will bind the shader, update material uniforms, and bind textures
	/// </summary>
	virtual void Apply() {
		// Material properties
		Shader->SetUniform("u_Material.Shininess", Shininess);

		// For textures, we pass the *slot* that the texture sure draw from
		Shader->SetUniform("u_Material.Diffuse", 0);

		// Bind the texture
		if (Texture != nullptr) {
			Texture->Bind(0);
		}
	}

	/// <summary>
	/// Loads a material from a JSON blob
	/// </summary>
	static MaterialInfo::Sptr FromJson(const nlohmann::json& data) {
		MaterialInfo::Sptr result = std::make_shared<MaterialInfo>();
		result->OverrideGUID(Guid(data["guid"]));
		result->Name = data["name"].get<std::string>();
		result->Shader = ResourceManager::GetShader(Guid(data["shader"]));

		// material specific parameters
		result->Texture = ResourceManager::GetTexture(Guid(data["texture"]));
		result->Shininess = data["shininess"].get<float>();
		return result;
	}

	/// <summary>
	/// Converts this material into it's JSON representation for storage
	/// </summary>
	nlohmann::json ToJson() const {
		return {
			{ "guid", GetGUID().str() },
			{ "name", Name },
			{ "shader", Shader ? Shader->GetGUID().str() : "" },
			{ "texture", Texture ? Texture->GetGUID().str() : "" },
			{ "shininess", Shininess },
		};
	}
};

// Helper structure to represent an object 
// with a transform, mesh, and material
struct RenderObject {
	// Human readable name for the object
	std::string             Name;
	// Unique ID for the object
	Guid                    GUID;
	// The object's world transform
	glm::mat4               Transform;
	// The object's mesh
	VertexArrayObject::Sptr Mesh;
	// The object's material
	MaterialInfo::Sptr      Material;

	// If we want to use MeshFactory, we can populate this list
	std::vector<MeshBuilderParam> MeshBuilderParams;

	// Position of the object
	glm::vec3 Position;
	// Rotation of the object in Euler angler
	glm::vec3 Rotation;
	// The scale of the object
	glm::vec3 Scale;

	RenderObject() :
		Name("Unknown"),
		GUID(Guid::New()),
		Transform(MAT4_IDENTITY),
		Mesh(nullptr),
		Material(nullptr),
		MeshBuilderParams(std::vector<MeshBuilderParam>()),
		Position(ZERO),
		Rotation(ZERO),
		Scale(ONE) {}

	// Recalculates the Transform from the parameters (pos, rot, scale)
	void RecalcTransform() {
		Rotation = glm::fmod(Rotation, glm::vec3(360.0f)); // Wrap all our angles into the 0-360 degree range
		Transform = glm::translate(MAT4_IDENTITY, Position) * glm::mat4_cast(glm::quat(glm::radians(Rotation))) * glm::scale(MAT4_IDENTITY, Scale);
	}

	// Regenerates this object's mesh if it is using the MeshFactory
	void GenerateMesh() {
		if (MeshBuilderParams.size() > 0) {
			if (Mesh != nullptr) {
				LOG_WARN("Overriding existing mesh!");
			}
			MeshBuilder<VertexPosNormTexCol> mesh;
			for (int ix = 0; ix < MeshBuilderParams.size(); ix++) {
				MeshFactory::AddParameterized(mesh, MeshBuilderParams[ix]);
			}
			Mesh = mesh.Bake();
		}
	}

	/// <summary>
	/// Loads a render object from a JSON blob
	/// </summary>
	static RenderObject FromJson(const nlohmann::json& data) {
		RenderObject result = RenderObject();
		result.Name = data["name"];
		result.GUID = Guid(data["guid"]);
		result.Mesh = ResourceManager::GetMesh(Guid(data["mesh"]));
		// TODO material is not in resource manager
		//objects[ix]["material"] = obj.Material->GetGUID().str();
		result.Position = ParseJsonVec3(data["position"]);
		result.Rotation = ParseJsonVec3(data["rotation"]);
		result.Scale = ParseJsonVec3(data["scale"]);
		// If we have mesh parameters, we'll use that instead of the existing mesh
		if (data.contains("mesh_params") && data["mesh_params"].is_array()) {
			std::vector<nlohmann::json> meshbuilderParams = data["mesh_params"].get<std::vector<nlohmann::json>>();
			MeshBuilder<VertexPosNormTexCol> mesh;
			for (int ix = 0; ix < meshbuilderParams.size(); ix++) {
				MeshBuilderParam p = MeshBuilderParam::FromJson(meshbuilderParams[ix]);
				result.MeshBuilderParams.push_back(p);
				MeshFactory::AddParameterized(mesh, p);
			}
			result.Mesh = mesh.Bake();
		}
		return result;
	}

	/// <summary>
	/// Converts this object into it's JSON representation for storage
	/// </summary>
	nlohmann::json ToJson() const {
		nlohmann::json result = {
			{ "name", Name },
			{ "guid", GUID.str() },
			{ "mesh", Mesh->GetGUID().str() },
			{ "material", Material->GetGUID().str() },
			{ "position", GlmToJson(Position) },
			{ "rotation", GlmToJson(Rotation) },
			{ "scale", GlmToJson(Scale) },
		};
		if (MeshBuilderParams.size() > 0) {
			std::vector<nlohmann::json> params = std::vector<nlohmann::json>();
			params.resize(MeshBuilderParams.size());
			for (int ix = 0; ix < MeshBuilderParams.size(); ix++) {
				params[ix] = MeshBuilderParams[ix].ToJson();
			}
			result["mesh_params"] = params;
		}
		return result;
	}
};

// Helper structure for our light data
struct Light {
	glm::vec3 Position;
	glm::vec3 Color;
	// Basically inverse of how far our light goes (1/dist, approx)
	float Attenuation = 1.0f / 5.0f;
	// The approximate range of our light
	float Range = 4.0f;

	/// <summary>
	/// Loads a light from a JSON blob
	/// </summary>
	static Light FromJson(const nlohmann::json& data) {
		Light result;
		result.Position = ParseJsonVec3(data["position"]);
		result.Color = ParseJsonVec3(data["color"]);
		result.Range = data["range"].get<float>();
		result.Attenuation = 1.0f / (1.0f + result.Range);
		return result;
	}

	/// <summary>
	/// Converts this object into it's JSON representation for storage
	/// </summary>
	nlohmann::json ToJson() const {
		return {
			{ "position", GlmToJson(Position) },
			{ "color", GlmToJson(Color) },
			{ "range", Range },
		};
	}

};

// Temporary structure for storing all our scene stuffs
struct Scene {
	typedef std::shared_ptr<Scene> Sptr;

	std::unordered_map<Guid, MaterialInfo::Sptr> Materials; // Really should be in resources but meh

	// Stores all the objects in our scene
	std::vector<RenderObject>  Objects;
	// Stores all the lights in our scene
	std::vector<Light>         Lights;
	// The camera for our scene
	Camera::Sptr               Camera;

	Shader::Sptr               BaseShader; // Should think of more elegant ways of handling this

	Scene() :
		Materials(std::unordered_map<Guid, MaterialInfo::Sptr>()),
		Objects(std::vector<RenderObject>()),
		Lights(std::vector<Light>()),
		Camera(nullptr),
		BaseShader(nullptr) {}

	/// <summary>
	/// Searches all render objects in the scene and returns the first
	/// one who's name matches the one given, or nullptr if no object
	/// is found
	/// </summary>
	/// <param name="name">The name of the object to find</param>
	RenderObject* FindObjectByName(const std::string name) {
		auto it = std::find_if(Objects.begin(), Objects.end(), [&](const RenderObject& obj) {
			return obj.Name == name;
			});
		return it == Objects.end() ? nullptr : &(*it);
	}

	/// <summary>
	/// Loads a scene from a JSON blob
	/// </summary>
	static Scene::Sptr FromJson(const nlohmann::json& data) {
		Scene::Sptr result = std::make_shared<Scene>();
		result->BaseShader = ResourceManager::GetShader(Guid(data["default_shader"]));

		LOG_ASSERT(data["materials"].is_array(), "Materials not present in scene!");
		for (auto& material : data["materials"]) {
			MaterialInfo::Sptr mat = MaterialInfo::FromJson(material);
			result->Materials[mat->GetGUID()] = mat;
		}

		LOG_ASSERT(data["objects"].is_array(), "Objects not present in scene!");
		for (auto& object : data["objects"]) {
			RenderObject obj = RenderObject::FromJson(object);
			obj.Material = result->Materials[Guid(object["material"])];
			result->Objects.push_back(obj);
		}

		LOG_ASSERT(data["lights"].is_array(), "Lights not present in scene!");
		for (auto& light : data["lights"]) {
			result->Lights.push_back(Light::FromJson(light));
		}

		// Create and load camera config
		result->Camera = Camera::Create();
		result->Camera->SetPosition(ParseJsonVec3(data["camera"]["position"]));
		result->Camera->SetForward(ParseJsonVec3(data["camera"]["normal"]));

		return result;
	}

	/// <summary>
	/// Converts this object into it's JSON representation for storage
	/// </summary>
	nlohmann::json ToJson() const {
		nlohmann::json blob;
		// Save the default shader (really need a material class)
		blob["default_shader"] = BaseShader->GetGUID().str();

		// Save materials (TODO: this should be managed by the ResourceManager)
		std::vector<nlohmann::json> materials;
		materials.resize(Materials.size());
		int ix = 0;
		for (auto& [key, value] : Materials) {
			materials[ix] = value->ToJson();
			ix++;
		}
		blob["materials"] = materials;

		// Save renderables
		std::vector<nlohmann::json> objects;
		objects.resize(Objects.size());
		for (int ix = 0; ix < Objects.size(); ix++) {
			objects[ix] = Objects[ix].ToJson();
		}
		blob["objects"] = objects;

		// Save lights
		std::vector<nlohmann::json> lights;
		lights.resize(Lights.size());
		for (int ix = 0; ix < Lights.size(); ix++) {
			lights[ix] = Lights[ix].ToJson();
		}
		blob["lights"] = lights;

		// Save camera info
		blob["camera"] = {
			{"position", GlmToJson(Camera->GetPosition()) },
			{"normal",   GlmToJson(Camera->GetForward()) }
		};

		return blob;
	}

	/// <summary>
	/// Saves this scene to an output JSON file
	/// </summary>
	/// <param name="path">The path of the file to write to</param>
	void Save(const std::string& path) {
		// Save data to file
		FileHelpers::WriteContentsToFile(path, ToJson().dump());
		LOG_INFO("Saved scene to \"{}\"", path);
	}

	/// <summary>
	/// Loads a scene from an input JSON file
	/// </summary>
	/// <param name="path">The path of the file to read from</param>
	/// <returns>A new scene loaded from the file</returns>
	static Scene::Sptr Load(const std::string& path) {
		LOG_INFO("Loading scene from \"{}\"", path);
		std::string content = FileHelpers::ReadFile(path);
		nlohmann::json blob = nlohmann::json::parse(content);
		return FromJson(blob);
	}
};

/// <summary>
/// Handles setting the shader uniforms for our light structure in our array of lights
/// </summary>
/// <param name="shader">The pointer to the shader</param>
/// <param name="uniformName">The name of the uniform (ex: u_Lights)</param>
/// <param name="index">The index of the light to set</param>
/// <param name="light">The light data to copy over</param>
void SetShaderLight(const Shader::Sptr& shader, const std::string& uniformName, int index, const Light& light) {
	std::stringstream stream;
	stream << uniformName << "[" << index << "]";
	std::string name = stream.str();

	// Set the shader uniforms for the light
	shader->SetUniform(name + ".Position", light.Position);
	shader->SetUniform(name + ".Color", light.Color);
	shader->SetUniform(name + ".Attenuation", light.Attenuation);
}

/// <summary>
/// Creates the shader and sets up all the lights
/// </summary>
void SetupShaderAndLights(const Shader::Sptr& shader, Light* lights, int numLights) {
	// Global light params
	shader->SetUniform("u_AmbientCol", glm::vec3(0.1f));

	// Send the lights to our shader
	shader->SetUniform("u_NumLights", numLights);
	for (int ix = 0; ix < numLights; ix++) {
		SetShaderLight(shader, "u_Lights", ix, lights[ix]);
	}
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
bool DrawLightImGui(const char* title, Light& light) {
	bool result = false;
	ImGui::PushID(&light); // We can also use pointers as numbers for unique IDs
	if (ImGui::CollapsingHeader(title)) {
		result |= ImGui::DragFloat3("Pos", &light.Position.x, 0.01f);
		result |= ImGui::ColorEdit3("Col", &light.Color.r);
		result |= ImGui::DragFloat("Range", &light.Range, 0.1f);
	}
	ImGui::PopID();
	if (result) {
		light.Attenuation = 1.0f / (light.Range + 1.0f);
	}
	return result;
}

//////////////////////////////////////////////////////
////////////////// END OF NEW ////////////////////////
//////////////////////////////////////////////////////

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

#pragma region Legacy Code for Reference
	//static const GLfloat points[] = {
	//	-0.5f, -0.5f, 0.5f,
	//	0.5f, -0.5f, 0.5f,
	//	-0.5f, 0.5f, 0.5f
	//};
	//
	//static const GLfloat colors[] = {
	//	1.0f, 0.0f, 0.0f,
	//	0.0f, 1.0f, 0.0f,
	//	0.0f, 0.0f, 1.0f
	//};
	//
	////VBO - Vertex buffer object
	//VertexBuffer::Sptr posVbo = VertexBuffer::Create();
	//posVbo->LoadData(points, 9);
	//
	//VertexBuffer::Sptr color_vbo = VertexBuffer::Create();
	//color_vbo->LoadData(colors, 9);
	//
	//VertexArrayObject::Sptr vao = VertexArrayObject::Create();
	//vao->AddVertexBuffer(posVbo, {
	//	BufferAttribute(0, 3, AttributeType::Float, 0, NULL, AttribUsage::Position)
	//	});
	//vao->AddVertexBuffer(color_vbo, {
	//	{ 1, 3, AttributeType::Float, 0, NULL, AttribUsage::Color }
	//	});
	//
	//static const float interleaved[] = {
	//	// X      Y    Z       R     G     B
	//	 0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 0.0f,
	//	 0.5f,  0.5f, 0.5f,   0.3f, 0.2f, 0.5f,
	//	-0.5f,  0.5f, 0.5f,   1.0f, 1.0f, 0.0f,
	//	-0.5f, -0.5f, 0.5f,   1.0f, 1.0f, 1.0f
	//};
	//VertexBuffer::Sptr interleaved_vbo = VertexBuffer::Create();
	//interleaved_vbo->LoadData(interleaved, 6 * 4);
	//
	//static const uint16_t indices[] = {
	//	3, 0, 1,
	//	3, 1, 2
	//};
	//
	//IndexBuffer::Sptr interleaved_ibo = IndexBuffer::Create();
	//interleaved_ibo->LoadData(indices, 3 * 2);
	//
	//size_t stride = sizeof(float) * 6;
	//VertexArrayObject::Sptr vao2 = VertexArrayObject::Create();
	//vao2->AddVertexBuffer(interleaved_vbo, {
	//	BufferAttribute(0, 3, AttributeType::Float, stride, 0, AttribUsage::Position),
	//	BufferAttribute(1, 3, AttributeType::Float, stride, sizeof(float) * 3, AttribUsage::Color),
	//	});
	//vao2->SetIndexBuffer(interleaved_ibo);
#pragma endregion


	// Load our shaders
	Shader* shader = new Shader();
	shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", ShaderPartType::Vertex);
	shader->LoadShaderPartFromFile("shaders/frag_shader.glsl", ShaderPartType::Fragment);
	shader->Link();

	// GL states, we'll enable depth testing and backface fulling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	// Get uniform location for the model view projection
	Camera::Sptr camera = Camera::Create();
	camera->SetPosition(glm::vec3(10, 0, 0));
	camera->LookAt(glm::vec3(0.0f));
	camera->SetOrthoVerticalScale(10);



	// Create a mat4 to store our mvp (for now)
	glm::mat4 transform = glm::mat4(1.0f);
	glm::mat4 hardTransform = glm::mat4(1.0f);

	// Our high-precision timer
	double lastFrame = glfwGetTime();

	//Mesh Code Example:
	LOG_INFO("Starting mesh build");
	MeshBuilder<VertexPosCol> mesh;
	MeshFactory::AddIcoSphere(mesh, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.5f), 3);
	MeshFactory::AddCube(mesh, glm::vec3(0.0f), glm::vec3(0.5f));
	VertexArrayObject::Sptr vao3 = mesh.Bake();

	VertexArrayObject::Sptr vao4 = ObjLoader::LoadFromFile("BaseModel.obj");

	bool isRotating = true;
	bool isButtonPressed = false;
	bool isOrtho = false;

	bool drawObject = true;
	float translateObject[3] = { 0.f,0.f,0.f };
	float rotateObject[3] = { 0.f,0.f,0.f };
	float rotationDelta = 0;


	//IMGUI Init...
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	///// Game loop /////
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// WEEK 5: Input handling
		if (glfwGetKey(window, GLFW_KEY_SPACE)) {

			if (!isButtonPressed) {
				// This is the action we want to perform on key press
				//isRotating = !isRotating;
				if (!isOrtho) {
					camera->SetOrthoEnabled(true);
					camera->SetPosition(glm::vec3(8, 0, 0));
					camera->LookAt(glm::vec3(0.0f));


					isOrtho = !isOrtho;
				}
				else {
					camera->SetOrthoEnabled(false);
					camera->SetPosition(glm::vec3(0, 3, 3));
					camera->LookAt(glm::vec3(0.0f));
					isOrtho = !isOrtho;
				}
				
			}
			
			isButtonPressed = true;
		}
		else {
			
			isButtonPressed = false;
		}

		// Calculate the time since our last frame (dt)
		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);

		// TODO: Week 5 - toggle code

		// Rotate our models around the z axis


		if (isRotating) {

			transform = glm::translate(glm::mat4(1.0f), glm::vec3(0 + translateObject[0], 0 + translateObject[1], 0 + translateObject[2]));
			hardTransform = glm::rotate(glm::mat4(1.0f), static_cast<float>(rotationDelta), glm::vec3(0+rotateObject[0], 0+rotateObject[1], 1+rotateObject[2]));

		}

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Bind our shader and upload the uniform
		shader->Bind();


		glm::vec4 color(1.0f,1.0f,1.0f,1.0f);



		// Draw OBJ loaded model
		// When adding a new transform simply multiply it in the order that you want them to apply
		
		//shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * transform);
		//vao4->Draw();

		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * transform*hardTransform);

		if (drawObject) {
			vao4->Draw();
		}


		VertexArrayObject::Unbind();

		//This must be written Before glfwSwapBuffers and After any Draw Calls
		ImGui::Begin("This here is a Window...");
		ImGui::Text("Hello there young Traveler...");
		ImGui::Checkbox("Draw Object", &drawObject);

		ImGui::SliderFloat3("Position", &translateObject[0], -2.0f, 2.0f);

		ImGui::SliderFloat("DeltaRotation", &rotationDelta, -4.0f, 4.0f);
		ImGui::SliderFloat3("Rotations", &rotateObject[0], 0.f, 1.0f);

		ImGui::ColorEdit4("Color", &color.x);
		ImGui::End();


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		glfwSwapBuffers(window);

		
	}
	//Clean up the processes once the application is done.
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}