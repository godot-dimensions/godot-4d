#ifndef GODOT_4D_DEFINES_H
#define GODOT_4D_DEFINES_H

// This file should be included before any other files.

// Uncomment one of these to help IDEs detect the build mode.
// The build system already defines one of these, so keep them
// commented out when committing.
#ifndef GDEXTENSION
//#define GDEXTENSION 1
#endif // GDEXTENSION

#ifndef GODOT_MODULE
//#define GODOT_MODULE 1
#endif // GODOT_MODULE

#if GDEXTENSION
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
// We will have no namespace collisions with Godot, and the namespace does
// not exist when building as a module, so just use the namespace.
using namespace godot;
#elif GODOT_MODULE
#include "core/object/class_db.h"
#include "core/string/ustring.h"
#endif

#ifndef _NO_DISCARD_
#define _NO_DISCARD_ [[nodiscard]]
#endif // _NO_DISCARD_

#endif // GODOT_4D_DEFINES_H
