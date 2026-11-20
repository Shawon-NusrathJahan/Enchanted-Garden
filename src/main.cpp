
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "mountain.h"
#include "animation.h"

using namespace std;

int main()
{
    if (!glfwInit())
    {
        cout << "GLFW initialization failed!" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(
        1280, 720,
        "Enchanted Garden - Mountain Journey",
        NULL, NULL
    );

    if (window == NULL)
    {
        cout << "Window creation failed!" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
    {
        cout << "GLEW initialization failed!" << endl;
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, 1280, 720);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.48f, 0.72f, 0.92f, 1.0f);

    initializeAnimation(glfwGetTime());

    bool spacePressed = false;
    bool rPressed = false;

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();

        // Space: pause or resume
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if (!spacePressed)
                toggleAnimation();

            spacePressed = true;
        }
        else
        {
            spacePressed = false;
        }

        // R: restart animation
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            if (!rPressed)
                resetAnimation(currentTime);

            rPressed = true;
        }
        else
        {
            rPressed = false;
        }

        updateAnimation(currentTime);

        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        drawMountains(getZoom(), getParallax());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
