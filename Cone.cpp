/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cone class
*  This is a subclass of SceneObject, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cone.h"
#include <math.h>

/**
 * Cone's intersection method.  The input is a ray.
 */
float Cone::intersect(glm::vec3 p0, glm::vec3 dir)
{
    float r = radius / height;
    float a = dir.x * dir.x + dir.z * dir.z - (r * r * dir.y * dir.y);
    float b = 2 * (dir.x * (p0.x - center.x) + dir.z * (p0.z - center.z) + (r * r * dir.y * (height - (p0.y - center.y))));
    float c = (p0.x - center.x) * (p0.x - center.x) + (p0.z - center.z) * (p0.z - center.z) - (r * r * (height - (p0.y - center.y)) * (height - (p0.y - center.y)));

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
 * Assumption: The input point p lies on the cone.
 */
glm::vec3 Cone::normal(glm::vec3 p)
{
    float theta = atan2(radius, height);
    float alpha = atan2((p.x - center.x), (p.z - center.z));
    return glm::vec3(sin(alpha) * cos(theta), sin(theta), cos(alpha) * cos(theta));
}
