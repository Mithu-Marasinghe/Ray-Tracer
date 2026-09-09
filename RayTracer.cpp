/*==================================================================================
 * COSC 363  Computer Graphics
 * Department of Computer Science and Software Engineering, University of Canterbury.
 *
 * A basic ray tracer
 * See Lab07.pdf   for details.
 *===================================================================================
 */
#include <iostream>
#include <cmath>
#include <vector>
#include "TextureBMP.h"
#include <glm/glm.hpp>
#include "Sphere.h"
#include "Plane.h"
#include "SceneObject.h"
#include "Cylinder.h"
#include "Cone.h"
#include "Ray.h"
#include <GL/freeglut.h>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;
const float ROOM_W = 160.0f;
const float ROOM_H = 70.0f;
const float ROOM_D = 500.0f;

TextureBMP texture;
vector<SceneObject *> sceneObjects;

//---The most important function in a ray tracer! ----------------------------------
//   Computes the colour value obtained by tracing a ray and finding its
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0); // Background colour = (0,0,0)
	glm::vec3 lightPos(20, 30, -120);
	glm::vec3 light2Pos(-20, 30, -120);
	glm::vec3 color(0);
	glm::vec3 lightDiffSpec(0);
	glm::vec3 secondLightDiffSpec(0);

	SceneObject *obj;
	SceneObject *shadowObj;

	ray.closestPt(sceneObjects); // Compare the ray with all objects in the scene
	if (ray.index == -1)
		return backgroundCol;	   // no intersection
	obj = sceneObjects[ray.index]; // object on which the closest point of intersection is found

	if (ray.index == 6)
	{
		int checkSize = 5;

		int iz = int((ray.hit.z + 230) / checkSize);
		int ix = int((ray.hit.x + 30) / checkSize);

		bool even = ((ix + iz) % 2 == 0);

		if (even)
			obj->setColor(glm::vec3(0.8, 0.8, 0));
		else
			obj->setColor(glm::vec3(0, 0.8, 0.8));
	}

	if (ray.index == 16)
	{
		glm::vec3 center(20, -18, -150);
		float radius = 7.0f;

		glm::vec3 d = (ray.hit - center) / radius;

		float u = 0.5f + (atan2(d.z, d.x) / (2.0f * M_PI));
		float v = 0.5f + (asin(glm::clamp(d.y, -1.0f, 1.0f)) / M_PI);

		color = texture.getColorAt(u, v);
		obj->setColor(color);
	}

	LightingResult lightingResult = obj->lighting(lightPos, light2Pos, -ray.dir, ray.hit);
	color = lightingResult.color;
	lightDiffSpec = lightingResult.lightDiffSpec;
	secondLightDiffSpec = lightingResult.secondLightDiffSpec;

	glm::vec3 ambient = 0.2f * obj->getColor();
	glm::vec3 diffSpec = color - ambient;

	glm::vec3 lightVec = lightPos - ray.hit;
	Ray shadowRay(ray.hit, lightVec);
	shadowRay.closestPt(sceneObjects);

	glm::vec3 secondLightVec = light2Pos - ray.hit;
	Ray secondShadowRay(ray.hit, secondLightVec);
	secondShadowRay.closestPt(sceneObjects);

	if (shadowRay.index > -1 && shadowRay.dist < glm::length(lightVec))
	{
		// Both hit
		if (secondShadowRay.index > -1 && secondShadowRay.dist < glm::length(secondLightVec))
		{
			shadowObj = sceneObjects[shadowRay.index];
			SceneObject *secondShadowObj = sceneObjects[secondShadowRay.index];
			if (shadowObj == obj)
			{
				color = ambient;
			}
			if (shadowObj->isTransparent() || shadowObj->isRefractive())
			{
				color = ambient + 0.4f * diffSpec;
			}
			else
				color = ambient;
		}
		else
		// Only first light is blocked
		{
			shadowObj = sceneObjects[shadowRay.index];
			if (shadowObj == obj)
			{
				color = ambient + secondLightDiffSpec;
			}
			if (shadowObj->isTransparent() || shadowObj->isRefractive())
			{
				color = ambient + secondLightDiffSpec + 0.2f * lightDiffSpec;
			}
			else
				color = ambient + secondLightDiffSpec;
		}
	}
	else if (secondShadowRay.index > -1 && secondShadowRay.dist < glm::length(secondLightVec))
	{
		// Only second light is blocked
		shadowObj = sceneObjects[secondShadowRay.index];
		if (shadowObj == obj)
		{
			color = ambient + lightDiffSpec;
		}
		if (shadowObj->isTransparent() || shadowObj->isRefractive())
		{
			color = ambient + lightDiffSpec + 0.2f * secondLightDiffSpec;
		}
		else
			color = ambient + lightDiffSpec;
	}

	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		if (ray.index == 18)
		{
			int numSamples = 40;
			float roughness = 0.08f;
			glm::vec3 totalColor(0);
			for (int i = 0; i < numSamples; i++)
			{
				glm::vec3 randomRay = glm::sphericalRand(roughness);
				glm::vec3 glossyDir = glm::normalize(reflectedDir + randomRay);
				Ray glossyRay(ray.hit + 0.001f * glossyDir, glossyDir);
				totalColor += trace(glossyRay, step + 1);
			}

			glm::vec3 averageColor = totalColor / (float)numSamples;
			color += rho * averageColor;
		}
		else
		{
			Ray reflectedRay(ray.hit, reflectedDir);
			glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
			color = color + (rho * reflectedColor);
		}
	}

	if (obj->isTransparent() && step < MAX_STEPS)
	{
		float alpha = obj->getTransparencyCoeff();
		glm::vec3 continuedDir = ray.dir;
		Ray continuedRay(ray.hit, continuedDir);
		continuedRay.closestPt(sceneObjects);
		glm::vec3 nextPos = continuedRay.hit;
		Ray nextRay(nextPos, continuedDir);
		glm::vec3 transparentColor = trace(nextRay, step + 1);

		color = (1 - alpha) * color + alpha * transparentColor;
	}

	if (obj->isRefractive() && step < MAX_STEPS)
	{
		float eta = obj->getRefractiveIndex();
		glm::vec3 n = obj->normal(ray.hit);
		glm::vec3 g = glm::refract(ray.dir, n, 1.0f / eta);
		if (glm::length(g) > 0.0f)
		{
			Ray refractedRay(ray.hit + 0.001f * g, g);
			refractedRay.closestPt(sceneObjects);
			glm::vec3 m = obj->normal(refractedRay.hit);
			glm::vec3 finalDir = glm::refract(g, -m, eta);
			if (glm::length(finalDir) > 0.0f)
			{
				Ray finalRay(refractedRay.hit + 0.001f * finalDir, finalDir);
				glm::vec3 refractedColor = trace(finalRay, step + 1);
				color = color + (1 - obj->getRefractionCoeff()) * refractedColor;
			}
		}
	}

	return color;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;						  // grid point
	float cellX = (XMAX - XMIN) / NUMDIV; // cell width
	float cellY = (YMAX - YMIN) / NUMDIV; // cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS); // Each cell is a tiny quad.

	for (int i = 0; i < NUMDIV; i++)
	{ // Scan every cell of the image plane
		xp = XMIN + i * cellX;
		for (int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j * cellY;
			glm::vec3 col(0);
			float offsets[2] = {0.25f, 0.75f};
			for (int si = 0; si < 2; si++)
			{
				for (int sj = 0; sj < 2; sj++)
				{
					float xa = xp + offsets[si] * cellX;
					float ya = yp + offsets[sj] * cellY;
					Ray ray = Ray(eye, glm::vec3(xa, ya, -EDIST));
					col += trace(ray, 1);
				}
			}
			col /= 4.0f;

			glColor3f(col.r, col.g, col.b);

			// glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST); // direction of the primary ray

			// Ray ray = Ray(eye, dir);

			// glm::vec3 col = trace(ray, 1); // Trace the primary ray and get the colour value
			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp); // Draw each cell with its color value
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
	}

	glEnd();
	glFlush();
}

