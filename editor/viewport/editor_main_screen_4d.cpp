#include "editor_main_screen_4d.h"

#include "editor_camera_4d.h"
#include "editor_camera_settings_4d.h"
#include "editor_main_viewport_4d.h"
#include "editor_transform_gizmo_4d.h"
#include "editor_viewport_rotation_4d.h"

#include "../../nodes/marker_4d.h"
#include "../../nodes/quad_split_container.h"
#include "../../render/rendering_server_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_paths.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/v_separator.hpp>

#if GODOT_VERSION < 0x040500
#define get_top_selected_nodes get_transformable_selected_nodes
#endif // GODOT_VERSION
#elif GODOT_MODULE
#include "editor/editor_data.h"
#include "editor/editor_interface.h"
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 5
#include "editor/editor_paths.h"
#else
#include "editor/file_system/editor_paths.h"
#endif
#include "scene/gui/popup_menu.h"
#include "scene/gui/separator.h"

#if VERSION_HEX < 0x040500
#define get_top_selected_nodes get_selected_node_list
#else
#define get_top_selected_nodes get_top_selected_node_list
#endif // VERSION_HEX
#endif

void EditorMainScreen4D::_apply_4d_editor_settings() {
	// This function handles everything except for what's handled in `EditorCameraSettings4D::setup()`.
	_on_layout_menu_id_pressed(_4d_editor_config_file->get_value("viewport", "layout", LAYOUT_ITEM_1_VIEWPORT));
	_on_transform_settings_menu_id_pressed(_4d_editor_config_file->get_value("transform", "keep_mode", TRANSFORM_SETTING_KEEP_FREEFORM));
	const bool transform_use_local_rotation = _4d_editor_config_file->get_value("transform", "use_local_rotation", false);
	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_ROTATION]->set_pressed(transform_use_local_rotation);
	_transform_gizmo_4d->set_use_local_rotation(transform_use_local_rotation);
	const bool viewport_show_origin_marker = _4d_editor_config_file->get_value("viewport", "show_origin_marker", true);
	_view_menu->get_popup()->set_item_checked(VIEW_ITEM_SHOW_ORIGIN_MARKER, viewport_show_origin_marker);
	_origin_marker->set_visible(viewport_show_origin_marker);
	_update_rendering_engine_menu();
}

void EditorMainScreen4D::_on_button_toggled(const bool p_toggled_on, const int p_option) {
	press_menu_item(p_option);
}

void EditorMainScreen4D::_on_selection_changed() {
	EditorSelection *selection = EditorInterface::get_singleton()->get_selection();
	TypedArray<Node> top_selected_nodes;
#if GDEXTENSION
	top_selected_nodes = selection->get_top_selected_nodes();
#elif GODOT_MODULE
	const List<Node *> &top_selected_node_list = selection->get_top_selected_nodes();
	for (Node *node : top_selected_node_list) {
		top_selected_nodes.push_back(node);
	}
#endif
	_transform_gizmo_4d->selected_nodes_changed(top_selected_nodes);
}

