/*
Práctica 5: Optimización y Carga de Modelos
*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
//para probar el importer
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;
Model Goddard_M;
Model PataFD_M;
Model PataFI_M;
Model PataTD_M;
Model PataTI_M;
Model Cabeza_M;
Model Mandibula_M;
Model Cuerpo_M;
Model Cola_M;
Skybox skybox;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// Vertex Shader
static const char* vShader = "shaders/shader_m.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_m.frag";

void CreateObjects()
{
	printf("Entrando a CreateObjects...\n");

	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			 0.0f, -1.0f,  1.0f,	0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			 1.0f, -1.0f, -0.6f,	1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			 0.0f,  1.0f,  0.0f,	0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f,  0.0f,	0.0f, -1.0f, 0.0f,
		 10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f,  10.0f,	0.0f,  10.0f,	0.0f, -1.0f, 0.0f,
		 10.0f, 0.0f,  10.0f,	10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);
	printf("obj1 = %p\n", (void*)obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);
	printf("obj2 = %p\n", (void*)obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);
	printf("obj3 = %p\n", (void*)obj3);

	printf("Saliendo de CreateObjects. Tamano meshList = %zu\n", meshList.size());
}

void CreateShaders()
{
	printf("Entrando a CreateShaders...\n");

	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	printf("Shader creado. Tamano shaderList = %zu\n", shaderList.size());
}

int main()
{
	printf("ESTE ES MI SOURCE CORRECTO\n");

	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	glEnable(GL_DEPTH_TEST);

	CreateObjects();

	printf("Despues de CreateObjects, meshList.size() = %zu\n", meshList.size());
	for (size_t i = 0; i < meshList.size(); i++)
	{
		printf("meshList[%zu] = %p\n", i, (void*)meshList[i]);
	}

	CreateShaders();


	camera = Camera(
		glm::vec3(4.0f, 1.0f, 6.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-60.0f,
		0.0f,
		0.3f,
		1.0f
	);

	Goddard_M = Model();
	Goddard_M.LoadModel("Models/goddard_base.obj");

	PataFD_M = Model();
	PataFD_M.LoadModel("Models/pata_fd.obj");

	PataFI_M = Model();
	PataFI_M.LoadModel("Models/pata_fi.obj");

	PataTD_M = Model();
	PataTD_M.LoadModel("Models/pata_td.obj");

	PataTI_M = Model();
	PataTI_M.LoadModel("Models/pata_ti.obj");

	Cabeza_M = Model();
	Cabeza_M.LoadModel("Models/cabeza.obj");

	Mandibula_M = Model();
	Mandibula_M.LoadModel("Models/mandibula.obj");

	Cuerpo_M = Model();
	Cuerpo_M.LoadModel("Models/cuerpo.obj");

	Cola_M = Model();
	Cola_M.LoadModel("Models/cola.obj");

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;

	glm::mat4 projection = glm::perspective(
		45.0f,
		(GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
		0.1f,
		1000.0f
	);

	glm::mat4 model(1.0f);
	glm::mat4 modelaux(1.0f);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	// Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		// Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Se dibuja el Skybox
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

		if (shaderList.empty())
		{
			printf("shaderList esta vacio\n");
			mainWindow.swapBuffers();
			continue;
		}

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		// INICIA DIBUJO DEL PISO
		color = glm::vec3(0.5f, 0.5f, 0.5f);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		if (meshList.size() > 2 && meshList[2] != nullptr)
		{
			printf("Antes de RenderMesh, meshList[2] = %p\n", (void*)meshList[2]);
			meshList[2]->RenderMesh();
		}
		else
		{
			printf("meshList[2] no es valido\n");
		}

		// ------------ INICIA DIBUJO DE LOS DEMAS OBJETOS ------------
		// Goddard
		color = glm::vec3(1.0f, 0.0f, 1.0f); // corregido
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(1.0f, -2.0f, -5.5f));
		model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0, 1, 0));

		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Goddard_M.RenderModel();

		color = glm::vec3(0.0f, 0.0f, 1.0f);

		// CUERPO
		color = glm::vec3(0.768f, 0.768f, 0.768f);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.61f, -1.5f));
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion15()), glm::vec3(0.0f, 0.0f, 1.0f));
		modelaux = model;
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Cuerpo_M.RenderModel();

		// CABEZA
		model = modelaux;
		model = glm::translate(model, glm::vec3(1.5f, 0.5f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Cabeza_M.RenderModel();

		// MANDIBULA
		model = modelaux;
		color = glm::vec3(0.256f, 0.256f, 0.256f);
		model = glm::translate(model, glm::vec3(3.0f, 0.75f, 0.065f));
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion5()), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Mandibula_M.RenderModel();

		// PATA FRONTAL DERECHA
		model = modelaux;
		model = glm::translate(model, glm::vec3(1.25f, -0.45f, 0.75f));
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion1()), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PataFD_M.RenderModel();

		// PATA FRONTAL IZQUIERDA
		model = modelaux;
		model = glm::translate(model, glm::vec3(1.25f, -0.38f, -0.75f));
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion3()), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PataFI_M.RenderModel();

		// PATA TRASERA DERECHA
		model = modelaux;
		model = glm::translate(model, glm::vec3(-0.475f, -1.1175f, 0.75f));
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion7()), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PataTD_M.RenderModel();

		// PATA TRASERA IZQUIERDA
		model = modelaux;
		model = glm::translate(model, glm::vec3(-0.475f, -1.1175f, -0.75f));
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion9()), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		PataTI_M.RenderModel();

		// COLA
		model = modelaux;
		model = glm::translate(model, glm::vec3(-1.67f, -0.1f, 0.0f));
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion11()), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Cola_M.RenderModel();

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}