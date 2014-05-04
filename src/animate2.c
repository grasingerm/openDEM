#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glfw3.h>

int main(int argc, char* argv[])
{
    glfwWindowHint(GLFW_FSAA_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

    if(!glfwInit())
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }

    // Open a window and create its OpenGL context
    GLFWwindow* window; // (In the accompanying source code, this variable is global)
    window = glfwCreateWindow( 1024, 768, "Tutorial 01", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental=true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    do{
        // Draw nothing, see you in tutorial 2 !

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
    glfwWindowShouldClose(window) == 0 );

    return 0;
}

