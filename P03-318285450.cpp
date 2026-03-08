// Práctica: Pyraminx (Rubik triangular)
// - 9 triángulos por cara (subdivisión orden 3)
// - 4 caras con colores diferentes
// - Separaciones negras entre triángulos pequeños usando triángulos negros debajo
//   y triángulos de color ligeramente más pequeños encima
//
// CONTROLES:
// - Click izquierdo + arrastrar: rotar figura
// - Click derecho + arrastrar: mover figura en X/Y
// - Scroll: mover figura en Z (acercar / alejar)
// - Teclas E, R, T: girar continuamente en X, Y, Z mientras la tecla esté presionada

#include <stdio.h>
#include <cmath>
#include <vector>

#include <glew.h>
#include <glfw3.h>

// glm
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Clases del profe
#include "Mesh.h"
#include "Shader.h"
#include "Window.h"
#include "Camera.h"

using std::vector;

// -------------------------
// Globales
// -------------------------
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

Window mainWindow;
Camera camera;
vector<Shader> shaderList;

// Rutas de shaders
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";

// -------------------------
// Control del objeto con mouse
// -------------------------
static float gScrollY = 0.0f;                           // acumulador scroll
static float rubikRotX = 25.0f;                        // rotación inicial
static float rubikRotY = -35.0f;
static float rubikRotZ = 0.0f;
static glm::vec3 rubikPos = glm::vec3(0.0f, 0.0f, -4.5f); // posición inicial
static float rubikScale = 1.2f;                        // escala fija inicial

static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gScrollY += (float)yoffset;
}

// -------------------------
// Crear shaders
// -------------------------
void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

// -------------------------
// Estructura para cada cara
// -------------------------
struct FaceMeshes
{
    Mesh* blackMesh;       // triángulos negros (bordes)
    Mesh* colorMesh;       // triángulos de color
    glm::vec3 faceColor;   // color de la cara
};

vector<FaceMeshes> pyrFaces;

// -------------------------
// Agrega un triángulo a buffers
// -------------------------
static void PushTri(vector<GLfloat>& verts, vector<unsigned int>& idx,
    const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    unsigned int base = (unsigned int)(verts.size() / 3);

    verts.push_back(a.x); verts.push_back(a.y); verts.push_back(a.z);
    verts.push_back(b.x); verts.push_back(b.y); verts.push_back(b.z);
    verts.push_back(c.x); verts.push_back(c.y); verts.push_back(c.z);

    idx.push_back(base + 0);
    idx.push_back(base + 1);
    idx.push_back(base + 2);
}

// -------------------------
// Punto baricéntrico en triángulo
// -------------------------
static glm::vec3 BaryPoint(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, float u, float v)
{
    return A * (1.0f - u - v) + B * u + C * v;
}

// -------------------------
// Construye una cara subdividida (orden N=3 => 9 triángulos)
// -------------------------
static void BuildFaceMeshes(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C,
    int N,
    float shrink,
    float offsetOut,
    const glm::vec3& faceColor)
{
    glm::vec3 tetraCenter = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 normal = glm::normalize(glm::cross(B - A, C - A));
    glm::vec3 faceCenter = (A + B + C) / 3.0f;

    // asegurar que la normal apunte hacia afuera
    if (glm::dot(normal, faceCenter - tetraCenter) < 0.0f)
    {
        normal = -normal;
    }

    vector<GLfloat> blackVerts;
    vector<unsigned int> blackIdx;

    vector<GLfloat> colorVerts;
    vector<unsigned int> colorIdx;

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < (N - i); j++)
        {
            float u0 = (float)i / (float)N;
            float v0 = (float)j / (float)N;

            float u1 = (float)(i + 1) / (float)N;
            float v1 = (float)j / (float)N;

            float u2 = (float)i / (float)N;
            float v2 = (float)(j + 1) / (float)N;

            // Triángulo 1
            glm::vec3 p0 = BaryPoint(A, B, C, u0, v0);
            glm::vec3 p1 = BaryPoint(A, B, C, u1, v1);
            glm::vec3 p2 = BaryPoint(A, B, C, u2, v2);

            PushTri(blackVerts, blackIdx, p0, p1, p2);

            {
                glm::vec3 cen = (p0 + p1 + p2) / 3.0f;

                glm::vec3 q0 = cen + (p0 - cen) * shrink + normal * offsetOut;
                glm::vec3 q1 = cen + (p1 - cen) * shrink + normal * offsetOut;
                glm::vec3 q2 = cen + (p2 - cen) * shrink + normal * offsetOut;

                PushTri(colorVerts, colorIdx, q0, q1, q2);
            }

            // Triángulo 2
            if (j < (N - i - 1))
            {
                float u3 = (float)(i + 1) / (float)N;
                float v3 = (float)(j + 1) / (float)N;

                glm::vec3 p3 = BaryPoint(A, B, C, u3, v3);

                PushTri(blackVerts, blackIdx, p1, p3, p2);

                {
                    glm::vec3 cen = (p1 + p3 + p2) / 3.0f;

                    glm::vec3 q1 = cen + (p1 - cen) * shrink + normal * offsetOut;
                    glm::vec3 q3 = cen + (p3 - cen) * shrink + normal * offsetOut;
                    glm::vec3 q2 = cen + (p2 - cen) * shrink + normal * offsetOut;

                    PushTri(colorVerts, colorIdx, q1, q3, q2);
                }
            }
        }
    }

    Mesh* mBlack = new Mesh();
    mBlack->CreateMeshGeometry(blackVerts, blackIdx,
        (unsigned int)blackVerts.size(),
        (unsigned int)blackIdx.size());

    Mesh* mColor = new Mesh();
    mColor->CreateMeshGeometry(colorVerts, colorIdx,
        (unsigned int)colorVerts.size(),
        (unsigned int)colorIdx.size());

    FaceMeshes fm;
    fm.blackMesh = mBlack;
    fm.colorMesh = mColor;
    fm.faceColor = faceColor;

    pyrFaces.push_back(fm);
}

