# BlockGame
This is my attempt at making a Minecraft clone using OpenGL and C++. My previous project RemEngine featured infinitely generated  terrain, however it used noise incorrectly and used instancing instead of meshing. This engine uses meshing, where all the visible faces of blocks in a chunk are combined into one mesh and then that mesh is drawn. Additionally, I plan on expanding the world generation beyond just being a infinite 1 chunk high grassland.

![Screenshot of the game running on Windows](screenshot.png)

# Current Features
- Infinite terrain (on x and z axis). Currently only 1 chunk on the y axis (i.e. up)
- Basic Entity-Component System (Entities have a list of components that store a reference to their owner allowing interaction between components)
- Free-form controller (doesn't have delta time yet so speed depends on frame-rate)
- Seamless terrain using simplex noise (appears like hilly grassland)
- ImGui is integrated

# Planned Features
- More varied noise **(High priority)**
  - Hills larger than 1 chunk in height
  - Mix flatland and hills
- Biomes **(High priority)**
  - Grassland
  - Desert
  - Forest
  - Snow
- Player controller (with gravity and collision) **(Low priority)**
- Directional light **(Medium Priority)**
- Fog at the end of the player's render distance **(Medium Priority)**
- Switch to using CMake and automatically download dependencies on Windows **(Low Priority)**
- Caves **(Very Low Priority)**
  - Surface entrances

# How to Use This Project
This project uses Visual Studio 2022.

1. Download the dependencies for this project here: https://drive.google.com/file/d/1XS9ZxI5qlj_9hXGBJIeZJifyONhMKnU3/view?usp=sharing<br>

2. Copy the dependencies folder into the folder with the Visual Studio Solution. The copied
folder should be named "Libraries" and it should contain several folders.

3. If they aren't already added as an existing file, add:
    - imgui.cpp
    - imgui_draw.cpp
    - imgui_impl_glfw.cpp, imgui_impl_opengl3.cpp
    - imgui_tables.cpp
    - imgui_widgets.cpp
    - stb_image.c
    - glad.c

4. You should now be able to build and run the project