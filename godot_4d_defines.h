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
#define GDEXTMOD_GUI_INPUT _gui_input
#define GET_NODE_TYPE(m_parent, m_type, m_path) m_parent->get_node<m_type>(NodePath(m_path))
// Including the namespace helps make GDExtension code more similar to module code.
using namespace godot;
#elif GODOT_MODULE
#include "core/object/class_db.h"
#include "core/string/ustring.h"
#include "core/version.h"
#define GDEXTMOD_GUI_INPUT gui_input
#define GET_NODE_TYPE(m_parent, m_type, m_path) Object::cast_to<m_type>(m_parent->get_node(NodePath(m_path)))

#define MOUSE_BUTTON_LEFT MouseButton::LEFT
#else
#error "Must build as Godot GDExtension or Godot module."
#endif

#ifndef _NO_DISCARD_
#define _NO_DISCARD_ [[nodiscard]]
#endif // _NO_DISCARD_

#endif // GODOT_4D_DEFINES_H
