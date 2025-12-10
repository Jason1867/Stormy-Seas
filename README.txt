Comp4900L - Final Project - README.txt
Project: Stormy Seas

=============================== [ MEMBERS ] ================================
Jason Xu 101220729
David Simonov 101232041

================== [ FILE LIST (with directory structure) ] ===================
- README.txt
- Comp4900L - Final Project Report.pdf <-- (The final report).
- Comp4900L - Final Project.sln <-- (The project solution).
- src\
    -> main.cpp
    -> ofApp.cpp
    -> ofApp.h
- bin\data\
    -> cloud_light.frag
    -> cloud_light.vert
    -> ocean.frag
    -> ocean.vert

========================== [ REQUIRED LIBRARIES ] ==========================
- openFrameworks.

=========================== [ REQUIRED ASSETS ] ===========================
- Visual Studio 2019 or 2022.
- GPU that supports at least OpenGL 3.2 (lower versions might work but would have to set it in the main.cpp file).

======================== [ HOW TO COMPILE AND RUN ] ========================
1. Make sure openframeworks is downloaded and in the appropriate path.
2. Open the solution, then clean it.
3. Build the solution.
4. Check the ofApp.cpp void setup function and modify whatever is under "MODIFIABLE" comments
as you wish.
5. Run the project by pressing the green arrow in VS or F5.

OR

1. Create a project using openFrameworks project generator.
2. Replace the src files of the project with our src files that we provided.
3. Add our shaders in bin\data to your new projects bin\data.
4. Check the ofApp.cpp void setup function and modify whatever is under "MODIFIABLE" comments
as you wish.
5. Build the solution.
6. Run the project by pressing the green arrow in VS or F5.

========================== [ RUN-TIME CONTROLS ] ==========================
[R] - Resets the camera.
[LMB + Hold + Drag] - Move the camera.
[MMB + Down/Up] - Zoom in and out.

================================ [ NOTES ] ================================
- Make sure your graphics drivers are up to date and SUPPORT OpenGL version 3.2.
Most computers have this by default, but older models and Lenovo's with built in Linux
or computers with multiple graphics cards (whether integrated or outdated) can potentially be
using the worse, out of date cards that lacks full 3d rendering capability resulting in
either a outdated OpenGL version or one that simply doesn't work with openframeworks.
Making sure you have the right drivers and function openGL will make sure the shaders
load properly. If the shaders were to fail, the wireframe of the ocean and clouds will be
formed instead as a safety precaution.
Otherwise, everything will work fine.

- Worst case, if it gives you an error such as "openframeworks not found", you can go to
the project generator wherever your openframeworks file is located, and generate a project
replacing the files with the ones we provided. This issue happens when the solution doesn't
know where your openframeworks file is, and thus, can't reference it. 

- To improve performance, reduce N to 512 or 1024 and/or move displacement to a GPU vertex shader.
    -> Can also reduce the cloud count or the range of layers for each cloud under the "build_Cloud()" function.
    -> Also can enable wireframing instead of shaders by going to ofApp.cpp void setup and setting "Enable_WF" to true.
