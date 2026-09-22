# Godot 4D Supported Model Formats

This file compares 4D model formats. A "model" is a file for storing 4D geometry and scene data, and may be composed of many meshes, or some formats just store one mesh. For Godot 4D's _mesh_ formats, please read [mesh/README.md](mesh/README.md).

Here is a brief comparison table:

|                   | Godot Resources    | G4MF             | OFF     | 4DO     | Hox     |
| ----------------- | ------------------ | ---------------- | ------- | ------- | ------- |
| File extension(s) | .res, .tres, .tscn | .g4tf, .g4b      | .off    | .4do    | .hox    |
| Polyhedral mesh   | ✅ PolyMesh4D      | ✅ Geometry data | ✅ Only | ✅ [^1] | ❌      |
| Tetrahedral mesh  | ✅ TetraMesh4D     | ✅ Simplex cells | ❌      | ✅      | ❌      |
| Voxel data        | ❌                 | ❌               | ❌      | ❌      | ✅ Only |
| Multiple meshes   | ✅ Yes             | ✅ Yes           | ❌      | ❌      | ✅      |
| Physics objects   | ✅ Yes             | ✅ Yes           | ❌      | ❌      | ❌      |
| Cameras           | ✅ Yes             | ✅ Yes           | ❌      | ❌      | ❌      |
| Lights            | ✅ Yes             | ✅ Yes           | ❌      | ❌      | ❌      |

Supplemental notes:

[^1]: 4DO can store both general polyhedral cells made of faces, and also cuboid cells made of vertices as a special case.
