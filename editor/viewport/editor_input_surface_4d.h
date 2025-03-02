#pragma once

#include "editor_viewport_4d_defines.h"

// Grabs input events and sends them to the main screen.
// This sits as a layer on top of most of the viewport, except the rotation gizmo.
class EditorInputSurface4D : public Control {
	GDCLASS(EditorInputSurface4D, Control);

	EditorMainScreen4D *_editor_main_screen = nullptr;
	EditorMainViewport4D *_editor_main_viewport = nullptr;

public:
	static void _bind_methods() {}
	virtual void GDEXTMOD_GUI_INPUT(const Ref<InputEvent> &p_event) override;

	void setup(EditorMainScreen4D *p_editor_main_screen, EditorMainViewport4D *p_editor_main_viewport);
};
