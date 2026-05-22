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

inline std::vector<BezierSegment> buildNBRCircuit(bool pitstop = false)
{
    const float Y = 0.0f;

    // Standaard            // hoogteverschil cockpit en beziercurve => .9
    std::vector<glm::vec3> pts = {

        // straight 1
        {0.0f, -1.5, 0.0f},
        {-95.0f, -2.8, 100.0f},
        {-314.0f, -6.4, 337.0f},

        // bocht 1
        {-405.0f, -10.7, 415.0f},
        {-436.0f, -12.0, 418.7f},
        {-445.0f, -12.9, 374.0f},

        // bocht 2
        {-400.0f, -9.45, 290.0f},
        {-400.0f, -8.3, 250.0f},
        {-463.0f, -8.8, 206.0f},

        // bocht 3
        {-587.0f, -14.4, 236.0f},
        {-613.0f, -15.55, 275.0f},
        {-594.0f, -15.52, 307.0f},

        // bocht 4
        {-530.0f, -13.6, 319.0f},
        {-512.0f, -13.25, 370.0f},

        // straight 2
        {-545.0f, -13.6, 538.0f},

        // bocht 5
        {-578.0f, -15.7, 672.0f},
        {-614.3f, -16.87, 700.0f},
        {-650.0f, -18.13, 680.0f},
        {-658.8f, -17, 639.9f},

        // straight 3
        {-606.5f, -13, 492.0f},

        // bocht 6
        {-605.0f, -13, 440.2f},
        {-635.0f, -12.7, 413.0f},

        // bocht 7
        {-735.0f, -14, 373.0f},
        {-767.0f, -15.75, 337.0f},
        {-769.0f, -18.5, 287.0f},

        // straight 4
        {-730.0f, -22.5, 231.0f},
        {-657.5f, -28.0, 137.9f},
        {-576.0f, -30.7, 34.3f},

        // bocht 8 (flauw)
        {-501.8f, -30.7, -23.96f},

        // straight 5
        {-330.7f, -24.8, -73.3f},
        {-250.0f, -20.1, -95.0f},
        {-210.5f, -17.4, -106.3f},

        // bocht 9
        {-112.0f, -12.4, -134.0f},
        {-90.0f, -11.7, -151.85f},
        {-88.0f, -11.7, -188.2f},
        {-66.4f, -10.6, -206.8f},

        // straight 6
        {-21.82f, -8.2, -224.85f},
        {35.745f, -3.95, -242.7f},
        {84.6f, -1.2, -256.59f},

        // bocht 10
        {165.0f, 0.7, -252.0f},
        {188.3f, 1.5, -206.0f},
        {173.0f, 1.8, -171.0f},
        //{52.0f, -0.35, -53.5f}, //

        // finish
        {0.0f, -1.5, 0.0f},
    };

    // Pitstop
    std::vector<glm::vec3> pts_ps = {

        {-6.5989f, -1.5, -6.6512f}, // rest place => start pos

        {-188.71f, -3.8, 182.5f},
        {-223.1f, -4.2, 222.6f},

        // straight 1
        {-314.0f, -6.4, 337.0f},

        // bocht 1
        {-405.0f, -10.7, 415.0f},
        {-436.0f, -12.0, 418.7f},
        {-445.0f, -12.9, 374.0f},

        // bocht 2
        {-400.0f, -9.45, 290.0f},
        {-400.0f, -8.3, 250.0f},
        {-463.0f, -8.8, 206.0f},

        // bocht 3
        {-587.0f, -14.4, 236.0f},
        {-613.0f, -15.55, 275.0f},
        {-594.0f, -15.52, 307.0f},

        // bocht 4
        {-530.0f, -13.6, 319.0f},
        {-512.0f, -13.25, 370.0f},

        // straight 2
        {-545.0f, -13.6, 538.0f},

        // bocht 5
        {-578.0f, -15.7, 672.0f},
        {-614.3f, -16.87, 700.0f},
        {-650.0f, -18.13, 680.0f},
        {-658.8f, -17, 639.9f},

        // straight 3
        {-606.5f, -13, 492.0f},

        // bocht 6
        {-605.0f, -13, 440.2f},
        {-635.0f, -12.7, 413.0f},

        // bocht 7
        {-735.0f, -14, 373.0f},
        {-767.0f, -15.75, 337.0f},
        {-769.0f, -18.5, 287.0f},

        // straight 4
        {-730.0f, -22.5, 231.0f},
        {-657.5f, -28.0, 137.9f},
        {-576.0f, -30.7, 34.3f},

        // bocht 8 (flauw)
        {-501.8f, -30.7, -23.96f},

        // straight 5
        {-330.7f, -24.8, -73.3f},
        {-250.0f, -20.1, -95.0f},
        {-210.5f, -17.4, -106.3f},

        // bocht 9
        {-112.0f, -12.4, -134.0f},
        {-90.0f, -11.7, -151.85f},
        {-88.0f, -11.7, -188.2f},
        {-66.4f, -10.6, -206.8f},

        // straight 6
        {-21.82f, -8.2, -224.85f},

        // pit
        {42.5f, -3.95, -232.17f},
        {85.5f, -1, -240.17f},
        {148.5f, 0.2, -247.625f},
        //{173.1f, 1, -229.2f},
        {171.19f, 1.8, -191.0f},

        {115.29f, 1.0, -131.0f},

    };

    if (pitstop)
    {
        pts = pts_ps;
    }
    
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