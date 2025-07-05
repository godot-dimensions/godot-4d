#pragma once

#include "editor_viewport_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/confirmation_dialog.hpp>
#include <godot_cpp/classes/editor_inspector.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/menu_button.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#elif GODOT_MODULE
#include "core/io/config_file.h"
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 5
#include "editor/editor_inspector.h"
#else
#include "editor/inspector/editor_inspector.h"
#endif
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/spin_box.h"
#endif

class Marker4D;
class QuadSplitContainer;

// Main class for the 4D editor main screen.
// Has a toolbar at the top and a display of the 4D scene below it.
// Has an EditorMainViewport4D, or up to 4 of them, for displaying the 4D scene.
class EditorMainScreen4D : public Control {
	GDCLASS(EditorMainScreen4D, Control);

	// Defined for readability but not intended to be changed ever.
	// QuadSplitContainer can only handle up to 4 Control children.
	static constexpr int _MAX_VIEWPORTS = 4;

public:
	// Ensure the MODE items are kept in sync with EditorTransformGizmo4D::GizmoMode.
	enum ToolbarButton {
		TOOLBAR_BUTTON_MODE_SELECT, // 0
		TOOLBAR_BUTTON_MODE_MOVE, // 1
		TOOLBAR_BUTTON_MODE_ROTATE, // 2
		TOOLBAR_BUTTON_MODE_SCALE, // 3
		TOOLBAR_BUTTON_MODE_STRETCH, // 4
		TOOLBAR_BUTTON_MODE_MAX, // 5
		TOOLBAR_BUTTON_USE_LOCAL_ROTATION = TOOLBAR_BUTTON_MODE_MAX, // Still 5
		TOOLBAR_BUTTON_MAX
	};

	// Ensure the KEEP items are kept in sync with EditorTransformGizmo4D::KeepMode.
	enum TransformSettingsItem {
		TRANSFORM_SETTING_KEEP_FREEFORM, // 0
		TRANSFORM_SETTING_KEEP_ORTHOGONAL, // 1
		TRANSFORM_SETTING_KEEP_CONFORMAL, // 2
		TRANSFORM_SETTING_KEEP_ORTHONORMAL, // 3
		TRANSFORM_SETTING_KEEP_MAX, // 4
	};

	enum LayoutItem {
		LAYOUT_ITEM_1_VIEWPORT, // 0
		LAYOUT_ITEM_2_VIEWPORTS_TOP_BOTTOM, // 1
		LAYOUT_ITEM_2_VIEWPORTS_LEFT_RIGHT, // 2
		LAYOUT_ITEM_3_VIEWPORTS_TOP_WIDE, // 3
		LAYOUT_ITEM_3_VIEWPORTS_RIGHT_TALL, // 4
		LAYOUT_ITEM_4_VIEWPORTS, // 5
		LAYOUT_ITEM_VIEWPORT_MAX, // 6
		LAYOUT_ITEM_PRESET_GROUND_VIEW = LAYOUT_ITEM_VIEWPORT_MAX, // Still 6
	};

	enum ViewItem {
		VIEW_ITEM_RENDERING_ENGINE,
		VIEW_ITEM_SHOW_ORIGIN_MARKER,
		VIEW_ITEM_CAMERA_SETTINGS,
	};

private:
	Button *_toolbar_buttons[TOOLBAR_BUTTON_MAX] = { nullptr };
	MenuButton *_transform_settings_menu = nullptr;
	MenuButton *_layout_menu = nullptr;
	MenuButton *_view_menu = nullptr;
	PopupMenu *_rendering_engine_menu_popup = nullptr;
	ConfirmationDialog *_camera_settings_dialog = nullptr;
	EditorInspector *_camera_settings_inspector = nullptr;
	EditorCameraSettings4D *_camera_settings = nullptr;
	QuadSplitContainer *_editor_main_viewport_holder = nullptr;
	EditorMainViewport4D *_editor_main_viewports[_MAX_VIEWPORTS] = { nullptr };
	HBoxContainer *_toolbar_hbox = nullptr;
	EditorTransformGizmo4D *_transform_gizmo_4d = nullptr;
	Marker4D *_origin_marker = nullptr;
	Ref<ConfigFile> _4d_editor_config_file;
	String _4d_editor_config_file_path;

	double _information_label_auto_hide_time = 0.0;

	void _apply_4d_editor_settings();
	void _on_button_toggled(const bool p_toggled_on, const int p_option);
	void _on_selection_changed();
	void _on_transform_settings_menu_id_pressed(const int p_id);
	void _on_layout_menu_id_pressed(const int p_id);
	void _on_view_menu_id_pressed(const int p_id);
	void _on_rendering_engine_menu_id_pressed(const int p_id);
	void _update_rendering_engine_menu();
	void _update_theme();

protected:
	static void _bind_methods() {}
	void _notification(int p_what);

public:
	void press_menu_item(const int p_option);
	void set_viewport_layout(const int8_t p_viewport_count, const Side p_dominant_side = SIDE_TOP);

	void setup(EditorUndoRedoManager *p_undo_redo_manager);
	~EditorMainScreen4D();
};
