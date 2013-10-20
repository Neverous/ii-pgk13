/* 2013
 * Maciej Szeptuch
 * II UWr
 */
#include <cstdio>
#include <cstdlib>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

int main(void)
{
    GLFWwindow *window = nullptr;
    if(!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW!\n");
        return 1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if(!(window = glfwCreateWindow(1024, 768, "Mastermind", nullptr, nullptr)))
    {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);
    if(glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    while(!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
