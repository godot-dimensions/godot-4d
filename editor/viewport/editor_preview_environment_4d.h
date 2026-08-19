#pragma once

#include "../../nodes/camera_4d.h"
#include "editor_viewport_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/color_picker_button.hpp>
#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/editor_spin_slider.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/popup_panel.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/v_separator.hpp>
#elif GODOT_MODULE
#include "core/io/config_file.h"
#include "editor/gui/editor_spin_slider.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/color_picker.h"
#include "scene/gui/label.h"
#include "scene/gui/popup.h"
#include "scene/gui/separator.h"
#endif

class DirectionalLight4D;
class WorldEnvironment4D;

class EditorPreviewEnvironment4D : public HBoxContainer {
	GDCLASS(EditorPreviewEnvironment4D, HBoxContainer);

	EditorMainScreen4D *_editor_main_screen = nullptr;
	EditorUndoRedoManager *_undo_redo = nullptr;
	Ref<ConfigFile> _4d_editor_config_file;
	String _4d_editor_config_file_path;
	bool _rendering_engine_supports_lighting = false;

	// Nodes in the scene.
	DirectionalLight4D *_preview_sun = nullptr;
	WorldEnvironment4D *_preview_world_environment = nullptr;

	// Buttons in the toolbar.
	Button *_toggle_preview_sun_button = nullptr;
	Button *_toggle_preview_environment_button = nullptr;
	Button *_sun_environment_settings_button = nullptr;

	// The popup panel and its members.
	PopupPanel *_sun_environment_popup = nullptr;
	VSeparator *_sun_environment_separator = nullptr;

	VBoxContainer *_sun_column_vbox = nullptr;
	Label *_sun_settings_disabled_label = nullptr;
	VBoxContainer *_sun_properties_vbox = nullptr;
	EditorSpinSlider *_sun_angle_altitude = nullptr;
	EditorSpinSlider *_sun_angle_azimuth_zx = nullptr;
	EditorSpinSlider *_sun_angle_azimuth_zw = nullptr;
	ColorPickerButton *_sun_color = nullptr;
	EditorSpinSlider *_sun_energy = nullptr;

	VBoxContainer *_environment_column_vbox = nullptr;
	Label *_environment_settings_disabled_label = nullptr;
	VBoxContainer *_environment_properties_vbox = nullptr;
	Label *_environment_single_color_label = nullptr;
	ColorPickerButton *_environment_single_color = nullptr;
	VBoxContainer *_environment_lit_sky_properties_vbox = nullptr;
	ColorPickerButton *_environment_top_color = nullptr;
	ColorPickerButton *_environment_horizon_color = nullptr;
	ColorPickerButton *_environment_bottom_color = nullptr;
	EditorSpinSlider *_environment_energy_multiplier = nullptr;

	void _on_sun_environment_settings_pressed();
	void _on_toggle_preview_changed(const bool p_toggled_ignored);
	void _on_sun_angle_changed(const double p_value_ignored);
	void _on_sun_color_changed(const Color &p_color_ignored);
	void _on_sun_energy_changed(const double p_value_ignored);
	void _on_environment_color_changed(const Color &p_color_ignored);
	void _on_environment_energy_multiplier_changed(const double p_value_ignored);
	void _on_scene_node_changed(Node *p_node);
	void _add_sun_to_scene(const bool p_already_added_environment = false);
	void _add_environment_to_scene(const bool p_already_added_sun = false);
	void _reset_sun();
	void _reset_environment();
	bool _edited_scene_contains(const StringName &p_type) const;
	void _update_environment(const bool p_toggled_ignored = false);
	void _update_theme();

protected:
	void _notification(int p_what);

public:
	void apply_to_nodes() const;
	void set_rendering_engine_supports_lighting(const bool p_supported);
	void setup(EditorMainScreen4D *p_editor_main_screen, EditorUndoRedoManager *p_undo_redo, const Ref<ConfigFile> &p_config_file, const String &p_config_file_path);
	void write_to_config_file() const;
};