//---This function initializes the scene -------------------------------------------
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
	texture = TextureBMP("../earth.bmp");

	float left = -ROOM_W / 2;
	float right = ROOM_W / 2;

	float floorY = -ROOM_H / 2;
	float ceilY = ROOM_H / 2;

	float frontZ = 40.0f;

	float backZ = -375.0f;
	float tableLeft = -30.0f;
	float tableRight = 30.0f;
	float tableBottom = -25.0f;

	glClearColor(0, 0, 0, 1);

	// FLOOR
	Plane *floor = new Plane(
		glm::vec3(left, floorY, frontZ),
		glm::vec3(right, floorY, frontZ),
		glm::vec3(right, floorY, backZ),
		glm::vec3(left, floorY, backZ));
	floor->setColor(glm::vec3(0, 0, 1));
	floor->setSpecularity(false);
	sceneObjects.push_back(floor);

	// CEILING
	Plane *ceiling = new Plane(
		glm::vec3(left, ceilY, frontZ),
		glm::vec3(left, ceilY, backZ),
		glm::vec3(right, ceilY, backZ),
		glm::vec3(right, ceilY, frontZ));
	ceiling->setColor(glm::vec3(0, 1, 0));
	ceiling->setSpecularity(false);
	sceneObjects.push_back(ceiling);

	// LEFT WALL
	Plane *leftWall = new Plane(
		glm::vec3(left, floorY, frontZ),
		glm::vec3(left, floorY, backZ),
		glm::vec3(left, ceilY, backZ),
		glm::vec3(left, ceilY, frontZ));
	leftWall->setColor(glm::vec3(0.8, 0.8, 0));
	leftWall->setSpecularity(false);
	sceneObjects.push_back(leftWall);

	// RIGHT WALL
	Plane *rightWall = new Plane(
		glm::vec3(right, floorY, frontZ),
		glm::vec3(right, ceilY, frontZ),
		glm::vec3(right, ceilY, backZ),
		glm::vec3(right, floorY, backZ));
	rightWall->setColor(glm::vec3(0, 0.8, 0.8));
	rightWall->setSpecularity(false);
	sceneObjects.push_back(rightWall);

	// BACK WALL
	Plane *backWall = new Plane(
		glm::vec3(left, floorY, backZ),
		glm::vec3(right, floorY, backZ),
		glm::vec3(right, ceilY, backZ),
		glm::vec3(left, ceilY, backZ));
	backWall->setColor(glm::vec3(0.3, 0.8, 0.5));
	backWall->setSpecularity(false);
	sceneObjects.push_back(backWall);

	// FRONT WALL
	Plane *frontWall = new Plane(
		glm::vec3(left, floorY, frontZ),
		glm::vec3(left, ceilY, frontZ),
		glm::vec3(right, ceilY, frontZ),
		glm::vec3(right, floorY, frontZ));
	frontWall->setColor(glm::vec3(1, 0, 0));
	frontWall->setSpecularity(false);
	sceneObjects.push_back(frontWall);

	// Table
	Plane *plane = new Plane(glm::vec3(tableLeft, tableBottom, -130),
							 glm::vec3(tableRight, tableBottom, -130),
							 glm::vec3(tableRight, tableBottom, -230),
							 glm::vec3(tableLeft, tableBottom, -230));
	plane->setColor(glm::vec3(0.8, 0.8, 0));
	plane->setSpecularity(false);
	sceneObjects.push_back(plane);

	Cylinder *leg1 = new Cylinder(glm::vec3(tableLeft + 5, floorY, -150), 2, 10);
	leg1->setColor(glm::vec3(0.8, 0.8, 0));
	leg1->setSpecularity(false);
	sceneObjects.push_back(leg1);

	Cylinder *leg2 = new Cylinder(glm::vec3(tableRight - 5, floorY, -150), 2, 10);
	leg2->setColor(glm::vec3(0.8, 0.8, 0));
	leg2->setSpecularity(false);
	sceneObjects.push_back(leg2);

	Cylinder *leg3 = new Cylinder(glm::vec3(tableLeft + 5, floorY, -210), 2, 10);
	leg3->setColor(glm::vec3(0.8, 0.8, 0));
	leg3->setSpecularity(false);
	sceneObjects.push_back(leg3);

	Cylinder *leg4 = new Cylinder(glm::vec3(tableRight - 5, floorY, -210), 2, 10);
	leg4->setColor(glm::vec3(0.8, 0.8, 0));
	leg4->setSpecularity(false);
	sceneObjects.push_back(leg4);

	Sphere *transparentSphere = new Sphere(glm::vec3(tableLeft + 10, tableBottom + 7, -155), 7.0);
	transparentSphere->setColor(glm::vec3(0.5, 0, 0));
	transparentSphere->setTransparency(true, 0.5f);
	sceneObjects.push_back(transparentSphere);

	Sphere *refractiveSphere = new Sphere(glm::vec3(tableLeft + 30, tableBottom + 7, -140), 7.0);
	refractiveSphere->setColor(glm::vec3(0.2, 0.2, 0.2));
	refractiveSphere->setRefractivity(true, 0.1f, 1.01f);
	sceneObjects.push_back(refractiveSphere);

	Cylinder *capCylinder = new Cylinder(glm::vec3(tableLeft + 20, tableBottom, -180), 7.0, 7.0);
	capCylinder->setColor(glm::vec3(0, 1, 0.2));
	sceneObjects.push_back(capCylinder);

	Cone *cone = new Cone(glm::vec3(tableLeft + 35, tableBottom, -180), 7.0, 10.0);
	cone->setColor(glm::vec3(1, 0.5, 0.2));
	sceneObjects.push_back(cone);

	Plane *mirror = new Plane(glm::vec3(left + 40, ceilY, backZ + 155),
							  glm::vec3(left + 40, floorY + 30, backZ + 145),
							  glm::vec3(right - 40, floorY + 30, backZ + 145),
							  glm::vec3(right - 40, ceilY, backZ + 155));
	mirror->setColor(glm::vec3(0, 0, 0));
	mirror->setReflectivity(true, 0.9);
	mirror->setSpecularity(false);
	sceneObjects.push_back(mirror);

	Sphere *texturedSphere = new Sphere(glm::vec3(tableLeft + 50, tableBottom + 7, -155), 7.0);
	texturedSphere->setColor(glm::vec3(1, 1, 1));
	texturedSphere->setSpecularity(false);
	sceneObjects.push_back(texturedSphere);

	Sphere *transformedSphere = new Sphere(glm::vec3(0, 0, 0), 7.0);
	transformedSphere->setColor(glm::vec3(0.2, 0.2, 0.2));
	glm::mat4 transform = glm::mat4(1.0);
	transform = glm::translate(transform, glm::vec3(tableLeft + 55, tableBottom + 7, -200));
	transform = glm::scale(transform, glm::vec3(2.5f, 1.0f, 1.0f));
	transformedSphere->setTransform(transform);
	sceneObjects.push_back(transformedSphere);

	Plane *roughPlane = new Plane(glm::vec3(tableRight - 20, tableBottom + 0.01f, -130),
								  glm::vec3(tableRight, tableBottom + 0.01f, -130),
								  glm::vec3(tableRight, tableBottom + 0.01f, -155),
								  glm::vec3(tableRight - 20, tableBottom + 0.01f, -155));
	roughPlane->setColor(glm::vec3(0.2, 0.2, 0.2));
	roughPlane->setReflectivity(true, 0.9f);
	sceneObjects.push_back(roughPlane);

	Plane *reflectionPane = new Plane(glm::vec3(tableLeft, tableBottom, -200),
									  glm::vec3(tableLeft + 10, tableBottom, -200),
									  glm::vec3(tableLeft + 10, tableBottom + 15, -200),
									  glm::vec3(tableLeft, tableBottom + 15, -200));
	reflectionPane->setColor(glm::vec3(0, 0.2, 0.2));
	reflectionPane->setReflectivity(true, 0.9f);
	sceneObjects.push_back(reflectionPane);
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(1300, 1300);
	glutInitWindowPosition(20, 20);
	glutCreateWindow("Raytracing");

	glutDisplayFunc(display);
	initialize();

	glutMainLoop();
	return 0;
}
