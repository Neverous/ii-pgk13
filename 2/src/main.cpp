/* 2013
 * Maciej Szeptuch
 * II UWr
 */
#include <cstdio>
#include <cstdlib>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "mastermind.h"

using namespace std;
using namespace glm;

mastermind::Engine engine;

void glfwErrorCallback(int error, const char* description);
void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void glfwKeyPressCallback(GLFWwindow *window, int key, int scancode, int mods);

int main(void)
{
    // Setup error callback
    glfwSetErrorCallback(glfwErrorCallback);

    GLFWwindow *window = nullptr;
    if(!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW!\n");
        return 1;
    }

    if(!(window = glfwCreateWindow(1024, 768, "Mastermind", nullptr, nullptr)))
    {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 2;
    }

    // VAO
    //GLuint VertexArrayID;
    //glGenVertexArrays(1, &VertexArrayID);
    //glBindVertexArray(VertexArrayID);

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, glfwKeyCallback);
    if(glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwTerminate();
        return -1;
    }

    while(!glfwWindowShouldClose(window))
    {
        engine.draw(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// GLFW Error Callback
void glfwErrorCallback(int/* error*/, const char* description)
{
    fputs(description, stderr);
}

void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    switch(action)
    {
        case GLFW_PRESS:
            glfwKeyPressCallback(window, key, scancode, mods);
            break;

        default:
            // Unhandled
            break;
    }
}


void glfwKeyPressCallback(GLFWwindow *window, int key, int/* scancode*/, int/* mods*/)
{
    switch(key)
    {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;

        case GLFW_KEY_ENTER:
            engine.guess.commit();
            break;

        case GLFW_KEY_LEFT:
        case GLFW_KEY_RIGHT:
            engine.guess.select(key);
            break;

        case GLFW_KEY_1:
        case GLFW_KEY_2:
        case GLFW_KEY_3:
        case GLFW_KEY_4:
        case GLFW_KEY_5:
        case GLFW_KEY_6:
        case GLFW_KEY_UP:
        case GLFW_KEY_DOWN:
            engine.guess.change(key);
            break;

        default:
            // Unhandled
            break;
    }
}