void EditorMainScreen4D::_on_transform_settings_menu_id_pressed(const int p_id) {
	PopupMenu *transform_settings_menu_popup = _transform_settings_menu->get_popup();
	if (p_id < TRANSFORM_SETTING_KEEP_MAX) {
		for (int i = 0; i < TRANSFORM_SETTING_KEEP_MAX; i++) {
			transform_settings_menu_popup->set_item_checked(i, i == p_id);
		}
		EditorTransformGizmo4D::KeepMode keep_mode = (EditorTransformGizmo4D::KeepMode)p_id;
		if (keep_mode == EditorTransformGizmo4D::KeepMode::ORTHONORMAL) {
			// Orthonormal can't scale, so if any of the scale buttons are selected, switch to select.
			if (_toolbar_buttons[TOOLBAR_BUTTON_MODE_SCALE]->is_pressed() || _toolbar_buttons[TOOLBAR_BUTTON_MODE_STRETCH]->is_pressed()) {
				press_menu_item(TOOLBAR_BUTTON_MODE_SELECT);
			}
			_toolbar_buttons[TOOLBAR_BUTTON_MODE_SCALE]->set_disabled(true);
			_toolbar_buttons[TOOLBAR_BUTTON_MODE_SCALE]->release_focus();
			_toolbar_buttons[TOOLBAR_BUTTON_MODE_STRETCH]->set_disabled(true);
			_toolbar_buttons[TOOLBAR_BUTTON_MODE_STRETCH]->release_focus();
		} else {
			_toolbar_buttons[TOOLBAR_BUTTON_MODE_SCALE]->set_disabled(false);
			_toolbar_buttons[TOOLBAR_BUTTON_MODE_STRETCH]->set_disabled(false);
		}
		_transform_gizmo_4d->set_keep_mode(keep_mode);
		if (p_id == TRANSFORM_SETTING_KEEP_FREEFORM) {
			if (_4d_editor_config_file->has_section_key("transform", "keep_mode")) {
				_4d_editor_config_file->erase_section_key("transform", "keep_mode");
			}
		} else {
			_4d_editor_config_file->set_value("transform", "keep_mode", (int)keep_mode);
		}
		_4d_editor_config_file->save(_4d_editor_config_file_path);
	}
}

void EditorMainScreen4D::_on_layout_menu_id_pressed(const int p_id) {
	PopupMenu *view_layout_menu_popup = _layout_menu->get_popup();
	if (p_id < LAYOUT_ITEM_VIEWPORT_MAX) {
		for (int i = 0; i < LAYOUT_ITEM_VIEWPORT_MAX; i++) {
			view_layout_menu_popup->set_item_checked(i, i == p_id);
		}
	} else if (p_id == LAYOUT_ITEM_PRESET_GROUND_VIEW) {
		for (int i = 0; i < LAYOUT_ITEM_VIEWPORT_MAX; i++) {
			view_layout_menu_popup->set_item_checked(i, i == LAYOUT_ITEM_4_VIEWPORTS);
		}
	}
	switch (p_id) {
		case LAYOUT_ITEM_1_VIEWPORT: {
			set_viewport_layout(1);
		} break;
		case LAYOUT_ITEM_2_VIEWPORTS_TOP_BOTTOM: {
			set_viewport_layout(2, Side::SIDE_TOP);
		} break;
		case LAYOUT_ITEM_2_VIEWPORTS_LEFT_RIGHT: {
			set_viewport_layout(2, Side::SIDE_RIGHT);
		} break;
		case LAYOUT_ITEM_3_VIEWPORTS_TOP_WIDE: {
			set_viewport_layout(3, Side::SIDE_TOP);
		} break;
		case LAYOUT_ITEM_3_VIEWPORTS_RIGHT_TALL: {
			set_viewport_layout(3, Side::SIDE_RIGHT);
		} break;
		case LAYOUT_ITEM_4_VIEWPORTS: {
			set_viewport_layout(4);
		} break;
		case LAYOUT_ITEM_PRESET_GROUND_VIEW: {
			_on_layout_menu_id_pressed(LAYOUT_ITEM_4_VIEWPORTS);
			_editor_main_viewports[0]->set_ground_view_axis(Vector4::AXIS_Y);
			_editor_main_viewports[1]->set_orthogonal_view_plane(Vector4::AXIS_X, Vector4::AXIS_Z);
			_editor_main_viewports[2]->set_orthogonal_view_plane(Vector4::AXIS_X, Vector4::AXIS_W);
			_editor_main_viewports[3]->set_orthogonal_view_plane(Vector4::AXIS_Z, Vector4::AXIS_W);
		} break;
	}
	_camera_settings->apply_to_cameras();
	if (p_id == LAYOUT_ITEM_1_VIEWPORT) {
		if (_4d_editor_config_file->has_section_key("viewport", "layout")) {
			_4d_editor_config_file->erase_section_key("viewport", "layout");
		}
	} else {
		_4d_editor_config_file->set_value("viewport", "layout", p_id);
	}
	_4d_editor_config_file->save(_4d_editor_config_file_path);
}

