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

inline std::vector<BezierSegment> buildNBRCircuit()
{
    const float Y = 0.0f;
    // std::vector<glm::vec3> pts = {
    //     {   0.0f, Y,   0.0f },
    //     {  15.0f, Y,   0.0f },
    //     {  30.0f, Y,   0.0f },
    //     {  45.0f, Y,   0.0f },
    //     {  60.0f, Y,   0.0f },
    //     {  70.0f, Y,   3.0f },
    //     {  79.0f, Y,   9.0f },
    //     {  86.0f, Y,  18.0f },
    //     {  91.0f, Y,  28.0f },
    //     {  94.0f, Y,  38.0f },
    //     {  95.0f, Y,  49.0f },
    //     {  93.0f, Y,  59.0f },
    //     {  88.0f, Y,  68.0f },
    //     {  81.0f, Y,  75.0f },
    //     {  72.0f, Y,  80.0f },
    //     {  58.0f, Y,  83.0f },
    //     {  44.0f, Y,  84.0f },
    //     {  30.0f, Y,  83.0f },
    //     {  18.0f, Y,  79.0f },
    //     {   8.0f, Y,  72.0f },
    //     {   0.0f, Y,  63.0f },
    //     {  -7.0f, Y,  53.0f },
    //     { -11.0f, Y,  43.0f },
    //     { -12.0f, Y,  32.0f },
    //     { -10.0f, Y,  22.0f },
    //     {  -5.0f, Y,  13.0f },
    //     {   0.0f, Y,   0.0f },
    // };

    std::vector<glm::vec3> pts = {
        {0.0f, -1.5, 0.0f},
        {-95.0f, -2.8, 100.0f},
        {-314.0f, -6.4, 337.0f},
        {-405.0f, -10.7, 415.0f},
        {-455.0f, -12.9, 395.0f},
        {-400.0f, -9.45, 290.0f},
        {-400.0f, -8.3, 250.0f},
        {-420.0f, -8.1, 225.0f},
        {-460.0f, -8.8, 213.0f},
        {-585.0f, -14.4, 230.0f},
        {-610.0f, -15.55, 290.0f},
        //{-570.0f, -14.7, 310.0f},
        {-530.0f, -13.6, 319.0f},
        {-512.0f, -13.25, 370.0f},
        {-545.0f, -13.6, 538.0f},
        {-578.0f, -15.7, 672.0f},
        {-615.0f, -16.45, 697.0f},
        {-650.0f, -18.25, 680.0f},
        {-650.0f, -17, 640.0f},
        {-604.0f, -13, 480.0f},
        {-607.0f, -13, 442.0f},
        {-635.0f, -12.7, 413.0f},
        {-735.0f, -13, 373.0f},
        {-767.0f, -13.95, 337.0f},
        {-769.0f, -18, 287.0f},
        {-730.0f, -21.5, 231.0f}, 
        {-575.0f, -30.7, 35.0f}, 
        {-508.0f, -30.7, -16.0f},  
        // hier punt toevoegen voor hoogte
        {-250.0f, -19.7, -95.0f},  
        {-112.0f, -12.4, -134.0f}, 
        {-93.0f, -11.8, -148.0f},
        //{-88.0f, -, -180.0f}, // miss nog aanpassen
        {-65.0f, -10, -209.0f},
        {100.0f, -0.75, -260.0f},
        {150.0f, 0.3, -258.0f},
        {180.0f, 0.8, -228.0f},
        {173.0f, 1.8, -171.0f},
        {0.0f, -1.5, 0.0f},
    };

    std::vector<BezierSegment> circuit;
    int   n       = (int)pts.size() - 1;
    float tension = 0.35f;

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

inline glm::vec3 cubicBezierTweedeAfgeleide(const glm::vec3& P0, const glm::vec3& C0,
                                              const glm::vec3& C1, const glm::vec3& P1,
                                              float t)
{
    float u = 1.0f - t;
    return 6.0f * (u*(C1 - 2.0f*C0 + P0) + t*(P1 - 2.0f*C1 + C0));
}

inline float sampleCurvature(const std::vector<BezierSegment>& circuit, float globalT)
{
    int   n   = (int)circuit.size();
    int   seg = (int)globalT % n;
    float t   = globalT - (float)(int)globalT;
    const auto& s = circuit[seg];

    glm::vec3 d1 = cubicBezierAfgeleide(s.P0, s.C0, s.C1, s.P1, t);
    glm::vec3 d2 = cubicBezierTweedeAfgeleide(s.P0, s.C0, s.C1, s.P1, t);

    float cross2D = d1.x * d2.z - d1.z * d2.x;
    float speed   = glm::length(d1);
    if (speed < 1e-6f) return 0.0f;

    return cross2D / (speed * speed * speed);
}