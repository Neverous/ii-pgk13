#include "defines.h"
#include "drawer.h"

#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "engine/engine.h"
#include "engine/objects.h"
#include "libs/logger/logger.h"

using namespace std;
using namespace terrain;
using namespace terrain::drawer;

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
    generateTile();
    generateGrid();

    log.notice("Started drawer");
}

void Drawer::run(void)
{
    log.debug("Running drawer");

    uint8_t adaptive    = 0;
    uint8_t lod         = 0;
    uint16_t fast       = 0;

    double  lastFrame   = 0;
    double  fps         = 0;
    for(uint16_t c = 0; state == Thread::STARTED; ++ c)
    {
        engine.updateViewport();
        lastFrame = glfwGetTime();
        lod = engine.options.lod ? (engine.options.lod - 1) * 10 : adaptive;

        glClear(GL_COLOR_BUFFER_BIT);
        drawGrid(lod / 10);
        drawTerrain(lod);
        glfwSwapBuffers(engine.gl.window);

        const double    currentFrame    = glfwGetTime();
        const double    diff            = currentFrame - lastFrame;
        fps += diff;

        // FRAMES COUNTER
        if(fps >= 2.0 || c >= 240)
        {
            log.debug("Current FPS: %.3lf", c / fps);
            c = fps = 0;
        }

        // ADAPTIVE LOD
        if(!engine.options.lod)
        {
            if(diff <= 1.0 / DRAWER_FPS && ++ fast >= DRAWER_FPS)
            {
                fast = 0;
                adaptive = max(0, adaptive - 1);
                log.debug("ADAPTIVE-: %d", adaptive);
            }

            else if(diff > 1.0 / DRAWER_FPS)
            {
                fast = 0;
                adaptive = min(adaptive + 1, DETAIL_LEVELS * 10);
                log.debug("ADAPTIVE+: %d", adaptive);
            }
        }

        // FRAMES LIMIT (~60fps)
        if(diff <= 1.0 / DRAWER_FPS)
            this_thread::sleep_for(chrono::milliseconds(static_cast<uint32_t>(1000.0 / (DRAWER_FPS - 1) - diff * 1000.0)));

        else
            log.warning("Rendering frame took: %.4lfs [LOD: %d]", diff, lod);
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
    //// INDICES
    glGenBuffers(DETAIL_LEVELS, engine.gl.gridIndice);
    glGenBuffers(DETAIL_LEVELS, engine.gl.tileIndice);
    glGenBuffers(19, engine.gl.buffer);
    for(int t = 0; t < 9; ++ t)
    {
        engine.local.tile[t].order = t;
        engine.local.tile[t].buffer = engine.gl.buffer[t];
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    glClearColor(0x2E / 255.0, 0x34 / 255.0, 0x36 / 255.0, 1.0);
}

inline
void Drawer::generateTile(void)
{
    log.debug("Creating %d levels of detail tables for tiles", DETAIL_LEVELS);
    const uint32_t density = (1 << DETAIL_LEVELS) + 1;
    for(int l = 0; l < DETAIL_LEVELS; ++ l)
    {
        const uint32_t  tileDensity = (1 << (DETAIL_LEVELS - l));
        const uint32_t  tileStep    = (1 << l);
        engine.local.tileSize[l]     = tileDensity * tileDensity * 6;
        uint32_t *indice = new uint32_t[engine.local.tileSize[l]];
        uint32_t c = 0;
        for(uint16_t h = 0; h < tileDensity; ++ h)
            for(uint16_t w = 0; w < tileDensity; ++ w)
            {
                uint32_t current    = density * tileStep * h + tileStep * w,
                         next       = current + density * tileStep;

                indice[c ++] = current + tileStep;
                indice[c ++] = next;
                indice[c ++] = current;

                indice[c ++] = next + tileStep;
                indice[c ++] = next;
                indice[c ++] = current + tileStep;
            }

        assert(c == engine.local.tileSize[l]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, engine.gl.tileIndice[l]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, c * sizeof(uint32_t), indice, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        delete[] indice;
    }
}

inline
void Drawer::generateGrid(void)
{
    log.debug("Creating background grid");
    const uint32_t density = (1 << DETAIL_LEVELS) + 1;
    {
        objects::Position   *point = new objects::Position[density * density];
        uint32_t p = 0;
        for(uint16_t h = 0; h < density; ++ h)
            for(uint16_t w = 0; w < density; ++ w)
            {
                point[p ++] = objects::Position(
                    -MERCATOR_BOUNDS + w * MERCATOR_BOUNDS * 2.0 / (density - 1),
                    -MERCATOR_BOUNDS + h * MERCATOR_BOUNDS * 2.0 / (density - 1),
                    0.0);
            }

        assert(p == density * density);
        glBindBuffer(GL_ARRAY_BUFFER, engine.gl.buffer[engine::GRID_BUFFER]);
        glBufferData(GL_ARRAY_BUFFER, p * sizeof(objects::Position), point, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        delete[] point;
    }

    log.debug("Creating %d levels of detail tables for grid", DETAIL_LEVELS);
    for(int l = 0; l < DETAIL_LEVELS; ++ l)
    {
        const uint32_t  gridDensity = (1 << (DETAIL_LEVELS - l)) + 1;
        const uint32_t  gridStep    = (1 << l);
        engine.local.gridSize[l]     = gridDensity * (gridDensity - 1) * 4;
        uint32_t *indice = new uint32_t[engine.local.gridSize[l]];
        uint32_t c = 0;
        for(uint16_t h = 0; h < gridDensity; ++ h)
            for(uint16_t w = 0; w < gridDensity - 1; ++ w)
            {
                uint32_t current    = density * gridStep * h + gridStep * w;
                indice[c ++] = current;
                indice[c ++] = current + gridStep;
            }

        for(uint16_t w = 0; w < gridDensity; ++ w)
            for(uint16_t h = 0; h < gridDensity - 1; ++ h)
            {
                uint32_t current    = density * gridStep * h + gridStep * w,
                         next       = current + density * gridStep;

                indice[c ++] = current;
                indice[c ++] = next;
            }

        assert(c == engine.local.gridSize[l]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, engine.gl.gridIndice[l]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, c * sizeof(uint32_t), indice, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        delete[] indice;
    }
}

inline
void Drawer::drawTerrain(int lod)
{
    for(uint8_t t = 0; t < 9; ++ t)
        drawTile(engine.local.tile[t], max(0, lod + t - 9) / 10);
}

inline
void Drawer::drawGrid(int lod)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, engine.gl.gridIndice[lod]);
    glBindBuffer(GL_ARRAY_BUFFER, engine.gl.buffer[engine::GRID_BUFFER]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glUseProgram(getProgram(GRID_PROGRAM));

    glm::mat4 uniform = engine.getUniform();
    glUniformMatrix4fv(getMVP(GRID_PROGRAM), 1, GL_FALSE, &uniform[0][0]);

    glDrawElements(GL_LINES, engine.local.gridSize[lod], GL_UNSIGNED_INT, nullptr);

    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

inline
void Drawer::drawTile(const objects::Tile &tile, int lod)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, engine.gl.tileIndice[lod]);
    glBindBuffer(GL_ARRAY_BUFFER, tile.buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_UNSIGNED_SHORT, GL_FALSE, 0, nullptr);
    glUseProgram(getProgram(TILE_PROGRAM));

    glm::mat4 uniform = engine.getUniform();
    glUniformMatrix4fv(getMVP(TILE_PROGRAM), 1, GL_FALSE, &uniform[0][0]);
    glUniform4fv(getBOX(TILE_PROGRAM), 1, &tile.box.x);

    glDrawElements(GL_TRIANGLES, engine.local.tileSize[lod], GL_UNSIGNED_INT, nullptr);

    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

inline
void Drawer::loadPrograms(void)
{
    log.debug("Loading programs");
    engine.gl.program[engine::VIEW_2D][GRID_PROGRAM] = loadProgram("src/shaders/2d/grid.vertex.glsl", "src/shaders/2d/grid.fragment.glsl");
    engine.gl.MVP[engine::VIEW_2D][GRID_PROGRAM] = glGetUniformLocation(engine.gl.program[engine::VIEW_2D][GRID_PROGRAM], "MVP");

    engine.gl.program[engine::VIEW_2D][TILE_PROGRAM] = loadProgram("src/shaders/2d/tile.vertex.glsl", "src/shaders/2d/tile.fragment.glsl");
    engine.gl.MVP[engine::VIEW_2D][TILE_PROGRAM] = glGetUniformLocation(engine.gl.program[engine::VIEW_2D][TILE_PROGRAM], "MVP");
    engine.gl.BOX[engine::VIEW_2D][TILE_PROGRAM] = glGetUniformLocation(engine.gl.program[engine::VIEW_2D][TILE_PROGRAM], "box");

    engine.gl.program[engine::VIEW_3D][GRID_PROGRAM] = loadProgram("src/shaders/3d/grid.vertex.glsl", "src/shaders/3d/grid.fragment.glsl");
    engine.gl.MVP[engine::VIEW_3D][GRID_PROGRAM] = glGetUniformLocation(engine.gl.program[engine::VIEW_3D][GRID_PROGRAM], "MVP");

    engine.gl.program[engine::VIEW_3D][TILE_PROGRAM] = loadProgram("src/shaders/3d/tile.vertex.glsl", "src/shaders/3d/tile.fragment.glsl");
    engine.gl.MVP[engine::VIEW_3D][TILE_PROGRAM] = glGetUniformLocation(engine.gl.program[engine::VIEW_3D][TILE_PROGRAM], "MVP");
    engine.gl.BOX[engine::VIEW_3D][TILE_PROGRAM] = glGetUniformLocation(engine.gl.program[engine::VIEW_3D][TILE_PROGRAM], "box");
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
GLuint &Drawer::getProgram(int view)
{
    return engine.gl.program[engine.options.viewType][view];
}

inline
GLuint &Drawer::getMVP(int view)
{
    return engine.gl.MVP[engine.options.viewType][view];
}

inline
GLuint &Drawer::getBOX(int view)
{
    return engine.gl.BOX[engine.options.viewType][view];
}

inline
void Drawer::throwError(const char *message)
{
    glfwDestroyWindow(engine.gl.window);
    glfwTerminate();

    throw runtime_error(message);
}
