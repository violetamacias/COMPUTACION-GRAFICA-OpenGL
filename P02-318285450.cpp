// Práctica 2: índices, mesh, proyecciones, transformaciones geométricas
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <glew.h>
#include <glfw3.h>

// glm
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// clases
#include "Mesh.h"
#include "Shader.h"
#include "Window.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

// Shaders por color (vertex cambia el color, fragment es común)
static const char* fShaderSolid = "shaders/shaderColorSolid.frag";
static const char* vShaderRojo = "shaders/shaderrojo.vert";
static const char* vShaderVerde = "shaders/shaderverde.vert";
static const char* vShaderAzul = "shaders/shaderazul.vert";
static const char* vShaderCafe = "shaders/shadercafe.vert";
static const char* vShaderVerdeOscuro = "shaders/shaderverdeoscuro.vert";

// ======== Geometrías ========

// Pirámide triangular regular (índices)
void CreaPiramide()
{
    unsigned int indices[] = {
        0, 1, 2,
        1, 3, 2,
        3, 0, 2,
        1, 0, 3
    };

    GLfloat vertices[] = {
        -0.5f, -0.5f,  0.0f,   // 0
         0.5f, -0.5f,  0.0f,   // 1
         0.0f,  0.5f, -0.25f,  // 2 (punta)
         0.0f, -0.5f, -0.5f    // 3
    };

    Mesh* piramide = new Mesh();
    piramide->CreateMesh(vertices, indices, 12, 12);
    meshList.push_back(piramide); // meshList[0]
}

// Cubo (índices)
void CrearCubo()
{
    unsigned int cubo_indices[] = {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
    };

    GLfloat cubo_vertices[] = {
        // front
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        // back
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f
    };

    Mesh* cubo = new Mesh();
    cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
    meshList.push_back(cubo); // meshList[1]
}

// ======== Shaders ========

void CreateShaders()
{
    Shader sR;  sR.CreateFromFiles(vShaderRojo, fShaderSolid);             shaderList.push_back(sR);  // [0] rojo
    Shader sG;  sG.CreateFromFiles(vShaderVerde, fShaderSolid);            shaderList.push_back(sG);  // [1] verde
    Shader sB;  sB.CreateFromFiles(vShaderAzul, fShaderSolid);             shaderList.push_back(sB);  // [2] azul
    Shader sC;  sC.CreateFromFiles(vShaderCafe, fShaderSolid);             shaderList.push_back(sC);  // [3] café
    Shader sDO; sDO.CreateFromFiles(vShaderVerdeOscuro, fShaderSolid);     shaderList.push_back(sDO); // [4] verde oscuro
}

// ======== Helpers de render ========

void RenderMeshInstanced(Mesh* m,
    Shader& sh,
    const glm::vec3& pos,
    const glm::vec3& scale,
    GLuint& uniformModel,
    GLuint& uniformProjection,
    const glm::mat4& projection)
{
    sh.useShader();
    uniformModel = sh.getModelLocation();
    uniformProjection = sh.getProjectLocation();

    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

    glm::mat4 model(1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, scale);

    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    m->RenderMesh();
}

// ======== Letras CVMN como “pixel art” (5x7) ========

static const char* LETTER_C[7] = {
    "11111",
    "10000",
    "10000",
    "10000",
    "10000",
    "10000",
    "11111"
};

static const char* LETTER_V[7] = {
    "10001",
    "10001",
    "10001",
    "10001",
    "01010",
    "01010",
    "00100"
};

static const char* LETTER_M[7] = {
    "10001",
    "11011",
    "10101",
    "10001",
    "10001",
    "10001",
    "10001"
};

static const char* LETTER_N[7] = {
    "10001",
    "11001",
    "10101",
    "10011",
    "10001",
    "10001",
    "10001"
};

void RenderLetterCubes(const char* pattern[7],
    const glm::vec3& origin,
    float cubeSize,
    Shader& shaderColor,
    GLuint& uniformModel,
    GLuint& uniformProjection,
    const glm::mat4& projection)
{
    shaderColor.useShader();
    uniformModel = shaderColor.getModelLocation();
    uniformProjection = shaderColor.getProjectLocation();
    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

    // pattern: 7 filas, 5 columnas
    for (int row = 0; row < 7; row++)
    {
        for (int col = 0; col < 5; col++)
        {
            if (pattern[row][col] == '1')
            {
                glm::mat4 model(1.0f);
                float x = origin.x + col * cubeSize;
                float y = origin.y + (6 - row) * cubeSize; // invertimos para que suba
                float z = origin.z;

                model = glm::translate(model, glm::vec3(x, y, z));
                model = glm::scale(model, glm::vec3(cubeSize, cubeSize, cubeSize));

                glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
                meshList[1]->RenderMesh(); // cubo
            }
        }
    }
}