void EditorMainScreen4D::_on_view_menu_id_pressed(const int p_id) {
	switch (p_id) {
		case VIEW_ITEM_RENDERING_ENGINE: {
			// See `_on_rendering_engine_menu_id_pressed()`.
		} break;
		case VIEW_ITEM_SHOW_ORIGIN_MARKER: {
			const bool new_is_visible = !_origin_marker->is_visible();
			_view_menu->get_popup()->set_item_checked(p_id, new_is_visible);
			_origin_marker->set_visible(new_is_visible);
			if (new_is_visible) {
				if (_4d_editor_config_file->has_section_key("viewport", "show_origin_marker")) {
					_4d_editor_config_file->erase_section_key("viewport", "show_origin_marker");
				}
			} else {
				_4d_editor_config_file->set_value("viewport", "show_origin_marker", new_is_visible);
			}
			_4d_editor_config_file->save(_4d_editor_config_file_path);
		} break;
		case VIEW_ITEM_CAMERA_SETTINGS: {
			_camera_settings_dialog->popup_centered(Size2(400, 300) * EDSCALE);
		} break;
	}
}

void EditorMainScreen4D::_on_rendering_engine_menu_id_pressed(const int p_id) {
	const String name = _rendering_engine_menu_popup->get_item_text(p_id);
	if (name == TTR("Automatic (default)")) {
		if (_4d_editor_config_file->has_section_key("camera", "rendering_engine")) {
			_4d_editor_config_file->erase_section_key("camera", "rendering_engine");
		}
	} else {
		_4d_editor_config_file->set_value("camera", "rendering_engine", name);
	}
	_update_rendering_engine_menu();
	_4d_editor_config_file->save(_4d_editor_config_file_path);
}

void EditorMainScreen4D::_update_rendering_engine_menu() {
	// First refresh the popup menu's radio button list of rendering engines.
	_rendering_engine_menu_popup->clear();
	_rendering_engine_menu_popup->add_radio_check_item(TTR("Automatic (default)"));
	const PackedStringArray rendering_engine_names = RenderingServer4D::get_singleton()->get_rendering_engine_names();
	for (const String &name : rendering_engine_names) {
		_rendering_engine_menu_popup->add_radio_check_item(name);
	}
	// Then select the rendering engine based on the saved name.
	const String rendering_engine_name = _4d_editor_config_file->get_value("camera", "rendering_engine", String());
	const int64_t index = rendering_engine_names.find(rendering_engine_name);
	if (index == -1) {
		_rendering_engine_menu_popup->set_item_checked(0, true); // Automatic (default).
		_camera_settings->set_rendering_engine("");
	} else {
		_rendering_engine_menu_popup->set_item_checked(index + 1, true); // +1 because of the "Automatic" item.
		_camera_settings->set_rendering_engine(rendering_engine_name);
	}
}

void EditorMainScreen4D::_update_theme() {
	// Set the toolbar offsets. Note that these numbers are designed to match Godot's 2D and 3D editor toolbars.
	_toolbar_hbox->set_offset(Side::SIDE_LEFT, 4.0f * EDSCALE);
	_toolbar_hbox->set_offset(Side::SIDE_RIGHT, -4.0f * EDSCALE);
	_toolbar_hbox->set_offset(Side::SIDE_TOP, 0.0f);
	_toolbar_hbox->set_offset(Side::SIDE_BOTTOM, 29.5f * EDSCALE);
	_editor_main_viewport_holder->set_offset(Side::SIDE_TOP, 33.0f * EDSCALE);
	// Set icons.
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SELECT]->set_button_icon(get_editor_theme_icon(StringName("ToolSelect")));
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_MOVE]->set_button_icon(get_editor_theme_icon(StringName("ToolMove")));
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_ROTATE]->set_button_icon(get_editor_theme_icon(StringName("ToolRotate")));
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SCALE]->set_button_icon(get_editor_theme_icon(StringName("ToolScale")));
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_STRETCH]->set_button_icon(get_editor_theme_icon(StringName("AnimationAutoFitBezier")));
	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_ROTATION]->set_button_icon(get_editor_theme_icon(StringName("Object")));
	// Set view layout popup icons.
	PopupMenu *view_layout_menu_popup = _layout_menu->get_popup();
	view_layout_menu_popup->set_item_icon(LAYOUT_ITEM_1_VIEWPORT, get_editor_theme_icon(StringName("Panels1")));
	view_layout_menu_popup->set_item_icon(LAYOUT_ITEM_2_VIEWPORTS_TOP_BOTTOM, get_editor_theme_icon(StringName("Panels2")));
	view_layout_menu_popup->set_item_icon(LAYOUT_ITEM_2_VIEWPORTS_LEFT_RIGHT, get_editor_theme_icon(StringName("Panels2Alt")));
	view_layout_menu_popup->set_item_icon(LAYOUT_ITEM_3_VIEWPORTS_TOP_WIDE, get_editor_theme_icon(StringName("Panels3")));
	view_layout_menu_popup->set_item_icon(LAYOUT_ITEM_3_VIEWPORTS_RIGHT_TALL, get_editor_theme_icon(StringName("Panels3Alt")));
	view_layout_menu_popup->set_item_icon(LAYOUT_ITEM_4_VIEWPORTS, get_editor_theme_icon(StringName("Panels4")));
}

