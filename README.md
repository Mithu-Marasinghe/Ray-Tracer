## Ray-Tracer

This scene depicts an implementation of a ray tracer with a set of unique features
(Figure 1). It portrays a set of objects on a table, with a mirror reflecting the back of the
tables, all encapsulated within a 6-axis aligned planes. In the back-left of the scene,
there is a reflective plane to showcase the wall behind the camera.

## Features

#### 1. Additional Object Types

a. Single Cone
i. Orange Cone in the middle of the table implemented by solving the
equations given in the lecture notes
b. Cylinder with clearly visible cap
i. Green Cylinder with cap implemented using the method given in
the lecture notes

#### 2. Refraction of light through an object

a. White/Gray sphere in the middle implemented by bending the ray twice
(once when it enters and once when it leaves) with a certain refractive
index

#### 3. Object-space transformations

a. Gray flattened sphere at the back right of the table. This was implemented
by inversing the direction vector according to the transformations and the
normal vectors accordingly.

#### 4. Anti-aliasing

a. Implemented using super-sampling (making 1 pixel into 4 quadrants and
averaging the colour). This difference can be seen with Figure 2 and Figure
3
Figure 2: Without Anti-Aliasing
Figure 3: With Anti-Aliasing

#### 5. Textured non-planar object

a. The Earth on the right side of the table was textured using UV mapping [1].

#### 6. Stochastic sampling (Rough surface reflections)

a. The small gray plane at the right of the table next to the Earth uses
stochastic sampling by recusively tracing with multiple reflected rays.

#### 7. Multiple light sources

a. There are two main light sources that illuminate the scene, located in
front and toward the left and right of the table. The specular reflections
and shadows can be seen earlier in the report (especially noticeable in
the mirror’s reflection).

## Build Commands

In VSCode install the CMake and CMake Tools extensions.

• Extract the file contents out of the folder
• Open the extracted directory in VSCode by using File->Open Folder
• In the CMake tab (found in the main left bar in VSCode), go to Project Status-Configure and press the Select A Kit button (pencil icon). Then select the Linux gcc
or g++ compiler. This should create the necessary files in a build directory.
• Go to Project Outline->lab1 and set the build and launch target to the executable
you want to run. You can do this by right clicking on the file and selecting
• Set As Build Target and Set As Launch/Debug Target.
• Go to Project Status->Launch and press the run button (play icon), this should
compile and run the executable.

## References

[1] Texture acquired from Solar System Scope
(https://www.solarsystemscope.com/textures)
