#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <cstdlib>
#include "config.h"
#include "circuit.h"

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float     intensity;
    float constant  = 1.0f;
    float linear    = LIGHT_LINEAR;
    float quadratic = LIGHT_QUADRATIC;
};

inline std::vector<PointLight> buildSceneLights()
{
    std::vector<BezierSegment> circuit = buildNBRCircuit();
    std::vector<PointLight> lights;

    int   numLights   = 32;
    float heightAbove = 8.0f;
    float totalT      = (float)circuit.size();

    for (int i = 0; i < numLights; i++)
    {
        float t = (float)i / (float)numLights * totalT;
        glm::vec3 pos = sampleCircuit(circuit, t);

        glm::vec3 afgeleide = sampleCircuitAfgeleide(circuit, t);
        glm::vec3 up        = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right     = glm::normalize(glm::cross(afgeleide, up));

        float sideOffset = (i % 2 == 0) ? 6.0f : -6.0f;
        float randomX = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        float randomZ = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;

        pos += right * sideOffset;
        pos.x += randomX;
        pos.y += heightAbove + (((float)rand() / RAND_MAX) * 1.5f);
        pos.z += randomZ;

        glm::vec3 color;
        float intensity;

        if (i % 5 == 0)
        {
            // fellere lichten
            color     = glm::vec3(1.0f, 1.0f, 1.2f);
            intensity = 8.0f;
        }
        else
        {
            // normale lichten
            color     = (i % 2 == 0) ? glm::vec3(1.0f, 0.85f, 0.65f) : glm::vec3(0.70f, 0.85f, 1.0f);
            intensity = 4.0f;
        }

        PointLight light;
        light.position  = pos;
        light.color     = color;
        light.intensity = intensity;
        
        if (i % 5 == 0) {
            light.linear    = LIGHT_LINEAR * 0.8f;
            light.quadratic = LIGHT_QUADRATIC * 0.6f;
        } else {
            light.linear    = LIGHT_LINEAR;
            light.quadratic = LIGHT_QUADRATIC;
        }

        lights.push_back(light);
    }

    return lights;
}

inline void applyLights(Shader& shader, const std::vector<PointLight>& lights)
{
    for (int i = 0; i < (int)lights.size(); i++)
    {
        std::string base = "pointLights[" + std::to_string(i) + "].";
        shader.setVec3 (base + "position",  lights[i].position);
        shader.setVec3 (base + "color",     lights[i].color);
        shader.setFloat(base + "intensity", lights[i].intensity);
        shader.setFloat(base + "constant",  lights[i].constant);
        shader.setFloat(base + "linear",    lights[i].linear);
        shader.setFloat(base + "quadratic", lights[i].quadratic);
    }
}