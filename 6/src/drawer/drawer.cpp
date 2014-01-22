#include "defines.h"
#include "drawer.h"

#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <IL/il.h>

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

    //glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    /* LOAD TEXTURES */
    log.notice("Loading textures");
    uint32_t textures = engine.gl.texture.size();
    ilInit();
    ILuint *imageID = new ILuint[textures];
    ilGenImages(textures, imageID);

    GLuint *textureID = new GLuint[textures];
    glGenTextures(textures, textureID);

    uint32_t i = 0;
    for(auto &it: engine.gl.texture)
    {
        string filename = engine.options.basedir + "/images/" + it.first;
        log.debug("Loading texture %s", filename.c_str());
        it.second = textureID[i];

        ilBindImage(imageID[i]);
        ilEnable(IL_ORIGIN_SET);
        ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

        if(ilLoadImage((ILstring) filename.c_str()))
        {
            ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
            glBindTexture(GL_TEXTURE_2D, textureID[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexImage2D(GL_TEXTURE_2D,
                0,
                ilGetInteger(IL_IMAGE_FORMAT),
                ilGetInteger(IL_IMAGE_WIDTH),
                ilGetInteger(IL_IMAGE_HEIGHT),
                0,
                ilGetInteger(IL_IMAGE_FORMAT),
                GL_UNSIGNED_BYTE,
                ilGetData());
        }

        else
            throw runtime_error("Couldnt load texture");

        ++ i;
    }

    ilDeleteImages(textures, imageID);
    delete[] imageID;
    delete[] textureID;

    // MESHES
    for(auto &mesh: engine.local.mesh)
        mesh.setup(engine.gl.texture);

    glClearColor(0x2E / 255.0, 0x34 / 255.0, 0x36 / 255.0, 1.0);
}

inline
void Drawer::drawModel(void)
{
    glUseProgram(engine.gl.program);
    glm::mat4 uniform = engine.getUniform();
    glUniformMatrix4fv(engine.gl.MVP, 1, GL_FALSE, &uniform[0][0]);

    for(auto &mesh: engine.local.mesh)
        mesh.draw();

    glUseProgram(0);
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
