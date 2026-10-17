#pragma once

// These defines are needed in basically all G4MF files.
// This should be the first file in the G4MF include hierarchy, directly or indirectly.

#include "../../math/geometric_algebra/rotor_4d.h"
#include "../../math/transform_4d.h"
#include "../../math/vector_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include <godot_cpp/core/gdvirtual.gen.inc>
#elif GODOT_MODULE
#include "core/io/resource.h"
#include "core/variant/typed_array.h"
#endif

// Don't list every G4MF class here, only the ones needed for forward declarations.
class G4MFItem4D;
class G4MFNode4D;
class G4MFState4D;