void EditorMainScreen4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_update_theme();
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_theme();
		} break;
	}
}

void EditorMainScreen4D::press_menu_item(const int p_option) {
	switch (p_option) {
		case TOOLBAR_BUTTON_MODE_SELECT:
		case TOOLBAR_BUTTON_MODE_MOVE:
		case TOOLBAR_BUTTON_MODE_ROTATE:
		case TOOLBAR_BUTTON_MODE_SCALE:
		case TOOLBAR_BUTTON_MODE_STRETCH: {
			for (int i = 0; i < TOOLBAR_BUTTON_MODE_MAX; i++) {
				_toolbar_buttons[i]->set_pressed(i == p_option);
			}
			_transform_gizmo_4d->set_gizmo_mode(EditorTransformGizmo4D::GizmoMode(p_option));
		} break;
		case TOOLBAR_BUTTON_USE_LOCAL_ROTATION: {
			const bool use_local_rotation = _toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_ROTATION]->is_pressed();
			_transform_gizmo_4d->set_use_local_rotation(use_local_rotation);
			if (!use_local_rotation) {
				if (_4d_editor_config_file->has_section_key("transform", "use_local_rotation")) {
					_4d_editor_config_file->erase_section_key("transform", "use_local_rotation");
				}
			} else {
				_4d_editor_config_file->set_value("transform", "use_local_rotation", use_local_rotation);
			}
			_4d_editor_config_file->save(_4d_editor_config_file_path);
		} break;
	}
}

void EditorMainScreen4D::set_viewport_layout(const int8_t p_viewport_count, const Side p_dominant_side) {
	ERR_FAIL_COND(p_viewport_count > _MAX_VIEWPORTS);
	for (int i = 0; i < p_viewport_count; i++) {
		if (_editor_main_viewports[i] == nullptr) {
			_editor_main_viewports[i] = memnew(EditorMainViewport4D);
			_editor_main_viewports[i]->set_name(StringName("EditorMainViewport4D_" + itos(i)));
			_editor_main_viewports[i]->setup(this, _transform_gizmo_4d);
			_editor_main_viewport_holder->add_child(_editor_main_viewports[i]);
		}
	}
	for (int i = p_viewport_count; i < _MAX_VIEWPORTS; i++) {
		if (_editor_main_viewports[i] != nullptr) {
			// Viewports are really computationally expensive, so we should delete them when not in use.
			_editor_main_viewports[i]->queue_free();
			_editor_main_viewports[i] = nullptr;
		}
	}
	_editor_main_viewport_holder->set_layout(p_viewport_count, p_dominant_side);
}

