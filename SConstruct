#!/usr/bin/env python

# This file is for building as a Godot GDExtension.

env = SConscript("addons/4d/src/godot-cpp/SConstruct")

# Add source files.
env.Append(CPPPATH=["./,addons/4d/src/", "math/", "nodes/", "resources/", "mesh", "mesh/tetra", "mesh/wire", "physics/", "physics/resources/"])
sources = Glob("*.cpp") + Glob("addons/4d/src/*.cpp") + Glob("math/*.cpp") + Glob("nodes/*.cpp") + Glob("resources/*.cpp") + Glob("mesh/*.cpp") + Glob("mesh/tetra/*.cpp") + Glob("mesh/wire/*.cpp") + Glob("physics/*.cpp") + Glob("physics/resources/*.cpp")

if env["target"] == "editor":
    env.Append(CPPPATH=["editor/", "editor/off/"])
    sources += Glob("editor/*.cpp") + Glob("editor/off/*.cpp")

env.Append(CPPDEFINES=["GDEXTENSION"])

bin_path = "addons/4d/bin"
extension_name = "godot_4d"
debug_or_release = "release" if env["target"] == "template_release" else "debug"

# Create the library target (e.g. libgodot_4d.linux.debug.x86_64.so).
if env["platform"] == "macos":
    library = env.SharedLibrary(
        "{0}/lib{1}.{2}.{3}.framework/{1}.{2}.{3}".format(
            bin_path,
            extension_name,
            env["platform"],
            debug_or_release,
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "{}/lib{}.{}.{}.{}{}".format(
            bin_path,
            extension_name,
            env["platform"],
            debug_or_release,
            env["arch_suffix"],
            env["SHLIBSUFFIX"],
        ),
        source=sources,
    )

Default(library)
