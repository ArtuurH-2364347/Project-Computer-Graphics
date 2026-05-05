//glad and GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//standard
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <filesystem>

// <ass imp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// stb_image
#include <stb_image/stb_image.h>

//matrix utility
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//self made files
#include <../src/shader.h>
#include <../src/camera.h>
#include <../src/mesh.h>
#include <../src/model.h>

using namespace std;

//Default settings

const int screenWidth = 800;
const int screenHeight = 600;

// camera object
Camera camera;

//function declarations
void mouse_callback(GLFWwindow *window, double posX, double posY);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// deltatime variables
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame


int main()
{
    cout << "BEGINNING OF THE DEBUG CONSOLE \n" << endl;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "F1 Computer Graphics Project", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    //capture and hide mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    //enable depth-testing
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    Shader myShader("src/shader.vs", "src/shader.fs");

    // load models
    Model myModel(filesystem::path("models/2021_F1_Mercedes-Benz_W12/2021_F1_Mercedes-Benz_W12.obj"));

    //-------------------------------------------------------------------//
    //                                                                   //
    // RENDER LOOP                                                       //
    //                                                                   //
    //-------------------------------------------------------------------//
    
    while (!glfwWindowShouldClose(window))
    {
        // i++;
        //std::cout << "NAME: " << glGetError() << " run: " << i << std::endl;

        //user input
        processInput(window);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // enable shader
        myShader.use();        


        // Model Matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));   // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));     // scaling it down
        myShader.setMat4("model", model);
        myModel.Draw(myShader);

        // Camera (View Matrix)
        glm::mat4 view = camera.GetViewMatrix();

        // Projection Matrix
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        unsigned int modelLoc = glGetUniformLocation(myShader.ID, "model");

        myShader.setMat4("model", model);
        myShader.setMat4("view", view);
        myShader.setMat4("projection", projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


//--------------------------//
//FUNCTIONS                 //
//--------------------------//
void mouse_callback(GLFWwindow *window, double posX, double posY)
{
    float dirX1 = static_cast<float>(posX);
    float dirY1 = static_cast<float>(posY);

    camera.ProcessMouse(window, dirX1, dirY1);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// user input
void processInput(GLFWwindow *window)
{
    // exiting the application
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // CALCULATING DELTATIME
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // CAMERA MOVEMENT
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}
