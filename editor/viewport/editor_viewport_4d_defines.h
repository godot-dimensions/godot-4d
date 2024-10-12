#pragma once

#include "../../godot_4d_defines.h"
#include "../../math/basis_4d.h"
#include "../../nodes/node_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/editor_settings.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>

#define GDEXTMOD_GUI_INPUT _gui_input

#define EDITOR_GET(m_name) Variant()
#define EDSCALE 1.0f
#define ERR_THREAD_GUARD
#define TTR(m_text) m_text
#define get_editor_theme_icon get_theme_icon
#elif GODOT_MODULE
#include "core/input/input.h"
#include "editor/editor_settings.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/control.h"

#define GDEXTMOD_GUI_INPUT gui_input

#if GODOT_VERSION < 0x040400
#define set_button_icon set_icon
#endif

#define KEY_A Key::A
#define KEY_D Key::D
#define KEY_E Key::E
#define KEY_F Key::F
#define KEY_Q Key::Q
#define KEY_R Key::R
#define KEY_S Key::S
#define KEY_T Key::T
#define KEY_W Key::W
#define KEY_SHIFT Key::SHIFT

#define MOUSE_BUTTON_LEFT MouseButton::LEFT
#define MOUSE_BUTTON_MASK_MIDDLE MouseButtonMask::MIDDLE
#define MOUSE_BUTTON_MASK_RIGHT MouseButtonMask::RIGHT
#define MOUSE_BUTTON_RIGHT MouseButton::RIGHT
#define MOUSE_BUTTON_WHEEL_UP MouseButton::WHEEL_UP
#define MOUSE_BUTTON_WHEEL_DOWN MouseButton::WHEEL_DOWN
#endif

class EditorCamera4D;
class EditorInputSurface4D;
class EditorMainScreen4D;
class EditorViewportRotation4D;
