//Alumna: Carmen Violeta Macias Niño
//Iniciales utilizadas: CVN

#include <stdio.h>      // printf: imprimir mensajes en consola
#include <string.h>     // strlen: obtener longitud del texto del shader
#include <time.h>       // time: obtener tiempo actual para semilla random
#include <cstdlib>      // rand, srand, RAND_MAX
#include <cmath>        // sqrtf: para longitud de vector (diagonales)
#include <glew.h>       // GLEW: cargar funciones modernas de OpenGL
#include <glfw3.h>      // GLFW: crear ventana y manejar eventos

// Tamaño de la ventana
const int WIDTH = 800;                          // Ancho de la ventana en pixeles
const int HEIGHT = 800;                         // Alto de la ventana en pixeles

// VAO/VBO para las letras (un solo paquete de vértices)
GLuint VAO_Letras = 0;                          // VAO: guarda configuración de atributos
GLuint VBO_Letras = 0;                          // VBO: guarda vértices en la GPU

// Shader program (vertex + fragment)
GLuint shaderProgram = 0;                       // Programa final de shaders
GLint uColorLoc = -1;                           // Ubicación del uniform uColor dentro del shader

// Cantidad de vértices de las letras (para glDrawArrays)
int LetrasVertexCount = 0;                      // Número de vértices a dibujar para las letras

// ===================== SHADERS (GPU) =====================

// Vertex Shader: recibe posición y la manda a pantalla
static const char* vShader =
" \n\
#version 330 \n\
layout (location = 0) in vec3 pos; \n\
void main() \n\
{ \n\
    gl_Position = vec4(pos, 1.0); \n\
}";

// Fragment Shader: pinta todo con el color uColor
static const char* fShader =
" \n\
#version 330 \n\
out vec4 color; \n\
uniform vec4 uColor; \n\
void main() \n\
{ \n\
    color = uColor; \n\
}";

// ===================== HELPERS: crear triángulos =====================

// Agrega un triángulo al arreglo "out" (cada punto es x,y,z)
static void AddTri(float* out, int* idx,
    float x1, float y1,
    float x2, float y2,
    float x3, float y3)
{
    out[(*idx)++] = x1;                         // x del vértice 1
    out[(*idx)++] = y1;                         // y del vértice 1
    out[(*idx)++] = 0.0f;                       // z del vértice 1 (plano 2D)

    out[(*idx)++] = x2;                         // x del vértice 2
    out[(*idx)++] = y2;                         // y del vértice 2
    out[(*idx)++] = 0.0f;                       // z del vértice 2

    out[(*idx)++] = x3;                         // x del vértice 3
    out[(*idx)++] = y3;                         // y del vértice 3
    out[(*idx)++] = 0.0f;                       // z del vértice 3
}

// Agrega un rectángulo usando 2 triángulos (sirve para barras de letras)
static void AddRect(float* out, int* idx,
    float x1, float y1,          // esquina inferior izquierda
    float x2, float y2)          // esquina superior derecha
{
    // Triángulo 1: (x1,y1) -> (x2,y1) -> (x2,y2)
    AddTri(out, idx, x1, y1, x2, y1, x2, y2);  // primer triángulo del rectángulo

    // Triángulo 2: (x1,y1) -> (x2,y2) -> (x1,y2)
    AddTri(out, idx, x1, y1, x2, y2, x1, y2);  // segundo triángulo del rectángulo
}

// Agrega un cuadrilátero (paralelogramo) usando 2 triángulos
static void AddQuad(float* out, int* idx,
    float ax, float ay,          // punto A
    float bx, float by,          // punto B
    float cx, float cy,          // punto C
    float dx, float dy)          // punto D
{
    // Triángulo 1: A-B-C
    AddTri(out, idx, ax, ay, bx, by, cx, cy);  // primer triángulo del quad

    // Triángulo 2: A-C-D
    AddTri(out, idx, ax, ay, cx, cy, dx, dy);  // segundo triángulo del quad
}

