# Godot 4D

Node4D and other 4-dimensional types for making 4D games in Godot.

Have you ever wanted to make 4D games in Godot?

...no? You say you haven't? Ok, then this module isn't for you.

For anyone who does want to make a 4D game, Node4D and other 4D nodes now
exist, just download the fork of Godot that includes this module!

## Nodes

* `Node4D`: Base class for 4D nodes.

## Math

* `Basis4D`: Singleton for working with 4D basis structures.
* `Euler4D`: Singleton for working with 4D Euler angles.
* `Transform4D`: Singleton for working with 4D transformations.
* `Vector4D`: Singleton with extra math functions for Vector4.

All classes are fully documented and have icons. The color is
Yellow to fit with the existing color pattern:
[Cyan is 1D](https://github.com/aaronfranke/godot-1d), Blue is 2D,
[Purple is 2.5D](https://github.com/godotengine/godot-demo-projects/tree/master/misc/2.5d),
Red is 3D, so if we continue adding hue we get to Yellow for 4D.

## Why?

You might be wondering, why? Why spend time making 4D
nodes if 4D games are basically pointless?

Besides, science isn't about why, it's about why not. Why are all of
these nodes 4 dimensional? Why not marry <= 3 dimensional nodes if you
love them so much? In fact, why not invent a special 3D door that
won't hit you on the butt on the way out because you are fired!
Yes, you. Box your 3D stuff. Out the front door.

## Versions

This repo can be compiled either as an engine module or a GDExtension.
Both fully support all features, but only the engine module has
documentation showing up in the editor.

This repo only supports Godot 4.3 and later. There are no plans to
support Godot 3.x, as it is missing critical Variant data types.

## Folder Structure

* `mesh/`: All mesh-related classes including nodes and resources.
* `physics/`: All physics-related classes including nodes and resources.
* `nodes/`: Any nodes that do not fit into the above categories.

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