// -------------------------
// Construye el Pyraminx
// -------------------------
void CrearPyraminx()
{
    pyrFaces.clear();

    // Tetraedro regular
    glm::vec3 V0 = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 V1 = glm::vec3(-1.0f, -1.0f, 1.0f);
    glm::vec3 V2 = glm::vec3(-1.0f, 1.0f, -1.0f);
    glm::vec3 V3 = glm::vec3(1.0f, -1.0f, -1.0f);

    int N = 3;                // 9 triángulos por cara
    float shrink = 0.88f;     // grosor de líneas negras
    float offsetOut = 0.01f;  // suficiente para que se vea el color sobre el negro

    glm::vec3 colYellow = glm::vec3(1.0f, 0.9f, 0.1f);
    glm::vec3 colRed = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 colGreen = glm::vec3(0.1f, 0.9f, 0.1f);
    glm::vec3 colPurple = glm::vec3(0.8f, 0.1f, 0.9f);

    BuildFaceMeshes(V0, V1, V2, N, shrink, offsetOut, colYellow);
    BuildFaceMeshes(V0, V2, V3, N, shrink, offsetOut, colRed);
    BuildFaceMeshes(V0, V3, V1, N, shrink, offsetOut, colGreen);
    BuildFaceMeshes(V1, V3, V2, N, shrink, offsetOut, colPurple);
}

// -------------------------
// MAIN
// -------------------------
int main()
{
    mainWindow = Window(900, 700);
    mainWindow.Initialise();

    GLFWwindow* win = glfwGetCurrentContext();
    if (win)
        glfwSetScrollCallback(win, ScrollCallback);

    glEnable(GL_DEPTH_TEST);

    CreateShaders();
    CrearPyraminx();

    // Cámara solo para View; el mouse manipula el objeto
    camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -90.0f, 0.0f,
        1.2f, 0.25f);

    GLuint uniformProjection = 0;
    GLuint uniformModel = 0;
    GLuint uniformView = 0;
    GLuint uniformColor = 0;

    glm::mat4 projection = glm::perspective(glm::radians(60.0f),
        (float)mainWindow.getBufferWidth() / (float)mainWindow.getBufferHeight(),
        0.1f, 100.0f);

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = (GLfloat)glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        glfwPollEvents();

        // Rotación continua con teclas
        if (mainWindow.getsKeys()[GLFW_KEY_E])
        {
            rubikRotX += 60.0f * deltaTime;
        }

        if (mainWindow.getsKeys()[GLFW_KEY_R])
        {
            rubikRotY += 60.0f * deltaTime;
        }

        if (mainWindow.getsKeys()[GLFW_KEY_T])
        {
            rubikRotZ += 60.0f * deltaTime;
        }

        // Si no necesitas mover la cámara con WASD, puedes comentar esta línea
        camera.keyControl(mainWindow.getsKeys(), deltaTime);

        // Control del objeto con mouse
        if (win)
        {
            float dx = mainWindow.getXChange();
            float dy = mainWindow.getYChange();

            // Click izquierdo -> rotar
            if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                rubikRotY += dx * 0.18f;
                rubikRotX += dy * 0.18f;
            }

            // Click derecho -> mover en X/Y
            if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
            {
                rubikPos.x += dx * 0.01f;
                rubikPos.y -= dy * 0.01f;
            }

            // Scroll -> mover en Z
            if (gScrollY != 0.0f)
            {
                rubikPos.z += gScrollY * 0.25f;

                if (rubikPos.z > -1.0f)  rubikPos.z = -1.0f;
                if (rubikPos.z < -20.0f) rubikPos.z = -20.0f;

                gScrollY = 0.0f;
            }
        }

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderList[0].useShader();

        uniformModel = shaderList[0].getModelLocation();
        uniformProjection = shaderList[0].getProjectLocation();
        uniformView = shaderList[0].getViewLocation();
        uniformColor = shaderList[0].getColorLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        // Modelo base del objeto
        glm::mat4 baseModel(1.0f);
        baseModel = glm::translate(baseModel, rubikPos);
        baseModel = glm::rotate(baseModel, glm::radians(rubikRotX), glm::vec3(1, 0, 0));
        baseModel = glm::rotate(baseModel, glm::radians(rubikRotY), glm::vec3(0, 1, 0));
        baseModel = glm::rotate(baseModel, glm::radians(rubikRotZ), glm::vec3(0, 0, 1));
        baseModel = glm::scale(baseModel, glm::vec3(rubikScale, rubikScale, rubikScale));

        // Dibujar caras
        // Se usa RenderMesh() porque tus índices corresponden a GL_TRIANGLES
        for (size_t f = 0; f < pyrFaces.size(); f++)
        {
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(baseModel));
            glUniform3f(uniformColor, 0.0f, 0.0f, 0.0f);
            pyrFaces[f].blackMesh->RenderMesh();

            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(baseModel));
            glUniform3fv(uniformColor, 1, glm::value_ptr(pyrFaces[f].faceColor));
            pyrFaces[f].colorMesh->RenderMesh();
        }

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}