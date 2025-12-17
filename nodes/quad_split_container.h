#pragma once

#include "../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/container.hpp>
#include <godot_cpp/classes/input_event.hpp>
#elif GODOT_MODULE
#include "scene/gui/container.h"
#endif

// This class actually has nothing to do with 4D, it's a general UI container
// for laying out up to 4 UI elements with draggable sliders between them.
// This is a generalized version of Godot's Node3DEditorViewportContainer.
// In fact, this code is so similar that here's a copy of the copyright statements:
// Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md).
// Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.
class QuadSplitContainer : public Container {
	GDCLASS(QuadSplitContainer, Container);

	Vector2 _drag_begin_pos = Vector2();
	Vector2 _drag_begin_ratio = Vector2();
	real_t _ratio_h = 0.5f;
	real_t _ratio_v = 0.5f;
	Side _dominant_side = SIDE_TOP;
	int8_t _visible_child_count = 1;
	bool _mouseover = false;
	bool _hovering_v = false;
	bool _hovering_h = false;
	bool _dragging_v = false;
	bool _dragging_h = false;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	virtual void GDEXTMOD_GUI_INPUT(const Ref<InputEvent> &p_event) override;

	void set_layout(const int8_t p_visible_child_count, const Side p_dominant_side = SIDE_TOP);

	QuadSplitContainer();
};
