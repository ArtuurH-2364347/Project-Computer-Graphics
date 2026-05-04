#ifndef CAMERA_H
#define CAMERA_H


#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.1f;

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;

    glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    const int screenWidth = 800;
    const int screenHeight = 600;

    float Yaw;
    float Pitch;

    bool mouseInit = true;
    float dirX0 = screenWidth / 2;
    float dirY0 = screenHeight / 2;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f))
    {
        Position = position;
        Up = up;
        Right = glm::normalize(glm::cross(Front, Up));
        Yaw = yaw;
        Pitch = pitch;

        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, WorldUp);
    }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = SPEED * deltaTime;
        if (direction == FORWARD) //move forward
            Position += Front * velocity;
        if (direction == BACKWARD) // move backward
            Position -= Front * velocity;
        if (direction == LEFT) // move left
            Position -= Right * velocity;
        if (direction == RIGHT) // move right
            Position += Right * velocity;
        if (direction == UP) // move up (relative to front)
            Position += Up * velocity;
        if (direction == DOWN) // move down (relative to front)
            Position -= Up * velocity;
    }

    void ProcessMouse(GLFWwindow *window, float dirX1, float dirY1)
    {
        float offsetX, offsetY;

        if (mouseInit)
        {
            dirX0 = dirX1;
            dirY0 = dirY1;
            mouseInit = false;
        }

        offsetX = dirX1 - dirX0;
        offsetY = dirY0 - dirY1; // reversed since y-coordinates go from bottom to top

        dirX0 = dirX1;
        dirY0 = dirY1;

        offsetX *= SENSITIVITY;
        offsetY *= SENSITIVITY;

        Yaw += offsetX;
        Pitch += offsetY;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

#endif