// ======== Dibujo Casa 3D (cubos + pirámides) ========

void RenderCasa(const glm::vec3& basePos,
    GLuint& uniformModel,
    GLuint& uniformProjection,
    const glm::mat4& projection)
{
    // indices de shaderList:
    // 0 rojo, 1 verde, 2 azul, 3 café, 4 verde oscuro

    // Pared principal (cubo café)
    RenderMeshInstanced(meshList[1], shaderList[3],
        basePos + glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.2f, 1.6f, 1.6f),
        uniformModel, uniformProjection, projection);

    // Puerta (cubo azul)
    RenderMeshInstanced(meshList[1], shaderList[2],
        basePos + glm::vec3(0.0f, -0.38f, 0.85f),
        glm::vec3(0.55f, 0.85f, 0.12f),

        uniformModel, uniformProjection, projection);

    // Ventanas (cubos verdes)
    RenderMeshInstanced(meshList[1], shaderList[1],
        basePos + glm::vec3(-0.75f, 0.1f, 0.85f),
        glm::vec3(0.45f, 0.45f, 0.12f),
        uniformModel, uniformProjection, projection);

    RenderMeshInstanced(meshList[1], shaderList[1],
        basePos + glm::vec3(0.75f, 0.1f, 0.85f),
        glm::vec3(0.45f, 0.45f, 0.12f),
        uniformModel, uniformProjection, projection);

    // Techo (2 pirámides rojas para “cubrir” el ancho)
    RenderMeshInstanced(meshList[0], shaderList[0],
        basePos + glm::vec3(-0.65f, 1.35f, 0.0f),
        glm::vec3(1.6f, 1.25f, 1.6f),
        uniformModel, uniformProjection, projection);

    RenderMeshInstanced(meshList[0], shaderList[0],
        basePos + glm::vec3(0.65f, 1.35f, 0.0f),
        glm::vec3(1.6f, 1.25f, 1.6f),
        uniformModel, uniformProjection, projection);

    // Pasto (cubo verde oscuro) como base del suelo
    RenderMeshInstanced(meshList[1], shaderList[4],
        basePos + glm::vec3(0.0f, -1.35f, 0.0f),
        glm::vec3(5.0f, 0.2f, 3.0f),
        uniformModel, uniformProjection, projection);

    // Árbol: tronco café + copa verde oscuro (pirámide)
    RenderMeshInstanced(meshList[1], shaderList[3],
        basePos + glm::vec3(2.6f, -0.75f, 0.0f),
        glm::vec3(0.35f, 1.0f, 0.35f),
        uniformModel, uniformProjection, projection);

    RenderMeshInstanced(meshList[0], shaderList[4],
        basePos + glm::vec3(2.6f, 0.35f, 0.0f),
        glm::vec3(1.05f, 1.05f, 1.05f),
        uniformModel, uniformProjection, projection);
}

int main()
{
    mainWindow = Window(800, 800);
    mainWindow.Initialise();

    glEnable(GL_DEPTH_TEST);

    // Geometrías
    CreaPiramide();  // meshList[0]
    CrearCubo();     // meshList[1]

    // Shaders colores
    CreateShaders();

    // Depth
    glEnable(GL_DEPTH_TEST);

    GLuint uniformProjection = 0;
    GLuint uniformModel = 0;

    // Proyección perspectiva
    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (float)mainWindow.getBufferWidth() / (float)mainWindow.getBufferHeight(),
        0.1f, 100.0f
    );

    while (!mainWindow.getShouldClose())
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ======= 1) Letras CVMN en diagonal (abajo -> arriba) =======
        glm::vec3 base(-2.2f, -1.6f, -7.0f);
        float s = 0.12f; // tamaño cubito

        // C rojo
        RenderLetterCubes(LETTER_C, base, s, shaderList[0], uniformModel, uniformProjection, projection);

        // V verde (sube y se mueve a la derecha)
        RenderLetterCubes(LETTER_V, base + glm::vec3(1.1f, 1.1f, 0.0f), s, shaderList[1], uniformModel, uniformProjection, projection);

        // M azul
        RenderLetterCubes(LETTER_M, base + glm::vec3(2.2f, 2.2f, 0.0f), s, shaderList[2], uniformModel, uniformProjection, projection);
       
       
        // N café
        RenderLetterCubes(LETTER_N, base + glm::vec3(3.3f, 3.3f, 0.0f), s, shaderList[3], uniformModel, uniformProjection, projection);

        // ======= 2) Casa (cubos + pirámides) =======
      
        RenderCasa(glm::vec3(2.0f, -1.1f, -11.0f), uniformModel, uniformProjection, projection);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}