// Agrega un "trazo diagonal" con grosor: se crea un quad (2 triángulos)
static void AddDiagonalStroke(float* out, int* idx,
    float x1, float y1, // punto inicio
    float x2, float y2, // punto fin
    float thickness)    // grosor del trazo
{
    float dx = x2 - x1;                          // componente x del vector dirección
    float dy = y2 - y1;                          // componente y del vector dirección

    float len = sqrtf(dx * dx + dy * dy);            // longitud del vector
    if (len < 0.00001f) return;                  // si es muy pequeño, no dibujamos

    float px = -dy / len;                        // perpendicular x (normalizada)
    float py = dx / len;                        // perpendicular y (normalizada)

    float ht = thickness * 0.5f;                 // medio grosor para expandir a ambos lados

    float ax = x1 + px * ht;                       // A: inicio + perpendicular
    float ay = y1 + py * ht;                       // A y
    float bx = x1 - px * ht;                       // B: inicio - perpendicular
    float by = y1 - py * ht;                       // B y
    float cx = x2 - px * ht;                       // C: fin   - perpendicular
    float cy = y2 - py * ht;                       // C y
    float dxq = x2 + px * ht;                       // D: fin   + perpendicular
    float dyq = y2 + py * ht;                       // D y

    AddQuad(out, idx, ax, ay, bx, by, cx, cy, dxq, dyq); // quad = 2 triángulos
}

// ===================== CREAR LETRAS CVN (con triángulos) =====================

static void CrearIniciales_CVN()
{
    float vertices[1200];                        // arreglo grande para todos los vértices
    int idx = 0;                                 // índice de floats usados (x,y,z)

    float w = 0.22f;                             // ancho base de letra
    float h = 0.32f;                             // alto base de letra
    float t = 0.05f;                             // grosor de los trazos (barras y diagonales)

    // Posiciones en diagonal (abajo -> arriba)
    float Cx = -0.88f, Cy = -0.20f;              // 
    float Vx = -0.25f, Vy = -0.20f;              //en el centro
    float Nx = 0.35f, Ny = -0.20f;              //

    // ----- LETRA C: 3 barras (vertical izq, arriba, abajo) -----
    AddRect(vertices, &idx, Cx, Cy, Cx + t, Cy + h);       // barra vertical izquierda
    AddRect(vertices, &idx, Cx, Cy + h - t, Cx + w, Cy + h);       // barra superior
    AddRect(vertices, &idx, Cx, Cy, Cx + w, Cy + t);       // barra inferior

    // ----- LETRA V: 2 diagonales que se juntan abajo -----
    AddDiagonalStroke(vertices, &idx,
        Vx, Vy + h,              // punto alto izquierdo
        Vx + w * 0.5f, Vy,         // punto bajo central
        t);                      // grosor del trazo

    AddDiagonalStroke(vertices, &idx,
        Vx + w, Vy + h,          // punto alto derecho
        Vx + w * 0.5f, Vy,         // punto bajo central
        t);                      // grosor del trazo

    // ----- LETRA N: 2 verticales + 1 diagonal -----

    AddRect(vertices, &idx, Nx, Ny, Nx + t, Ny + h);       // barra vertical izquierda
    AddRect(vertices, &idx, Nx + w - t, Ny, Nx + w, Ny + h);       // barra vertical derecha


    AddDiagonalStroke(vertices, &idx,
        Nx + t, Ny + h,            // punto abajo-izq (dentro de la barra izq)
        Nx + w - t, Ny,      // punto arriba-der (dentro de la barra der)
        t);                      // grosor del trazo

    // Subir los vértices a la GPU usando VAO/VBO
    glGenVertexArrays(1, &VAO_Letras);          // crea un VAO
    glBindVertexArray(VAO_Letras);              // lo activa

    glGenBuffers(1, &VBO_Letras);               // crea un VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Letras);  // lo activa como buffer de vértices

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * idx, vertices, GL_STATIC_DRAW); // copia datos a GPU

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0); // atributo 0 = vec3
    glEnableVertexAttribArray(0);               // habilita atributo 0

    glBindBuffer(GL_ARRAY_BUFFER, 0);           // desactiva VBO
    glBindVertexArray(0);                       // desactiva VAO

    LetrasVertexCount = idx / 3;                // convertimos floats a cantidad de vértices
}

// ===================== COMPILAR SHADERS =====================

// Compila un shader (vertex o fragment) y lo adjunta al programa
static void AddShader(GLuint program, const char* code, GLenum type)
{
    GLuint sh = glCreateShader(type);           // crea shader del tipo indicado

    const GLchar* src[1] = { code };            // arreglo de punteros al texto del shader
    GLint lengths[1] = { (GLint)strlen(code) }; // longitud del texto

    glShaderSource(sh, 1, src, lengths);        // asigna código al shader
    glCompileShader(sh);                        // compila

    GLint ok = 0;                               // variable para revisar estado
    GLchar log[1024] = { 0 };                   // buffer para errores

    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);  // revisa si compiló
    if (!ok)                                    
    {
        glGetShaderInfoLog(sh, sizeof(log), NULL, log); // obtiene error
        printf("Error compilando shader %d: %s\n", type, log); 
        return;                                 
    }

    glAttachShader(program, sh);                // adjunta shader al programa
}

