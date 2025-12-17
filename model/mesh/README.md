# 4D Mesh Type Comparison

Godot 4D provides 3 main mesh types for representing visible 4D geometry:

- **Poly Meshes**: Composed of a hierarchy of geometric structures assembling together to form higher-dimensional structures. Edges are made of vertices, faces are made of edges, and cells are made of faces.
- **Tetra Meshes**: Composed entirely of tetrahedra (4D analog of triangles) referencing vertices.
- **Wire Meshes**: Composed entirely of edges referencing vertices.

Here is table comparing their features (referring to the `Array`\* implementations of each mesh type):

| Feature                   | Poly Meshes | Tetra Meshes | Wire Meshes |
| ------------------------- | ----------- | ------------ | ----------- |
| Fast & Efficient          | ❌          | ✅           | ✅          |
| Good for Authoring        | ✅          | ❌           | ✅          |
| Explicit Edges            | ✅          | ❌           | ✅          |
| Explicit Tetrahedra       | ⚠️          | ✅           | ❌          |
| Tetrahedralizable         | ✅          | ✅           | ❌          |
| Convertible to Poly Mesh  | 🟢          | ⚠️           | ❌          |
| Convertible to Tetra Mesh | ✅          | 🟢           | ❌          |
| Convertible to Wire Mesh  | ✅          | ⚠️           | 🟢          |

Legend: ✅ = yes, ❌ = no, ⚠️ = partial or lossy, 🟢 = same format.

Poly meshes are the ideal format for authoring complex 4D geometry, as they can represent detailed hierarchical geometry, and be converted to other formats as needed. Wire meshes can also be good for authoring geometry, if the desired end result geometry is wireframe, without faces or cells. Tetra meshes are poorly suited for authoring, since working with individual tetrahedra can be tedious and unintuitive for complex shapes.

Poly meshes can be converted to any other format losslessly. Tetra meshes can be converted to poly meshes if cells sharing a start vertex function as a pivot that combines multiple cells into a polytope cell, but the conversion may not be perfect. Tetra meshes can also be converted to wire meshes, but extra edges may be created within a face or cell. Wire meshes are not convertible to other formats, as they lack the necessary information to reconstruct faces or cells.

Poly meshes are tetrahedralizable, meaning they can be rendered as cross-sections just like tetra meshes. In fact, PolyMesh4D inherits TetraMesh4D, allowing for PolyMesh4D to be used as-is by functions that accept a TetraMesh4D or Mesh4D. Poly meshes can represent tetrahedra directly, but they are usually used for more complex shapes, therefore the intended use case of poly meshes does not involve explicit tetrahedra. Tetra meshes directly represent tetrahedra. Wire meshes cannot be tetrahedralized, but they can still be used in the cross-section renderer via its support for rendering wireframes.
