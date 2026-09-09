/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cylinder class
*  This is a subclass of SceneObject, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>

/**
 * Cylinder's intersection method.  The input is a ray.
 */
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir)
{
    float a = (dir.x * dir.x) + (dir.z * dir.z);
    float b = 2 * (dir.x * (p0.x - center.x) + dir.z * (p0.z - center.z));
    float c = ((p0.x - center.x) * (p0.x - center.x)) + ((p0.z - center.z) * (p0.z - center.z)) - (radius * radius);

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        return -1; // No intersection
    }
    float t1 = (-b - sqrt(discriminant)) / (2 * a);
    float t2 = (-b + sqrt(discriminant)) / (2 * a);

    float t = (t1 < t2) ? t1 : t2;
    if (t < 0)
    {
        return -1;
    }

    glm::vec3 hit = p0 + t * dir;
    glm::vec3 secondHit = p0 + ((t == t1) ? t2 : t1) * dir;

    float yMin = center.y;
    float yMax = center.y + height;

    if (hit.y < yMin || hit.y > yMax)
    {
        if (secondHit.y >= yMin && secondHit.y <= yMax)
        {
            t = (center.y + height - p0.y) / dir.y;
        }
        else
            return -1;
    }

    return t;
}

/**
 * Returns the unit normal vector at a given point.
 * Assumption: The input point p lies on the cylinder.
 */
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    float yTop = center.y + height;
    float yBottom = center.y;

    const float EPS = 1e-4;

    if (fabs(p.y - yTop) < EPS)
    {
        return glm::vec3(0, 1, 0);
    }
    else if (fabs(p.y - yBottom) < EPS)
    {
        return glm::vec3(0, -1, 0);
    }

    glm::vec3 n = glm::vec3((p.x - center.x) / radius, 0, (p.z - center.z) / radius);
    return n;
}
