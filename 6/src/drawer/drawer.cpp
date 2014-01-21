#include "defines.h"
#include "drawer.h"

#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "engine/engine.h"
#include "libs/logger/logger.h"

using namespace std;
using namespace viewer;
using namespace viewer::drawer;

Drawer::Drawer(Log &_log, engine::Engine &_engine)
:log(_log, "DRAWER")
,engine(_engine)
{
}

Drawer::~Drawer(void)
{
}


void Drawer::start(void)
{
    pthread_setname_np(handle.native_handle(), "Drawer");

    log.debug("Starting drawer");
    setupGL();
    loadPrograms();

    log.notice("Started drawer");
}

void Drawer::run(void)
{
    log.debug("Running drawer");

    double  lastFrame   = 0;
    while(state == Thread::STARTED)
    {
        engine.updateViewport();
        lastFrame = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawModel();
        glfwSwapBuffers(engine.gl.window);

        const double    currentFrame    = glfwGetTime();
        const double    diff            = currentFrame - lastFrame;

        // FRAMES LIMIT (~60fps)
        if(diff <= 1.0 / DRAWER_FPS)
            this_thread::sleep_for(chrono::milliseconds(static_cast<uint32_t>(1000.0 / (DRAWER_FPS - 1) - diff * 1000.0)));

        else
            log.warning("Rendering frame took: %.4lfs", diff);
    }
}

void Drawer::stop(void)
{
}

void Drawer::terminate(void)
{
}


inline
void Drawer::setupGL(void)
{
    log.debug("Setting up GL");
    glfwMakeContextCurrent(engine.gl.window);
    if(glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(engine.gl.window);
        glfwTerminate();

        throw runtime_error("GLEWInit error");
    }

    glfwSwapInterval(0);

    // GL
    glGenBuffers(1, &engine.gl.indice);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, engine.gl.indice);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, engine.local.indice.size() * sizeof(uint32_t), &engine.local.indice[0], GL_STATIC_DRAW);

    glGenBuffers(1, &engine.gl.vert);
    glBindBuffer(GL_ARRAY_BUFFER, engine.gl.vert);
    glBufferData(GL_ARRAY_BUFFER, engine.local.vert.size() * sizeof(float), &engine.local.vert[0], GL_STATIC_DRAW);

    //glEnable(GL_CULL_FACE);
    //glEnable(GL_MULTISAMPLE);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_DEPTH_TEST);

    glClearColor(0x2E / 255.0, 0x34 / 255.0, 0x36 / 255.0, 1.0);
}

inline
void Drawer::drawModel(void)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, engine.gl.indice);
    glBindBuffer(GL_ARRAY_BUFFER, engine.gl.vert);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glUseProgram(engine.gl.program);
    glm::mat4 uniform = engine.getUniform();
    glUniformMatrix4fv(engine.gl.MVP, 1, GL_FALSE, &uniform[0][0]);


    glDrawElements(GL_TRIANGLES, engine.local.indice.size(), GL_UNSIGNED_INT, nullptr);

    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

inline
void Drawer::loadPrograms(void)
{
    log.debug("Loading programs");
    engine.gl.program   = loadProgram("src/shaders/vertex.glsl", "src/shaders/fragment.glsl");
    engine.gl.MVP       = glGetUniformLocation(engine.gl.program, "MVP");
}

inline
GLuint Drawer::loadProgram(const char *vertex, const char *fragment)
{
    log.debug("Loading program %s %s", vertex, fragment);
    GLuint  vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint  fShader = glCreateShader(GL_FRAGMENT_SHADER);

    loadShader(vShader, vertex);
    loadShader(fShader, fragment);

    log.debug("Linking shaders into program");

    GLuint shader = glCreateProgram();
    glAttachShader(shader, vShader);
    glAttachShader(shader, fShader);
    glLinkProgram(shader);

    GLint status = GL_FALSE;
    glGetProgramiv(shader, GL_LINK_STATUS, &status);

    if(status == GL_FALSE)
    {
        GLint logLength = 0;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        char buffer[logLength];
        glGetProgramInfoLog(shader, logLength, nullptr, buffer);
        throwError(buffer);
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    return shader;
}

inline
void Drawer::loadShader(const GLuint shader, const char *filename)
{
    log.debug("Loading %s shader", filename);
    FILE    *file   = fopen(filename, "rb");
    if(!file)
        throwError("Cannot open shader file");

    fseek(file, 0, SEEK_END);
    uint32_t bytes = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *code = new char[bytes + 1];
    if(!code)
        throwError("Insufficient memory to read shader");

    if(fread(code, sizeof(char), bytes, file) != bytes)
        throwError("Couldn't read shader");

    code[bytes] = 0;
    fclose(file);
    file = nullptr;

    log.debug("Compiling %s shader", filename);
    glShaderSource(shader, 1, (const char **) &code, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE)
    {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        char buffer[logLength];
        glGetShaderInfoLog(shader, logLength, nullptr, buffer);
        throwError(buffer);
    }

    delete[] code;
}

inline
void Drawer::throwError(const char *message)
{
    glfwDestroyWindow(engine.gl.window);
    glfwTerminate();

    throw runtime_error(message);
}
