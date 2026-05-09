#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "config.h"

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float     intensity;

    float constant  = 1.0f;
    float linear    = LIGHT_LINEAR;
    float quadratic = LIGHT_QUADRATIC;
};

// Te renderen lichten in de scene
inline std::vector<PointLight> buildSceneLights()
{
    return {
        { glm::vec3( 50.0f, 5.0f, 10.0f), glm::vec3(1.0f, 0.85f, 0.6f), 3.0f },
        { glm::vec3(100.0f, 5.0f, 40.0f), glm::vec3(0.6f, 0.8f,  1.0f), 3.0f },
        { glm::vec3( 90.0f, 5.0f, 75.0f), glm::vec3(1.0f, 0.5f,  0.5f), 3.0f },
        { glm::vec3( 40.0f, 5.0f, 84.0f), glm::vec3(0.8f, 1.0f,  0.8f), 3.0f },
        { glm::vec3(-10.0f, 5.0f, 60.0f), glm::vec3(1.0f, 1.0f,  0.6f), 3.0f },
        { glm::vec3(-35.0f, 5.0f, 35.0f), glm::vec3(0.8f, 0.6f,  1.0f), 3.0f },
    };
}