void EditorMainScreen4D::setup(EditorUndoRedoManager *p_undo_redo_manager) {
	// Things that we should do in the constructor but can't in GDExtension
	// due to how GDExtension runs the constructor for each registered class.
	set_name(StringName("EditorMainScreen4D"));
	set_visible(false);
	set_v_size_flags(SIZE_EXPAND_FILL);
	set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);

	// Load 4D editor settings.
	_4d_editor_config_file_path = EditorInterface::get_singleton()->get_editor_paths()->get_project_settings_dir().path_join("4d_editor.cfg");
	_4d_editor_config_file.instantiate();
	_4d_editor_config_file->load(_4d_editor_config_file_path);

	// Set up the toolbar and tool buttons.
	_toolbar_hbox = memnew(HBoxContainer);
	_toolbar_hbox->set_anchors_and_offsets_preset(Control::PRESET_TOP_WIDE);

	add_child(_toolbar_hbox);
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SELECT] = memnew(Button);
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SELECT]->set_toggle_mode(true);
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SELECT]->set_theme_type_variation("FlatButton");
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SELECT]->set_pressed(true);
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SELECT]->set_tooltip_text(TTR("(Q) Select nodes and manipulate their position."));
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SELECT]->connect(StringName("pressed"), callable_mp(this, &EditorMainScreen4D::press_menu_item).bind(TOOLBAR_BUTTON_MODE_SELECT));
	_toolbar_hbox->add_child(_toolbar_buttons[TOOLBAR_BUTTON_MODE_SELECT]);
	_toolbar_hbox->add_child(memnew(VSeparator));

	_toolbar_buttons[TOOLBAR_BUTTON_MODE_MOVE] = memnew(Button);
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_MOVE]->set_toggle_mode(true);
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_MOVE]->set_theme_type_variation("FlatButton");
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_MOVE]->set_tooltip_text(TTR("(W) Move selected node, changing its position."));
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_MOVE]->connect(StringName("pressed"), callable_mp(this, &EditorMainScreen4D::press_menu_item).bind(TOOLBAR_BUTTON_MODE_MOVE));
	_toolbar_hbox->add_child(_toolbar_buttons[TOOLBAR_BUTTON_MODE_MOVE]);

	_toolbar_buttons[TOOLBAR_BUTTON_MODE_ROTATE] = memnew(Button);
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_ROTATE]->set_toggle_mode(true);
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_ROTATE]->set_theme_type_variation("FlatButton");
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_ROTATE]->set_tooltip_text(TTR("(E) Rotate selected node."));
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_ROTATE]->connect(StringName("pressed"), callable_mp(this, &EditorMainScreen4D::press_menu_item).bind(TOOLBAR_BUTTON_MODE_ROTATE));
	_toolbar_hbox->add_child(_toolbar_buttons[TOOLBAR_BUTTON_MODE_ROTATE]);

	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SCALE] = memnew(Button);
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SCALE]->set_toggle_mode(true);
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SCALE]->set_theme_type_variation("FlatButton");
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SCALE]->set_tooltip_text(TTR("(R) Scale selected node."));
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_SCALE]->connect(StringName("pressed"), callable_mp(this, &EditorMainScreen4D::press_menu_item).bind(TOOLBAR_BUTTON_MODE_SCALE));
	_toolbar_hbox->add_child(_toolbar_buttons[TOOLBAR_BUTTON_MODE_SCALE]);

	_toolbar_buttons[TOOLBAR_BUTTON_MODE_STRETCH] = memnew(Button);
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_STRETCH]->set_toggle_mode(true);
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_STRETCH]->set_theme_type_variation("FlatButton");
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_STRETCH]->set_tooltip_text(TTR("Stretch selected node."));
	_toolbar_buttons[TOOLBAR_BUTTON_MODE_STRETCH]->connect(StringName("pressed"), callable_mp(this, &EditorMainScreen4D::press_menu_item).bind(TOOLBAR_BUTTON_MODE_STRETCH));
	_toolbar_hbox->add_child(_toolbar_buttons[TOOLBAR_BUTTON_MODE_STRETCH]);
	_toolbar_hbox->add_child(memnew(VSeparator));

	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_ROTATION] = memnew(Button);
	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_ROTATION]->set_toggle_mode(true);
	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_ROTATION]->set_theme_type_variation("FlatButton");
	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_ROTATION]->set_tooltip_text(TTR("(T) If pressed, use the object's local rotation for the gizmo. Else, transform in global space."));
	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_ROTATION]->connect("toggled", callable_mp(this, &EditorMainScreen4D::_on_button_toggled).bind(TOOLBAR_BUTTON_USE_LOCAL_ROTATION));
	_toolbar_hbox->add_child(_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_ROTATION]);

	// All viewports share one gizmo and origin marker.
	_transform_gizmo_4d = memnew(EditorTransformGizmo4D);
	add_child(_transform_gizmo_4d);

	_origin_marker = memnew(Marker4D);
	_origin_marker->set_name(StringName("OriginMarker4D"));
	_origin_marker->set_marker_extents(1 << 30);
	_origin_marker->set_darken_negative_amount(0.0f);
	_origin_marker->set_subdivisions(40);
	add_child(_origin_marker);

	// Set up the viewports.
	_editor_main_viewport_holder = memnew(QuadSplitContainer);
	_editor_main_viewport_holder->set_name(StringName("EditorMainViewportHolder"));
	_editor_main_viewport_holder->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	_editor_main_viewport_holder->set_offset(Side::SIDE_TOP, 33.0f * EDSCALE);
	add_child(_editor_main_viewport_holder);
	set_viewport_layout(1);

	// Set up the Transform settings menu in the toolbar.
	_transform_settings_menu = memnew(MenuButton);
	_transform_settings_menu->set_flat(false);
	_transform_settings_menu->set_theme_type_variation("FlatMenuButton");
	_transform_settings_menu->set_text(TTR("Transform"));
	_transform_settings_menu->set_tooltip_text(TTR("Change the transform gizmo settings for the 4D editor."));
	_toolbar_hbox->add_child(_transform_settings_menu);

	PopupMenu *transform_settings_menu_popup = _transform_settings_menu->get_popup();
	transform_settings_menu_popup->add_radio_check_item(TTR("Freeform (allow skew/shear)"), TRANSFORM_SETTING_KEEP_FREEFORM);
	transform_settings_menu_popup->add_radio_check_item(TTR("Keep Orthogonal (fix skew/shear)"), TRANSFORM_SETTING_KEEP_ORTHOGONAL);
	transform_settings_menu_popup->add_radio_check_item(TTR("Keep Conformal (uniform scale)"), TRANSFORM_SETTING_KEEP_CONFORMAL);
	transform_settings_menu_popup->add_radio_check_item(TTR("Keep Orthonormal (no scale)"), TRANSFORM_SETTING_KEEP_ORTHONORMAL);
	transform_settings_menu_popup->set_item_checked((int)EditorTransformGizmo4D::KeepMode::FREEFORM, true);
	transform_settings_menu_popup->connect(StringName("id_pressed"), callable_mp(this, &EditorMainScreen4D::_on_transform_settings_menu_id_pressed));

	// Set up the Layout menu in the toolbar.
	_layout_menu = memnew(MenuButton);
	_layout_menu->set_flat(false);
	_layout_menu->set_theme_type_variation("FlatMenuButton");
	_layout_menu->set_text(TTR("Layout"));
	_layout_menu->set_tooltip_text(TTR("Change the layout of the 4D editor viewports."));
	_toolbar_hbox->add_child(_layout_menu);

	PopupMenu *layout_menu_popup = _layout_menu->get_popup();
	layout_menu_popup->add_radio_check_item(TTR("1 Viewport"), LAYOUT_ITEM_1_VIEWPORT);
	layout_menu_popup->add_radio_check_item(TTR("2 Viewports Top/Bottom"), LAYOUT_ITEM_2_VIEWPORTS_TOP_BOTTOM);
	layout_menu_popup->add_radio_check_item(TTR("2 Viewports Left/Right"), LAYOUT_ITEM_2_VIEWPORTS_LEFT_RIGHT);
	layout_menu_popup->add_radio_check_item(TTR("3 Viewports Top Wide"), LAYOUT_ITEM_3_VIEWPORTS_TOP_WIDE);
	layout_menu_popup->add_radio_check_item(TTR("3 Viewports Right Tall"), LAYOUT_ITEM_3_VIEWPORTS_RIGHT_TALL);
	layout_menu_popup->add_radio_check_item(TTR("4 Viewports"), LAYOUT_ITEM_4_VIEWPORTS);
	layout_menu_popup->set_item_checked(LAYOUT_ITEM_1_VIEWPORT, true);
	layout_menu_popup->add_separator();
	layout_menu_popup->add_item(TTR("Preset: Ground View"), LAYOUT_ITEM_PRESET_GROUND_VIEW);
	layout_menu_popup->connect(StringName("id_pressed"), callable_mp(this, &EditorMainScreen4D::_on_layout_menu_id_pressed));

	// Set up the View menu in the toolbar.
	_view_menu = memnew(MenuButton);
	_view_menu->set_flat(false);
	_view_menu->set_theme_type_variation("FlatMenuButton");
	_view_menu->set_text(TTR("View"));
	_view_menu->set_tooltip_text(TTR("Change visual settings such as rendering and camera settings."));
	_toolbar_hbox->add_child(_view_menu);

	PopupMenu *view_menu_popup = _view_menu->get_popup();
	_rendering_engine_menu_popup = memnew(PopupMenu);
	_rendering_engine_menu_popup->set_name("RenderingEngine");
	_rendering_engine_menu_popup->connect(StringName("id_pressed"), callable_mp(this, &EditorMainScreen4D::_on_rendering_engine_menu_id_pressed));
	view_menu_popup->add_child(_rendering_engine_menu_popup);

	view_menu_popup->add_submenu_item(TTR("Rendering Engine"), "RenderingEngine", VIEW_ITEM_RENDERING_ENGINE);
	view_menu_popup->add_check_item(TTR("Show Origin Marker"), VIEW_ITEM_SHOW_ORIGIN_MARKER);
	view_menu_popup->set_item_checked(VIEW_ITEM_SHOW_ORIGIN_MARKER, true);
	view_menu_popup->add_item(TTR("Camera Settings..."), VIEW_ITEM_CAMERA_SETTINGS);
	view_menu_popup->connect(StringName("id_pressed"), callable_mp(this, &EditorMainScreen4D::_on_view_menu_id_pressed));
	_toolbar_hbox->add_child(memnew(VSeparator));

	_camera_settings = memnew(EditorCameraSettings4D);
	_camera_settings->setup(this, _4d_editor_config_file, _4d_editor_config_file_path);
	_camera_settings_dialog = memnew(ConfirmationDialog);
	_camera_settings_dialog->set_name(StringName("CameraSettingsDialog"));
	_camera_settings_dialog->set_title(TTR("Editor Camera Settings"));
	_camera_settings_inspector = memnew(EditorInspector);
	_camera_settings_inspector->set_name(StringName("CameraSettingsInspector"));
	_camera_settings_inspector->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	_camera_settings_inspector->set_custom_minimum_size(Size2(400, 200) * EDSCALE);
