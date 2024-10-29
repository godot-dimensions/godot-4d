#pragma once

#include "editor_viewport_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/menu_button.hpp>
#elif GODOT_MODULE
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/menu_button.h"
#endif

class QuadSplitContainer;

// Main class for the 4D editor main screen.
// Has a toolbar at the top and a display of the 4D scene below it.
// Has an EditorMainViewport4D, or up to 4 of them, for displaying the 4D scene.
class EditorMainScreen4D : public Control {
	GDCLASS(EditorMainScreen4D, Control);

	// Defined for readability but not intended to be changed ever.
	// QuadSplitContainer can only handle up to 4 Control children.
	static const int _MAX_VIEWPORTS = 4;

public:
	enum ToolbarButton {
		TOOLBAR_BUTTON_SELECT, // 0
		TOOLBAR_BUTTON_MOVE, // 1
		TOOLBAR_BUTTON_ROTATE, // 2
		TOOLBAR_BUTTON_SCALE, // 3
		TOOLBAR_BUTTON_STRETCH, // 4
		TOOLBAR_BUTTON_MODE_MAX, // 5
		TOOLBAR_BUTTON_USE_LOCAL_ROTATION = TOOLBAR_BUTTON_MODE_MAX, // Still 5
		TOOLBAR_BUTTON_MAX
	};

	enum ViewLayoutItem {
		VIEW_LAYOUT_ITEM_1_VIEWPORT, // 0
		VIEW_LAYOUT_ITEM_2_VIEWPORTS_TOP_BOTTOM, // 1
		VIEW_LAYOUT_ITEM_2_VIEWPORTS_LEFT_RIGHT, // 2
		VIEW_LAYOUT_ITEM_3_VIEWPORTS_TOP_WIDE, // 3
		VIEW_LAYOUT_ITEM_3_VIEWPORTS_RIGHT_TALL, // 4
		VIEW_LAYOUT_ITEM_4_VIEWPORTS, // 5
		VIEW_LAYOUT_ITEM_VIEWPORT_MAX, // 6
		VIEW_LAYOUT_ITEM_PRESET_GROUND_VIEW = VIEW_LAYOUT_ITEM_VIEWPORT_MAX, // Still 6
	};

private:
	Button *_toolbar_buttons[TOOLBAR_BUTTON_MAX] = { nullptr };
	MenuButton *_transform_settings_menu = nullptr;
	MenuButton *_view_layout_menu = nullptr;
	QuadSplitContainer *_editor_main_viewport_holder = nullptr;
	EditorMainViewport4D *_editor_main_viewports[_MAX_VIEWPORTS] = { nullptr };
	HBoxContainer *_toolbar_hbox = nullptr;
	EditorTransformGizmo4D *_transform_gizmo_4d = nullptr;

	double _information_label_auto_hide_time = 0.0;

	void _on_button_toggled(const bool p_toggled_on, const int p_option);
	void _on_selection_changed();
	void _on_transform_settings_menu_id_pressed(const int p_id);
	void _on_view_layout_menu_id_pressed(const int p_id);
	void _update_theme();

protected:
	static void _bind_methods() {}
	void _notification(int p_what);

public:
	void press_menu_item(const int p_option);
	void set_viewport_layout(const int8_t p_viewport_count, const Side p_dominant_side = SIDE_TOP);

	void setup(EditorUndoRedoManager *p_undo_redo_manager);
	EditorMainScreen4D();
};
