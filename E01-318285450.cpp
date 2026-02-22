#include <stdio.h>
#include <string.h>
#include <glew.h>
#include <glfw3.h>

//Dimensiones de la ventana
const int WIDTH = 800, HEIGHT = 800;

// VAOs/VBOs para 2 figuras
GLuint VAO_Rombo, VBO_Rombo;
GLuint VAO_Trap, VBO_Trap;

GLuint shader;
GLint uColorLoc = -1; // uniform para color del objeto

// ================== SHADERS ==================

// Vertex Shader
static const char* vShader =
" \n\
#version 330 \n\
layout (location = 0) in vec3 pos; \n\
void main() \n\
{ \n\
    gl_Position = vec4(pos, 1.0); \n\
}";

// Fragment Shader (con color por uniform)
static const char* fShader =
" \n\
#version 330 \n\
out vec4 color; \n\
uniform vec4 uColor; \n\
void main() \n\
{ \n\
    color = uColor; \n\
}";

// ============== CREAR FIGURAS ==============

// Rombo (2 triángulos) — colocado a la izquierda
void CrearRombo()
{
    // Rombo centrado en x=-0.5, y=0
    // puntos: arriba, derecha, abajo, izquierda
    // lo triangulamos como: (arriba, derecha, abajo) y (arriba, abajo, izquierda)
    GLfloat vertices[] = {
        -0.50f,  0.40f, 0.0f,   // arriba
        -0.25f,  0.00f, 0.0f,   // derecha
        -0.50f, -0.40f, 0.0f,   // abajo

        -0.50f,  0.40f, 0.0f,   // arriba
        -0.50f, -0.40f, 0.0f,   // abajo
        -0.75f,  0.00f, 0.0f    // izquierda
    };

    glGenVertexArrays(1, &VAO_Rombo);
    glBindVertexArray(VAO_Rombo);

    glGenBuffers(1, &VBO_Rombo);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Rombo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Trapecio isósceles (2 triángulos) — colocado a la derecha
void CrearTrapecioIsosceles()
{
    // Trapecio centrado en x=+0.5, y=0
    // base superior más corta, base inferior más larga, simétrico
    // puntos: sup-izq, sup-der, inf-der, inf-izq
    // triangulación: (sup-izq, sup-der, inf-der) y (sup-izq, inf-der, inf-izq)
    GLfloat vertices[] = {
         0.30f,  0.25f, 0.0f,   // sup-izq
         0.70f,  0.25f, 0.0f,   // sup-der
         0.85f, -0.35f, 0.0f,   // inf-der

         0.30f,  0.25f, 0.0f,   // sup-izq
         0.85f, -0.35f, 0.0f,   // inf-der
         0.15f, -0.35f, 0.0f    // inf-izq
    };

    glGenVertexArrays(1, &VAO_Trap);
    glBindVertexArray(VAO_Trap);

    glGenBuffers(1, &VBO_Trap);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Trap);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// ============== SHADERS HELPERS ==============

void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
    GLuint theShader = glCreateShader(shaderType);

    const GLchar* theCode[1];
    theCode[0] = shaderCode;

    GLint codeLength[1];
    codeLength[0] = (GLint)strlen(shaderCode);

    glShaderSource(theShader, 1, theCode, codeLength);
    glCompileShader(theShader);

    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(theShader, sizeof(eLog), NULL, eLog);
        printf("Error al compilar shader tipo %d: %s\n", shaderType, eLog);
        return;
    }

    glAttachShader(theProgram, theShader);
}

void CompileShaders()
{
    shader = glCreateProgram();
    if (!shader)
    {
        printf("Error creando el programa shader\n");
        return;
    }

    AddShader(shader, vShader, GL_VERTEX_SHADER);
    AddShader(shader, fShader, GL_FRAGMENT_SHADER);

    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
        printf("Error al linkear: %s\n", eLog);
        return;
    }

    glValidateProgram(shader);
    glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
        printf("Error al validar: %s\n", eLog);
        return;
    }

    // Guardamos ubicación del uniform
    uColorLoc = glGetUniformLocation(shader, "uColor");
}

// ============== MAIN ==============

int main()
{
    if (!glfwInit())
    {
        printf("Falló inicializar GLFW\n");
        glfwTerminate();
        return 1;
    }

    // (Puedes bajar a 3.3 si algún equipo no soporta 4.3)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Ventana OpenGL", NULL, NULL);
    if (!mainWindow)
    {
        printf("Fallo en crearse la ventana con GLFW\n");
        glfwTerminate();
        return 1;
    }

    int BufferWidth, BufferHeight;
    glfwGetFramebufferSize(mainWindow, &BufferWidth, &BufferHeight);
    glfwMakeContextCurrent(mainWindow);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Falló inicialización de GLEW\n");
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    glViewport(0, 0, BufferWidth, BufferHeight);

    // Crear figuras y shader
    CrearRombo();
    CrearTrapecioIsosceles();
    CompileShaders();

    // ======= CONTROL DE COLOR DEL FONDO (cíclico RGB) =======
    // Periodo cómodo para ojo humano: 1.0 segundo por color
    const double SEG_POR_COLOR = 1.0;

    while (!glfwWindowShouldClose(mainWindow))
    {
        glfwPollEvents();

        // 1) Fondo cíclico: Rojo -> Verde -> Azul
        double t = glfwGetTime();
        int fase = (int)(t / SEG_POR_COLOR) % 3;

        float r = 0.0f, g = 0.0f, b = 0.0f;
        if (fase == 0) { r = 1.0f; }   // rojo
        if (fase == 1) { g = 1.0f; }   // verde
        if (fase == 2) { b = 1.0f; }   // azul

        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 2) Dibujar rombo y trapecio simultáneamente (mismo frame)
        glUseProgram(shader);

        // Color del rombo (blanco)
        glUniform4f(uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(VAO_Rombo);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Color del trapecio (amarillo)
        glUniform4f(uColorLoc, 1.0f, 1.0f, 0.0f, 1.0f);
        glBindVertexArray(VAO_Trap);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glUseProgram(0);

        glfwSwapBuffers(mainWindow);
    }

    // Limpieza (opcional)
    glfwDestroyWindow(mainWindow);
    glfwTerminate();
    return 0;
}
