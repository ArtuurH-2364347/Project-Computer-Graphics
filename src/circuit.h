#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// -----------------------------------------------------------------------
//  BEZIER STRUCTS & MATH
// -----------------------------------------------------------------------

struct BezierSegment
{
    glm::vec3 P0; // start
    glm::vec3 C0;
    glm::vec3 C1;
    glm::vec3 P1; // end
};

inline glm::vec3 cubicBezier(const glm::vec3& P0, const glm::vec3& C0,
                               const glm::vec3& C1, const glm::vec3& P1,
                               float t)
{
    float u  = 1.0f - t;
    float u2 = u  * u;
    float u3 = u2 * u;
    float t2 = t  * t;
    float t3 = t2 * t;
    return u3*P0 + 3.0f*u2*t*C0 + 3.0f*u*t2*C1 + t3*P1;
}


//Om de draai van de auto te bepalen berekenen we ook de afgeleide van de bezier, 
//die geeft de richting van de auto aan
inline glm::vec3 cubicBezierAfgeleide(const glm::vec3& P0, const glm::vec3& C0,
                                     const glm::vec3& C1, const glm::vec3& P1,
                                     float t)
{
    float u = 1.0f - t;
    return 3.0f * (u*u*(C0-P0) + 2.0f*u*t*(C1-C0) + t*t*(P1-C1));
}

// -----------------------------------------------------------------------
//  CIRCUIT BUILDER
// -----------------------------------------------------------------------

inline std::vector<BezierSegment> buildSpaCircuit()
{
    const float Y = 0.0f;
    std::vector<glm::vec3> pts = {

        {   0.0f, Y,   0.0f },
        {  15.0f, Y,  10.0f },
        {  30.0f, Y,  18.0f },
        {  45.0f, Y,  10.0f },
        {  45.0f, 1.0f, -10.0f },
        {  30.0f, 5.0f, -18.0f },
        {  15.0f, 3.0f, -10.0f },
        {   0.0f, Y,   0.0f },
        { -15.0f, Y, -10.0f },
        { -30.0f, Y, -18.0f },
        { -45.0f, Y, -10.0f },
        { -45.0f, Y,  10.0f },
        { -30.0f, Y,  18.0f },
        { -15.0f, Y,  10.0f },
        {   0.0f, Y,   0.0f },
    };

    std::vector<BezierSegment> circuit;
    int   n       = (int)pts.size() - 1;
    float tension = 0.4f;

    for (int i = 0; i < n; i++) {
        glm::vec3 p0 = pts[(i - 1 + n) % n];
        glm::vec3 p1 = pts[i];
        glm::vec3 p2 = pts[(i + 1) % n];
        glm::vec3 p3 = pts[(i + 2) % n];

        glm::vec3 t1 = (p2 - p0) * tension;
        glm::vec3 t2 = (p3 - p1) * tension;

        glm::vec3 c0 = p1 + (t1 / 3.0f);
        glm::vec3 c1 = p2 - (t2 / 3.0f);

        circuit.push_back({ p1, c0, c1, p2 });
    }

    return circuit;
}

// -----------------------------------------------------------------------
//  CIRCUIT SAMPLING
// -----------------------------------------------------------------------

inline glm::vec3 sampleCircuit(const std::vector<BezierSegment>& circuit, float globalT)
{
    int   n   = (int)circuit.size();
    int   seg = (int)globalT % n;
    float t   = globalT - (float)(int)globalT;
    const auto& s = circuit[seg];
    return cubicBezier(s.P0, s.C0, s.C1, s.P1, t);
}

inline glm::vec3 sampleCircuitAfgeleide(const std::vector<BezierSegment>& circuit, float globalT)
{
    int   n   = (int)circuit.size();
    int   seg = (int)globalT % n;
    float t   = globalT - (float)(int)globalT;
    const auto& s = circuit[seg];
    glm::vec3 tan = cubicBezierAfgeleide(s.P0, s.C0, s.C1, s.P1, t);
    if (glm::length(tan) < 1e-6f) tan = glm::vec3(1, 0, 0);
    return glm::normalize(tan);
}