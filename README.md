# Godot 4D

Node4D and other 4-dimensional types for making 4D games in Godot.

Have you ever wanted to make 4D games in Godot?

...no? You say you haven't? Ok, then this module isn't for you.

For anyone who does want to make a 4D game, Node4D and other 4D nodes now
exist, just download the fork of Godot that includes this module!

## Nodes

-   `Node4D`: Base class for 4D nodes. Has a 4D transform which can be parented.
-   `Camera4D`: 4D camera for projecting 4D scenes onto a 2D screen.
-   `CollisionObject4D` and derived classes: 4D bodies for 4D physics.
    -   These are the same as Godot's 2D and 3D physics bodies but in 4D.
-   `CollisionShape4D`: 4D collision shape for 4D physics (see `Shape4D`).
    -   Physics materials are defined on the shape instead of the body.
-   `MeshInstance4D`: Instance of a 4D mesh in the scene.

## Resources

Note: The indent level does not indicate inheritance.

-   `Mesh4D`: Base class for 4D meshes.
    -   `TetraMesh4D` and derived classes: 4D tetrahedral meshes.
    -   `WireMesh4D` and derived classes: 4D wireframe meshes.
    -   Tetrahedral meshes can also be rendered as wireframe meshes.
    -   Ability to import OFF files as 4D meshes (basic format, like OBJ).
    -   `Material4D`: 4D material for 4D meshes and mesh instances.
        -   `TetraMaterial4D`: Material for 4D tetrahedral meshes.
        -   `WireMaterial4D`: Material for 4D wireframe meshes.
-   `Shape4D` and derived classes: 4D shapes for 4D physics.
    -   Box, sphere, capsule, sphere, cubinder, cylinder, duocylinder, orthoplex.
    -   Helper functions for nearest point, support point, hypervolume, and more.
    -   Can be added to the scene as a `CollisionShape4D` as a child of a `CollisionObject4D`.

## Math

-   `Basis4D`: Singleton for working with 4D basis structures.
-   `Euler4D`: Class for working with 4D Euler angles.
-   `Geometry4D`: Singleton for working with 4D geometry.
-   `Rotor4D`: Geometric algebra rotors for 4D rotations.
-   `Transform4D`: Class for working with 4D transformations.
-   `Vector4D`: Singleton with extra math functions for Vector4.

## Folder Structure

-   `editor/`: All editor-related classes including the 4D viewport main screen tab, and OFF editor import plugins.
-   `math/`: All math-related classes including linear algebra and geometric algebra.
-   `mesh/`: All mesh-related classes including nodes and resources. Two types of meshes are supported, tetrahedral and wireframe.
-   `nodes/`: Any nodes that do not fit into other categories. Node4D, Camera4D, and QuadSplitContainer.
-   `physics/`: All physics-related classes including bodies, shapes, server, and engines. Currently 2 physics engines are included, AxisAlignedBox (AABBs, everything is boxes, does not support angular velocity) and Ghost (objects do not collide, but correctly calculates forces, torque, rotation, etc).
-   `render/`: All rendering-related classes including server and engines.
-   `addons/4d/`: Contains documentation, icons, and files for GDExtension.

All classes are fully documented and have icons. The color is
Yellow to fit with the existing color pattern:
[Cyan is 1D](https://github.com/godot-dimensions/godot-1d), Blue is 2D,
[Purple is 2.5D](https://github.com/godot-dimensions/godot-2pt5d),
Red is 3D, so if we continue adding hue we get to Yellow for 4D.

Every abstract class has virtual methods, allowing users to implement
custom 4D classes in GDScript, GDExtension, etc.

## Why?

You might be wondering, why? Why spend time making 4D
nodes if 4D games are basically pointless? They are confusing and
difficult to understand!

Consider that this module is not just for 4D games. It's an educational
tool for Godot users to learn about 4D geometry, and it's a sandbox
for 4D math nerds to play around with 4D shapes and transformations.

Besides, science isn't about why, it's about why not. Why are all of
these nodes 4 dimensional? Why not marry <= 3 dimensional nodes if you
love them so much? In fact, why not invent a special 3D door that
won't hit you on the butt on the way out because you are fired!
Yes, you. Box your 3D stuff. Out the front door.

## Versions

This repo can be compiled either as an engine module or a GDExtension.
Most editor features only work as a module, so the module is highly recommended.
The GDExtension version is provided on a best-effort basis, and supports
all runtime features but limited editor functionality.

This repo only supports Godot 4.3 and later. There are no plans to support
Godot 3.x, as it is missing critical Variant data types (Vector4 and
Projection). Supporting Godot 4.0 to 4.2 would be possible but difficult.

## License

This repo is free and open source software licensed under The Unlicense.
Credit and attribution is appreciated but not required.

Some parts of some files in this repository are derivative from Godot Engine
and therefore [its MIT license](https://godotengine.org/license) applies.
You must provide credit to Godot Engine by including its LICENSE.
This includes the icons, some of the docs, lots of the math code,
and lots of the code in the nodes.
Considering this repo is only usable in conjunction with Godot anyway,
this will not be a problem because you should already be crediting Godot.
