#include <Logging.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <json.hpp>
#include <fstream>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexArrayObject.h"
#include "Shader.h"
#include "Camera.h"

#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "VertexTypes.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
	camera->SetPosition(glm::vec3(0, 3, 3));
	camera->LookAt(glm::vec3(0.0f));
	camera->SetOrthoVerticalScale(10);


	// Create a mat4 to store our mvp (for now)
	glm::mat4 transform = glm::mat4(1.0f);

	// Our high-precision timer
	double lastFrame = glfwGetTime();

	//Mesh Code Example:
	LOG_INFO("Starting mesh build");
	MeshBuilder<VertexPosCol> mesh;
	MeshFactory::AddIcoSphere(mesh, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.5f), 3);
	MeshFactory::AddCube(mesh, glm::vec3(0.0f), glm::vec3(0.5f));
	VertexArrayObject::Sptr vao3 = mesh.Bake();

	VertexArrayObject::Sptr vao4 = ObjLoader::LoadFromFile("Monkey.obj");

	bool isRotating = true;
	bool isButtonPressed = false;
	bool isOrtho = false;

	bool drawObject = true;
	float translateObjectX = 0.f;
	float translateObjectY = 0.f;
	float translateObjectZ = 0.f;

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

			transform  = glm::rotate(glm::mat4(1.0f), static_cast<float>(thisFrame), glm::vec3(0, 0, 1)) *
				glm::translate(glm::mat4(1.0f), glm::vec3(0 + translateObjectX,0 + translateObjectY, 0 + translateObjectZ));

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

		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * transform);

		if (drawObject) {
			vao4->Draw();
		}


		VertexArrayObject::Unbind();

		//This must be written Before glfwSwapBuffers and After an Draw Calls
		ImGui::Begin("This here is a Window...");
		ImGui::Text("Hello there young Traveler...");
		ImGui::Checkbox("Draw Object", &drawObject);
		ImGui::SliderFloat("Position X", &translateObjectX, -2.0f, 2.0f);
		ImGui::SliderFloat("Position Y", &translateObjectY, -2.0f, 2.0f);
		ImGui::SliderFloat("Position Z", &translateObjectZ, -2.0f, 2.0f);
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