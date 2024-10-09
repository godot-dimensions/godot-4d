#pragma once

#include "editor_viewport_4d_defines.h"

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