#if GODOT_MODULE || (GODOT_VERSION_MAJOR > 4 || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 4))
	// Set up the camera settings dialog as long as this is Godot >= 4.4.
	_camera_settings_inspector->edit(_camera_settings);
	_camera_settings_dialog->add_child(_camera_settings_inspector);
	add_child(_camera_settings_dialog);
#else
	view_menu_popup->set_item_disabled(VIEW_ITEM_CAMERA_SETTINGS, true);
	view_menu_popup->set_item_tooltip(VIEW_ITEM_CAMERA_SETTINGS, TTR("Camera settings requires compiling with a minimum Godot version of 4.4, but this was compiled for Godot " + itos(GODOT_VERSION_MAJOR) + "." + itos(GODOT_VERSION_MINOR) + "."));
#endif

	EditorInterface::get_singleton()->get_selection()->connect(StringName("selection_changed"), callable_mp(this, &EditorMainScreen4D::_on_selection_changed));

	// Set up things with the arguments (not constructor things).
	_transform_gizmo_4d->setup(p_undo_redo_manager);

	_apply_4d_editor_settings();
}

EditorMainScreen4D::~EditorMainScreen4D() {
#if GODOT_MODULE || (GODOT_VERSION_MAJOR > 4 || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 4))
	if (_camera_settings_inspector != nullptr) {
		_camera_settings_inspector->edit(nullptr);
	}
#endif
	if (_camera_settings != nullptr) {
		memdelete(_camera_settings);
	}
}