// Crea el programa final y obtiene uniform uColor
static void CompileShaders()
{
    shaderProgram = glCreateProgram();          // crea el programa de shaders
    if (!shaderProgram)                        
    {
        printf("Error creando shader program\n");
        return;                                 
    }

    AddShader(shaderProgram, vShader, GL_VERTEX_SHADER);     // compila + adjunta vertex shader
    AddShader(shaderProgram, fShader, GL_FRAGMENT_SHADER);   // compila + adjunta fragment shader

    GLint ok = 0;                               // variable para revisar link/validate
    GLchar log[1024] = { 0 };                   // buffer para errores

    glLinkProgram(shaderProgram);               // enlaza shaders (link)
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &ok); // revisa link
    if (!ok)                                    
    {
        glGetProgramInfoLog(shaderProgram, sizeof(log), NULL, log); // error de link
        printf("Error linkeando: %s\n", log);    
        return;                                 
    }

    glValidateProgram(shaderProgram);           // valida el programa
    glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &ok); // revisa validate
    if (!ok)                                   
    {
        glGetProgramInfoLog(shaderProgram, sizeof(log), NULL, log); // error validate
        printf("Error validando: %s\n", log);    
        return;                              
    }

    uColorLoc = glGetUniformLocation(shaderProgram, "uColor"); // obtiene ubicación del uniform
}

// ===================== MAIN =====================

int main()
{
    if (!glfwInit())                            // inicializa GLFW
    {
        printf("Fallo inicializar GLFW\n");     // mensaje de error
        glfwTerminate();                        // cierra GLFW
        return 1;                               // termina con error
    }

    // Configuración del contexto OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);           
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);            
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);       

    // Crear ventana
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Practica CVN + Fondo Random", NULL, NULL);
    if (!window)                               
    {
        printf("Fallo crear ventana\n");      
        glfwTerminate();                       // cierra GLFW
        return 1;                              
    }

    int fbW = 0, fbH = 0;                      // variables para framebuffer
    glfwGetFramebufferSize(window, &fbW, &fbH);// obtiene tamaño real del framebuffer
    glfwMakeContextCurrent(window);            // activa el contexto de esta ventana

    glewExperimental = GL_TRUE;                // habilita carga experimental de GLEW
    if (glewInit() != GLEW_OK)                 // inicializa GLEW
    {
        printf("Fallo inicializar GLEW\n");    
        glfwDestroyWindow(window);             // destruye ventana
        glfwTerminate();                       // termina GLFW
        return 1;                              // termina con error
    }

    glViewport(0, 0, fbW, fbH);                // define área de dibujo

    srand((unsigned int)time(NULL));           // semilla random distinta en cada ejecución

    CrearIniciales_CVN();                      // crea los vértices de las letras y los sube a GPU
    CompileShaders();                          // compila y enlaza shaders

    const double PERIOD = 2.0;                 // periodicidad pedida: 2 segundos
    double lastChange = -PERIOD;               // forzar cambio inmediato al inicio

    float bgR = 0.0f, bgG = 0.0f, bgB = 0.0f;  // color actual del fondo

    while (!glfwWindowShouldClose(window))     // loop hasta cerrar ventana
    {
        glfwPollEvents();                      // procesa eventos (teclado/mouse/cerrar)

        double t = glfwGetTime();              // tiempo actual desde inicio

        if ((t - lastChange) >= PERIOD)        // si ya pasaron 2 segundos...
        {
            lastChange = t;                    // actualiza el tiempo del cambio

            bgR = (float)rand() / (float)RAND_MAX; // random 0..1 para rojo
            bgG = (float)rand() / (float)RAND_MAX; // random 0..1 para verde
            bgB = (float)rand() / (float)RAND_MAX; // random 0..1 para azul
        }

        glClearColor(bgR, bgG, bgB, 1.0f);     // establece color de limpieza (fondo)
        glClear(GL_COLOR_BUFFER_BIT);          // limpia la pantalla con ese color

        glUseProgram(shaderProgram);           // activa el shader program

        glUniform4f(uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // color de TODAS las letras (mismo color)

        glBindVertexArray(VAO_Letras);         // activa el VAO de letras
        glDrawArrays(GL_TRIANGLES, 0, LetrasVertexCount); // dibuja todos los triángulos de letras
        glBindVertexArray(0);                  // desactiva VAO

        glUseProgram(0);                       // desactiva shader (opcional)

        glfwSwapBuffers(window);               // muestra el frame (doble buffer)
    }

    glfwDestroyWindow(window);                 // destruye ventana
    glfwTerminate();                           // finaliza GLFW
    return 0;                                  // termina correcto
}