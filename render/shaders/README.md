# Godot 4D Shaders folder

This folder is for general-purpose GLSL shader code which can potentially be reused in multiple shaders.

Do not put shader code here that is specific to a single RenderingEngine4D. Instead, put that code in the folder for that RenderingEngine4D.

Due to the nature of how Godot's shader header system works, it will generate a single GLSL file for each shader, which will include all of the headers. This means that it is impractical to have large files with many functions akin to Vector4D, as they would all be duplicated in every shader. Instead, keep each file small and focused on a single purpose, usually a single function.

Files only ever used as `#include`s in other shaders, not directly read in C++, should not be marked for processing with `GLSL_HEADER`.

If you edit the shaders in this folder, be sure to delete the `*.glsl.gen.h` files for users of these shaders in order to force them to be regenerated.
