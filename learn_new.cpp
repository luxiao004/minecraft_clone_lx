#include <iostream>

#include "shader.h"
//#include "Vertex.h"
#include "Texture.h"
//#include "modellx.h"
#include "cameralx.h"
#include "ChunkManager.h"
//#include "Chunk.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


//----------------------参数设置(start)-------------------------
#pragma region
	const int screenWidth = 800;
	const int screenHeight = 600;
	vec3 lightPos[4] = { vec3(5, 2, 5),vec3(-5, 2, -5),vec3(5, 2, -5),vec3(-5, 2, 5) };
	float lightSpeed = 2.0;
	float lightRange = 10.0;
	vec3 lightColor = vec3(1);
	bool mouseCapture = true;
	
	vec3 camPos(20.0, 70.0, 8.0);
	Camera camera(45, 10., 0.05, camPos);
#pragma endregion
//---------------------参数设置(end)------------------------

//----------------------回调函数(start)-------------------------
#pragma region
	//鼠标pos
	void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
		if (mouseCapture)
			camera.mousePos(xpos, ypos);
	}
	//滚轮
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
		camera.mouseScroll(yoffset);
	}
	//键盘
	void key_callback(GLFWwindow* window, int, int b, int c, int d) {
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
			if (mouseCapture)
				mouseCapture = false;
			else
				mouseCapture = true;
	}
	//模型导入
	void loadModel(string path)
	{
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
			return;
		}
	}
#pragma endregion
//---------------------回调函数(end)------------------------




int main() {
//----------------------初始化(start)-------------------------
#pragma region
	glfwInit();
	//指定版本
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "opengl yyds", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window,scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}
	glEnable(GL_DEPTH_TEST);

	const char* vs = "resources/shaders/vertexShader.glsl";
	const char* fs = "resources/shaders/fragmentShader.glsl";
	Shader shader(vs, fs);
	
#pragma endregion
//---------------------初始化(end)----------------------------

//----------------------顶点数据(start)-------------------------
#pragma region
	//Vertex vertex(vertices, sizeof(vertices), indices, sizeof(indices), prop, propSize);//载入模型
	//int vertexCount = sizeof(vertices) / (vertex.sumProp * sizeof(float));
	//ChunkManager chunkManager(2);
	Chunk chunk(ChunkCoord{0,0});
#pragma endregion
//---------------------顶点数据(end)----------------------------

//----------------------纹理(start)-------------------------
#pragma region
	static unsigned char* data;
	texture("resources/textures/blocks.png", 0,true, data,GL_RGBA,GL_NEAREST);
	texture("resources/textures/container2_specular.png", 1, true, data,GL_RGBA,GL_NEAREST);
#pragma endregion
//---------------------纹理(end)------------------------

//----------------------GUI参数(start)-------------------------
#pragma region
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();(void)io;
	ImGui::StyleColorsDark();//主题颜色
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	bool drawMesh = true;
	float light_pos = 0;
	vec3 light_color = vec3(1, 1, 1);
	float light_Int = 1;
	int counter = 0;
	float xxx=1;
	bool debug = false;
	float outRange = 30;
	float inRange = 0;
	int chunkCount = 0;
	int blockCount = 0;
#pragma endregion
//---------------------GUI参数(end)----------------------------
	
	
//---------------------窗口循环(start)---------------------------

#pragma region
	//shader常量设置
	while (!glfwWindowShouldClose(window)) {
		//输入
		camera.processInput(window);
		//鼠标是否隐藏
		if (mouseCapture)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		//获取时间变量
		float time = glfwGetTime();

		
		
		//指定清空颜色
		glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // 天空蓝
		//清空屏幕（缓冲位）
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		//变换矩阵
		mat4 model(1);
		mat4 view = camera.GetV();
		mat4 projection = perspective(radians(camera.fov), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		shader.use();
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		
		glActiveTexture(GL_TEXTURE0);
		//更新区块
		//chunkManager.update(camera.cameraPos);
		
		
		//chunkManager.render();
		chunk.render();
		
		
		//GUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("hi");
			ImGui::Text("hi xjj.");
			ImGui::Checkbox("DrawLight", &drawMesh);
			ImGui::Checkbox("debug", &debug);
			ImGui::SliderFloat("LightPosition", &light_pos, 0.0f, 3.14f); 
			ImGui::SliderFloat("LightIntencity", &light_Int, 0.0f, 10.0f);
			ImGui::SliderFloat("xxx", &xxx, 0.0f, 10.0f);
			ImGui::ColorEdit3("Light color", (float*)&light_color);
			if (ImGui::Button("TestButton"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//检查有没有键盘 鼠标 事件更新窗口状态
		glfwPollEvents();
		//交换画好的缓冲 发送给 window
		glfwSwapBuffers(window);
	}
#pragma endregion
//--------------------窗口循环(end)-----------------------------
	//清理缓存
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	//vertex.deleteBuffer();
	glfwTerminate();
	return 0;
}