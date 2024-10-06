#pragma once

#if GDEXTENSION
#include <godot_cpp/classes/control.hpp>

#define GDEXTMOD_GUI_INPUT _gui_input
#elif GODOT_MODULE
#include "scene/gui/control.h"

#define GDEXTMOD_GUI_INPUT gui_input
#endif

class EditorMainScreen4D;

// Grabs input events and sends them to the main screen.
// This sits as a layer on top of most of the viewport, except the rotation gizmo.
class EditorInputSurface4D : public Control {
	GDCLASS(EditorInputSurface4D, Control);

	EditorMainScreen4D *_editor_main_screen = nullptr;

public:
	virtual void GDEXTMOD_GUI_INPUT(const Ref<InputEvent> &p_event) override;

	void set_editor_main_screen(EditorMainScreen4D *p_editor_main_screen);

	EditorInputSurface4D();
};
