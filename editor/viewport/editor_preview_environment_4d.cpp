#include "editor_preview_environment_4d.h"

#include "../../nodes/light/directional_light_4d.h"
#include "../../render/environment/sky/gradient_sky_material_4d.h"
#include "../../render/environment/sky/plain_sky_material_4d.h"
#include "../../render/environment/world_environment_4d.h"

#ifdef TOOLS_ENABLED
#if GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 5
#include "editor/scene_tree_dock.h"
#else
#include "editor/docks/scene_tree_dock.h"
#endif
#include "editor/editor_interface.h"
#endif
#endif // TOOLS_ENABLED

#if GDEXTENSION
#include <godot_cpp/classes/scene_tree.hpp>
#elif GODOT_MODULE
#include "scene/main/scene_tree.h"
#endif

void EditorPreviewEnvironment4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			const Callable node_changed_callable = callable_mp(this, &EditorPreviewEnvironment4D::_on_scene_node_changed);
			if (!get_tree()->is_connected(StringName("node_added"), node_changed_callable)) {
				get_tree()->connect(StringName("node_added"), node_changed_callable);
				get_tree()->connect(StringName("node_removed"), node_changed_callable);
			}
			_update_theme();
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_theme();
		} break;
		case NOTIFICATION_READY: {
			// Children register with RenderingServer4D after this node enters the tree.
			_update_environment();
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED: {
			_update_environment();
		} break;
	}
}

void EditorPreviewEnvironment4D::_on_sun_environment_settings_pressed() {
	const Vector2 button_bottom = _sun_environment_settings_button->get_screen_position() + _sun_environment_settings_button->get_size();
	_sun_environment_popup->reset_size();
	const float popup_half_width = _sun_environment_popup->get_contents_minimum_size().x * 0.5f;
	_sun_environment_popup->set_position(Vector2i(button_bottom - Vector2(popup_half_width, 0.0f)));
	_sun_environment_popup->popup();
	_sun_environment_popup->grab_focus();
}

void EditorPreviewEnvironment4D::_on_toggle_preview_changed(const bool p_toggled_ignored) {
	_update_environment();
	write_to_config_file();
}

void EditorPreviewEnvironment4D::_on_sun_angle_changed(const double p_value_ignored) {
	apply_to_nodes();
	write_to_config_file();
}

void EditorPreviewEnvironment4D::_on_sun_color_changed(const Color &p_color_ignored) {
	apply_to_nodes();
	write_to_config_file();
}

void EditorPreviewEnvironment4D::_on_sun_energy_changed(const double p_value_ignored) {
	apply_to_nodes();
	write_to_config_file();
}

void EditorPreviewEnvironment4D::_on_environment_color_changed(const Color &p_color_ignored) {
	apply_to_nodes();
	write_to_config_file();
}

void EditorPreviewEnvironment4D::_on_environment_energy_multiplier_changed(const double p_value_ignored) {
	apply_to_nodes();
	write_to_config_file();
}

void EditorPreviewEnvironment4D::_on_scene_node_changed(Node *p_node) {
	const bool directional_light_changed = p_node != _preview_sun && Object::cast_to<DirectionalLight4D>(p_node) != nullptr;
	const bool world_environment_changed = p_node != _preview_world_environment && Object::cast_to<WorldEnvironment4D>(p_node) != nullptr;
	if (directional_light_changed || world_environment_changed) {
		// SceneTree's node_added/node_removed signals may run before the node's
		// enter/exit-tree notification has finished registering it with RenderingServer4D.
		callable_mp(this, &EditorPreviewEnvironment4D::_update_environment).call_deferred(false);
	}
}

bool EditorPreviewEnvironment4D::_edited_scene_contains(const StringName &p_type) const {
	Node *edited_scene_root = EditorInterface::get_singleton()->get_edited_scene_root();
	if (edited_scene_root == nullptr) {
		return false;
	}
	if (edited_scene_root->is_class(p_type)) {
		return true;
	}
	return !edited_scene_root->find_children("*", p_type, true, false).is_empty();
}

void EditorPreviewEnvironment4D::_add_sun_to_scene(const bool p_already_added_environment) {
	_sun_environment_popup->hide();
	if (!p_already_added_environment && !_edited_scene_contains("WorldEnvironment4D") && Input::get_singleton()->is_key_pressed(KEY_SHIFT)) {
		_add_environment_to_scene(true);
	}
	Node *edited_scene_root = EditorInterface::get_singleton()->get_edited_scene_root();
#if GODOT_MODULE
	if (edited_scene_root == nullptr) {
		SceneTreeDock::get_singleton()->add_root_node(memnew(Node4D));
		edited_scene_root = EditorInterface::get_singleton()->get_edited_scene_root();
	}
#endif
	ERR_FAIL_NULL_MSG(edited_scene_root, "A scene root is required before adding the preview sun to the scene.");
	ERR_FAIL_NULL(_undo_redo);
	DirectionalLight4D *new_sun = Object::cast_to<DirectionalLight4D>(_preview_sun->duplicate());
	ERR_FAIL_NULL(new_sun);
	new_sun->set_name(StringName("DirectionalLight4D"));
	_undo_redo->create_action(TTR("Add Preview Sun to Scene"));
	_undo_redo->add_do_method(edited_scene_root, StringName("add_child"), new_sun, true);
	_undo_redo->add_do_method(edited_scene_root, StringName("move_child"), new_sun, 0);
	_undo_redo->add_do_method(new_sun, StringName("set_owner"), edited_scene_root);
	_undo_redo->add_undo_method(edited_scene_root, StringName("remove_child"), new_sun);
	_undo_redo->add_do_reference(new_sun);
	_undo_redo->commit_action();
}

void EditorPreviewEnvironment4D::_add_environment_to_scene(const bool p_already_added_sun) {
	_sun_environment_popup->hide();
	if (!p_already_added_sun && !_edited_scene_contains("DirectionalLight4D") && Input::get_singleton()->is_key_pressed(KEY_SHIFT)) {
		_add_sun_to_scene(true);
	}
	Node *edited_scene_root = EditorInterface::get_singleton()->get_edited_scene_root();
#if GODOT_MODULE
	if (edited_scene_root == nullptr) {
		SceneTreeDock::get_singleton()->add_root_node(memnew(Node4D));
		edited_scene_root = EditorInterface::get_singleton()->get_edited_scene_root();
	}
#endif
	ERR_FAIL_NULL_MSG(edited_scene_root, "A scene root is required before adding the preview environment to the scene.");
	ERR_FAIL_NULL(_undo_redo);
	WorldEnvironment4D *new_environment = Object::cast_to<WorldEnvironment4D>(_preview_world_environment->duplicate());
	ERR_FAIL_NULL(new_environment);
	new_environment->set_current(false);
	Ref<SkyMaterial4D> preview_sky_material = _preview_world_environment->get_sky_material();
	if (preview_sky_material.is_valid()) {
		Ref<SkyMaterial4D> duplicated_sky_material = preview_sky_material->duplicate();
		new_environment->set_sky_material(duplicated_sky_material);
	}
	new_environment->set_name(StringName("WorldEnvironment4D"));
	_undo_redo->create_action(TTR("Add Preview Environment to Scene"));
	_undo_redo->add_do_method(edited_scene_root, StringName("add_child"), new_environment, true);
	_undo_redo->add_do_method(edited_scene_root, StringName("move_child"), new_environment, 0);
	_undo_redo->add_do_method(new_environment, StringName("set_owner"), edited_scene_root);
	_undo_redo->add_undo_method(edited_scene_root, StringName("remove_child"), new_environment);
	_undo_redo->add_do_reference(new_environment);
	_undo_redo->commit_action();
}

void EditorPreviewEnvironment4D::_reset_sun() {
	_sun_angle_altitude->set_value_no_signal(60.0);
	_sun_angle_azimuth_zx->set_value_no_signal(30.0);
	_sun_angle_azimuth_zw->set_value_no_signal(0.0);
	_sun_color->set_pick_color(Color(1.0f, 1.0f, 1.0f));
	_sun_energy->set_value_no_signal(1.0);
	apply_to_nodes();
	write_to_config_file();
}

void EditorPreviewEnvironment4D::_reset_environment() {
	_environment_single_color->set_pick_color(Color(0.0f, 0.0f, 0.0f));
	_environment_top_color->set_pick_color(Color(0.385f, 0.454f, 0.55f));
	_environment_horizon_color->set_pick_color(Color(0.6463f, 0.6558f, 0.6708f));
	_environment_bottom_color->set_pick_color(Color(0.2f, 0.169f, 0.133f));
	_environment_energy_multiplier->set_value_no_signal(1.0);
	apply_to_nodes();
	write_to_config_file();
}

void EditorPreviewEnvironment4D::apply_to_nodes() const {
	ERR_FAIL_NULL(_preview_sun);
	ERR_FAIL_NULL(_preview_world_environment);
	Euler4D sun_rotation;
	sun_rotation.yz = Math::deg_to_rad(-_sun_angle_altitude->get_value());
	sun_rotation.zx = Math::deg_to_rad(180.0 - _sun_angle_azimuth_zx->get_value());
	sun_rotation.zw = Math::deg_to_rad(_sun_angle_azimuth_zw->get_value());
	_preview_sun->set_rotation(sun_rotation);
	_preview_sun->set_light_color(_sun_color->get_pick_color());
	_preview_sun->set_light_energy(_sun_energy->get_value());
	if (_rendering_engine_supports_lighting) {
		Ref<GradientSkyMaterial4D> gradient_sky_material = _preview_world_environment->get_sky_material();
		if (gradient_sky_material.is_null()) {
			gradient_sky_material.instantiate();
			_preview_world_environment->set_sky_material(gradient_sky_material);
		}
		gradient_sky_material->set_top_color(_environment_top_color->get_pick_color());
		gradient_sky_material->set_horizon_color(_environment_horizon_color->get_pick_color());
		gradient_sky_material->set_bottom_color(_environment_bottom_color->get_pick_color());
		gradient_sky_material->set_energy_multiplier(_environment_energy_multiplier->get_value());
	} else {
		Ref<PlainSkyMaterial4D> plain_sky_material = _preview_world_environment->get_sky_material();
		if (plain_sky_material.is_null()) {
			plain_sky_material.instantiate();
			_preview_world_environment->set_sky_material(plain_sky_material);
		}
		plain_sky_material->set_color(_environment_single_color->get_pick_color());
	}
}

void EditorPreviewEnvironment4D::_update_environment(const bool p_toggled_ignored) {
	if (_preview_sun != nullptr) {
		const bool scene_has_directional_light = _edited_scene_contains("DirectionalLight4D");
		_toggle_preview_sun_button->set_disabled(scene_has_directional_light);
		_sun_settings_disabled_label->set_visible(scene_has_directional_light);
		_sun_properties_vbox->set_visible(!scene_has_directional_light);
		const bool preview_sunlight_enabled = is_visible_in_tree() && _toggle_preview_sun_button->is_pressed() && !scene_has_directional_light;
		_preview_sun->set_visible(preview_sunlight_enabled);
	}
	if (_preview_world_environment == nullptr) {
		return;
	}
	Node *edited_scene_root = EditorInterface::get_singleton()->get_edited_scene_root();
	TypedArray<Node> scene_world_environments;
	if (edited_scene_root != nullptr) {
		if (Object::cast_to<WorldEnvironment4D>(edited_scene_root) != nullptr) {
			scene_world_environments.append(edited_scene_root);
		}
		scene_world_environments.append_array(edited_scene_root->find_children("*", "WorldEnvironment4D", true, false));
	}
	const bool scene_has_world_environment = !scene_world_environments.is_empty();
	_toggle_preview_environment_button->set_disabled(scene_has_world_environment);
	_environment_settings_disabled_label->set_visible(scene_has_world_environment);
	_environment_properties_vbox->set_visible(!scene_has_world_environment);
	const bool preview_environment_enabled = is_visible_in_tree() && _toggle_preview_environment_button->is_pressed() && !scene_has_world_environment;
	if (preview_environment_enabled) {
		_preview_world_environment->make_current();
	} else {
		_preview_world_environment->clear_current();
		bool scene_environment_is_current = false;
		for (int i = 0; i < scene_world_environments.size(); i++) {
			WorldEnvironment4D *world_environment = Object::cast_to<WorldEnvironment4D>(scene_world_environments[i]);
			CRASH_COND(world_environment == nullptr);
			scene_environment_is_current |= world_environment->is_current();
		}
		if (!scene_environment_is_current && scene_has_world_environment) {
			WorldEnvironment4D *first_world_environment = Object::cast_to<WorldEnvironment4D>(scene_world_environments[0]);
			CRASH_COND(first_world_environment == nullptr);
			first_world_environment->make_current();
		}
	}
}

void EditorPreviewEnvironment4D::_update_theme() {
	// Set icons.
	_toggle_preview_sun_button->set_button_icon(get_editor_theme_icon(StringName("PreviewSun")));
	_toggle_preview_environment_button->set_button_icon(get_editor_theme_icon(StringName("PreviewEnvironment")));
	_sun_environment_settings_button->set_button_icon(get_editor_theme_icon(StringName("GuiTabMenuHl")));
	// Set the minimum height of the color pickers.
	const Size2 min_color_size = Size2(100.0f, 30.0f) * EDSCALE;
	_sun_color->set_custom_minimum_size(min_color_size);
	_environment_single_color->set_custom_minimum_size(min_color_size);
	_environment_top_color->set_custom_minimum_size(min_color_size);
	_environment_horizon_color->set_custom_minimum_size(min_color_size);
	_environment_bottom_color->set_custom_minimum_size(min_color_size);
}

void EditorPreviewEnvironment4D::set_rendering_engine_supports_lighting(const bool p_supported) {
	_rendering_engine_supports_lighting = p_supported;
	_toggle_preview_sun_button->set_visible(p_supported);
	_sun_column_vbox->set_visible(p_supported);
	_sun_environment_separator->set_visible(p_supported);
	_environment_single_color_label->set_visible(!p_supported);
	_environment_single_color->set_visible(!p_supported);
	_environment_lit_sky_properties_vbox->set_visible(p_supported);
	apply_to_nodes();
}

void EditorPreviewEnvironment4D::setup(EditorMainScreen4D *p_editor_main_screen, EditorUndoRedoManager *p_undo_redo, const Ref<ConfigFile> &p_config_file, const String &p_config_file_path) {
	set_name(StringName("EditorPreviewEnvironment4D"));
	_editor_main_screen = p_editor_main_screen;
	_undo_redo = p_undo_redo;
	_4d_editor_config_file = p_config_file;
	_4d_editor_config_file_path = p_config_file_path;

	_toggle_preview_sun_button = memnew(Button);
	_toggle_preview_sun_button->set_toggle_mode(true);
	_toggle_preview_sun_button->set_theme_type_variation("FlatButton");
	_toggle_preview_sun_button->set_tooltip_text(TTR("Toggle preview sunlight.\nIf a DirectionalLight4D node is added to the scene, preview sunlight is disabled."));
	_toggle_preview_sun_button->connect(StringName("toggled"), callable_mp(this, &EditorPreviewEnvironment4D::_on_toggle_preview_changed));
	_toggle_preview_sun_button->set_pressed_no_signal(p_config_file->get_value("preview_environment", "sun_enabled", true));
	add_child(_toggle_preview_sun_button);

	_toggle_preview_environment_button = memnew(Button);
	_toggle_preview_environment_button->set_toggle_mode(true);
	_toggle_preview_environment_button->set_theme_type_variation("FlatButton");
	_toggle_preview_environment_button->set_tooltip_text(TTR("Toggle preview environment.\nIf a WorldEnvironment4D node is added to the scene, preview environment is disabled."));
	_toggle_preview_environment_button->connect(StringName("toggled"), callable_mp(this, &EditorPreviewEnvironment4D::_on_toggle_preview_changed));
	_toggle_preview_environment_button->set_pressed_no_signal(p_config_file->get_value("preview_environment", "environment_enabled", true));
	add_child(_toggle_preview_environment_button);

	_sun_environment_settings_button = memnew(Button);
	_sun_environment_settings_button->set_theme_type_variation("FlatButton");
	_sun_environment_settings_button->set_tooltip_text(TTR("Edit Sun and Environment settings."));
	_sun_environment_settings_button->connect(StringName("pressed"), callable_mp(this, &EditorPreviewEnvironment4D::_on_sun_environment_settings_pressed));
	add_child(_sun_environment_settings_button);
	add_child(memnew(VSeparator));

	// Note: Most of the below code is AI generated, with the main instruction to the AI
	// being to implement a settings popup in the style of Godot's 3D environment settings.

	// Set up the preview Sun and Environment settings popup.
	_sun_environment_popup = memnew(PopupPanel);
	add_child(_sun_environment_popup);
	HBoxContainer *sun_environment_hbox = memnew(HBoxContainer);
	sun_environment_hbox->set_h_size_flags(SIZE_EXPAND_FILL);
	sun_environment_hbox->set_v_size_flags(SIZE_EXPAND_FILL);
	sun_environment_hbox->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	_sun_environment_popup->add_child(sun_environment_hbox);
	constexpr float MIN_COLUMN_HEIGHT = 130.0f;

	_sun_column_vbox = memnew(VBoxContainer);
	_sun_column_vbox->set_custom_minimum_size(Size2(240.0f, MIN_COLUMN_HEIGHT) * EDSCALE);
	_sun_column_vbox->set_v_size_flags(SIZE_EXPAND_FILL);
	sun_environment_hbox->add_child(_sun_column_vbox);
	Label *sun_title = memnew(Label);
	sun_title->set_theme_type_variation("HeaderMedium");
	sun_title->set_text(TTR("Preview Sun"));
	sun_title->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	_sun_column_vbox->add_child(sun_title);

	_sun_settings_disabled_label = memnew(Label);
	// Translations may alter the placement of line breaks, keeping the lines at a limited width.
	_sun_settings_disabled_label->set_text(TTR("Disabled because a\nDirectionalLight4D\nnode exists in the scene."));
	_sun_settings_disabled_label->set_autowrap_mode(TextServer::AUTOWRAP_WORD);
	_sun_settings_disabled_label->set_h_size_flags(SIZE_EXPAND_FILL);
	_sun_settings_disabled_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	_sun_settings_disabled_label->set_visible(false);
	_sun_column_vbox->add_child(_sun_settings_disabled_label);

	_sun_properties_vbox = memnew(VBoxContainer);
	_sun_properties_vbox->set_h_size_flags(SIZE_EXPAND_FILL);
	_sun_column_vbox->add_child(_sun_properties_vbox);

	Label *sun_altitude_label = memnew(Label);
	sun_altitude_label->set_text(TTR("Angular Altitude"));
	_sun_properties_vbox->add_child(sun_altitude_label);
	_sun_angle_altitude = memnew(EditorSpinSlider);
	_sun_angle_altitude->set_suffix(U"\u00B0");
	_sun_angle_altitude->set_min(-90.0);
	_sun_angle_altitude->set_max(90.0);
	_sun_angle_altitude->set_step(0.1);
	_sun_angle_altitude->set_value(p_config_file->get_value("preview_environment", "sun_angle_altitude", 60.0));
	_sun_angle_altitude->connect(StringName("value_changed"), callable_mp(this, &EditorPreviewEnvironment4D::_on_sun_angle_changed));
	_sun_properties_vbox->add_child(_sun_angle_altitude);

	HBoxContainer *sun_azimuth_hbox = memnew(HBoxContainer);
	sun_azimuth_hbox->set_h_size_flags(SIZE_EXPAND_FILL);
	sun_azimuth_hbox->add_theme_constant_override("separation", 10);
	_sun_properties_vbox->add_child(sun_azimuth_hbox);
	VBoxContainer *sun_azimuth_zx_vbox = memnew(VBoxContainer);
	sun_azimuth_zx_vbox->set_h_size_flags(SIZE_EXPAND_FILL);
	sun_azimuth_hbox->add_child(sun_azimuth_zx_vbox);
	Label *sun_azimuth_zx_label = memnew(Label);
	sun_azimuth_zx_label->set_text(TTR("ZX Azimuth"));
	sun_azimuth_zx_vbox->add_child(sun_azimuth_zx_label);
	_sun_angle_azimuth_zx = memnew(EditorSpinSlider);
	_sun_angle_azimuth_zx->set_suffix(U"\u00B0");
	_sun_angle_azimuth_zx->set_min(-180.0);
	_sun_angle_azimuth_zx->set_max(180.0);
	_sun_angle_azimuth_zx->set_step(0.1);
	_sun_angle_azimuth_zx->set_allow_greater(true);
	_sun_angle_azimuth_zx->set_allow_lesser(true);
	_sun_angle_azimuth_zx->set_value(p_config_file->get_value("preview_environment", "sun_angle_azimuth_zx", 30.0));
	_sun_angle_azimuth_zx->connect(StringName("value_changed"), callable_mp(this, &EditorPreviewEnvironment4D::_on_sun_angle_changed));
	sun_azimuth_zx_vbox->add_child(_sun_angle_azimuth_zx);
	VBoxContainer *sun_azimuth_zw_vbox = memnew(VBoxContainer);
	sun_azimuth_zw_vbox->set_h_size_flags(SIZE_EXPAND_FILL);
	sun_azimuth_hbox->add_child(sun_azimuth_zw_vbox);
	Label *sun_azimuth_zw_label = memnew(Label);
	sun_azimuth_zw_label->set_text(TTR("ZW Azimuth"));
	sun_azimuth_zw_vbox->add_child(sun_azimuth_zw_label);
	_sun_angle_azimuth_zw = memnew(EditorSpinSlider);
	_sun_angle_azimuth_zw->set_suffix(U"\u00B0");
	_sun_angle_azimuth_zw->set_min(-180.0);
	_sun_angle_azimuth_zw->set_max(180.0);
	_sun_angle_azimuth_zw->set_step(0.1);
	_sun_angle_azimuth_zw->set_allow_greater(true);
	_sun_angle_azimuth_zw->set_allow_lesser(true);
	_sun_angle_azimuth_zw->set_value(p_config_file->get_value("preview_environment", "sun_angle_azimuth_zw", 0.0)); // Default the sun to a default-slice-aligned angle.
	_sun_angle_azimuth_zw->connect(StringName("value_changed"), callable_mp(this, &EditorPreviewEnvironment4D::_on_sun_angle_changed));
	sun_azimuth_zw_vbox->add_child(_sun_angle_azimuth_zw);

	Label *sun_color_label = memnew(Label);
	sun_color_label->set_text(TTR("Sun Color"));
	_sun_properties_vbox->add_child(sun_color_label);
	_sun_color = memnew(ColorPickerButton);
	_sun_color->set_h_size_flags(SIZE_EXPAND_FILL);
	_sun_color->set_edit_alpha(false);
	_sun_color->set_pick_color(p_config_file->get_value("preview_environment", "sun_color", Color(1.0f, 1.0f, 1.0f)));
	_sun_color->connect(StringName("color_changed"), callable_mp(this, &EditorPreviewEnvironment4D::_on_sun_color_changed));
	_sun_properties_vbox->add_child(_sun_color);
	Label *sun_energy_label = memnew(Label);
	sun_energy_label->set_text(TTR("Sun Energy"));
	_sun_properties_vbox->add_child(sun_energy_label);
	_sun_energy = memnew(EditorSpinSlider);
	_sun_energy->set_min(0.1);
	_sun_energy->set_max(2.0);
	_sun_energy->set_step(0.01);
	_sun_energy->set_exp_ratio(true);
	_sun_energy->set_allow_greater(true);
	_sun_energy->set_allow_lesser(true);
	_sun_energy->set_value(p_config_file->get_value("preview_environment", "sun_energy", 1.0));
	_sun_energy->connect(StringName("value_changed"), callable_mp(this, &EditorPreviewEnvironment4D::_on_sun_energy_changed));
	_sun_properties_vbox->add_child(_sun_energy);

	_sun_properties_vbox->add_spacer(false)->set_v_size_flags(SIZE_EXPAND_FILL);
	HBoxContainer *sun_action_hbox = memnew(HBoxContainer);
	_sun_properties_vbox->add_child(sun_action_hbox);
	Button *sun_reset_button = memnew(Button);
	sun_reset_button->set_text(TTR("Reset"));
	sun_reset_button->set_h_size_flags(SIZE_EXPAND_FILL);
	sun_reset_button->connect(StringName("pressed"), callable_mp(this, &EditorPreviewEnvironment4D::_reset_sun));
	sun_action_hbox->add_child(sun_reset_button);
	Button *sun_add_to_scene_button = memnew(Button);
	sun_add_to_scene_button->set_text(TTR("Add Sun to Scene"));
	sun_add_to_scene_button->set_tooltip_text(TTR("Adds a DirectionalLight4D node matching the preview sun settings to the current scene.\nHold Shift while clicking to also add the preview environment to the current scene."));
	sun_add_to_scene_button->set_h_size_flags(SIZE_EXPAND_FILL);
	sun_add_to_scene_button->connect(StringName("pressed"), callable_mp(this, &EditorPreviewEnvironment4D::_add_sun_to_scene).bind(false));
	sun_action_hbox->add_child(sun_add_to_scene_button);

	_preview_sun = memnew(DirectionalLight4D);
	_preview_sun->set_name(StringName("PreviewSun4D"));
	_preview_sun->set_visible(false);
	add_child(_preview_sun);

	_preview_world_environment = memnew(WorldEnvironment4D);
	_preview_world_environment->set_name(StringName("PreviewWorldEnvironment4D"));
	// Match the 3D editor preview environment's default tonemapping.
	_preview_world_environment->set_tonemapper(WorldEnvironment4D::TONE_MAPPER_FILMIC);
	add_child(_preview_world_environment);

	_sun_environment_separator = memnew(VSeparator);
	_sun_environment_separator->set_custom_minimum_size(Size2(10.0f, MIN_COLUMN_HEIGHT) * EDSCALE);
	_sun_environment_separator->set_v_size_flags(SIZE_EXPAND_FILL);
	sun_environment_hbox->add_child(_sun_environment_separator);

	_environment_column_vbox = memnew(VBoxContainer);
	_environment_column_vbox->set_v_size_flags(SIZE_EXPAND_FILL);
	_environment_column_vbox->set_custom_minimum_size(Size2(200.0f, MIN_COLUMN_HEIGHT) * EDSCALE);
	sun_environment_hbox->add_child(_environment_column_vbox);
	Label *environment_title = memnew(Label);
	environment_title->set_theme_type_variation("HeaderMedium");
	environment_title->set_text(TTR("Preview Environment"));
	environment_title->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	_environment_column_vbox->add_child(environment_title);

	_environment_settings_disabled_label = memnew(Label);
	// Translations may alter the placement of line breaks, keeping the lines at a limited width.
	_environment_settings_disabled_label->set_text(TTR("Disabled because a\nWorldEnvironment4D\nnode exists in the scene."));
	_environment_settings_disabled_label->set_autowrap_mode(TextServer::AUTOWRAP_WORD);
	_environment_settings_disabled_label->set_h_size_flags(SIZE_EXPAND_FILL);
	_environment_settings_disabled_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	_environment_settings_disabled_label->set_visible(false);
	_environment_column_vbox->add_child(_environment_settings_disabled_label);

	_environment_properties_vbox = memnew(VBoxContainer);
	_environment_properties_vbox->set_h_size_flags(SIZE_EXPAND_FILL);
	_environment_column_vbox->add_child(_environment_properties_vbox);

	_environment_single_color_label = memnew(Label);
	_environment_single_color_label->set_text(TTR("Single Color"));
	_environment_properties_vbox->add_child(_environment_single_color_label);
	_environment_single_color = memnew(ColorPickerButton);
	_environment_single_color->set_h_size_flags(SIZE_EXPAND_FILL);
	_environment_single_color->set_edit_alpha(false);
	_environment_single_color->set_pick_color(p_config_file->get_value("preview_environment", "single_color", Color(0.0f, 0.0f, 0.0f)));
	_environment_single_color->connect(StringName("color_changed"), callable_mp(this, &EditorPreviewEnvironment4D::_on_environment_color_changed));
	_environment_properties_vbox->add_child(_environment_single_color);

	_environment_lit_sky_properties_vbox = memnew(VBoxContainer);
	_environment_lit_sky_properties_vbox->set_h_size_flags(SIZE_EXPAND_FILL);
	_environment_properties_vbox->add_child(_environment_lit_sky_properties_vbox);

	// Keep these colors in sync with `gradient_sky_shader.glsl`,
	// `GradientSkyMaterial4D`, and Godot's 3D `ProceduralSkyMaterial`.
	Label *environment_top_color_label = memnew(Label);
	environment_top_color_label->set_text(TTR("Top Color"));
	_environment_lit_sky_properties_vbox->add_child(environment_top_color_label);
	_environment_top_color = memnew(ColorPickerButton);
	_environment_top_color->set_h_size_flags(SIZE_EXPAND_FILL);
	_environment_top_color->set_edit_alpha(false);
	_environment_top_color->set_pick_color(p_config_file->get_value("preview_environment", "top_color", Color(0.385f, 0.454f, 0.55f)));
	_environment_top_color->connect(StringName("color_changed"), callable_mp(this, &EditorPreviewEnvironment4D::_on_environment_color_changed));
	_environment_lit_sky_properties_vbox->add_child(_environment_top_color);
	Label *environment_horizon_color_label = memnew(Label);
	environment_horizon_color_label->set_text(TTR("Horizon Color"));
	_environment_lit_sky_properties_vbox->add_child(environment_horizon_color_label);
	_environment_horizon_color = memnew(ColorPickerButton);
	_environment_horizon_color->set_h_size_flags(SIZE_EXPAND_FILL);
	_environment_horizon_color->set_edit_alpha(false);
	_environment_horizon_color->set_pick_color(p_config_file->get_value("preview_environment", "horizon_color", Color(0.6463f, 0.6558f, 0.6708f)));
	_environment_horizon_color->connect(StringName("color_changed"), callable_mp(this, &EditorPreviewEnvironment4D::_on_environment_color_changed));
	_environment_lit_sky_properties_vbox->add_child(_environment_horizon_color);
	Label *environment_bottom_color_label = memnew(Label);
	environment_bottom_color_label->set_text(TTR("Bottom Color"));
	_environment_lit_sky_properties_vbox->add_child(environment_bottom_color_label);
	_environment_bottom_color = memnew(ColorPickerButton);
	_environment_bottom_color->set_h_size_flags(SIZE_EXPAND_FILL);
	_environment_bottom_color->set_edit_alpha(false);
	_environment_bottom_color->set_pick_color(p_config_file->get_value("preview_environment", "bottom_color", Color(0.2f, 0.169f, 0.133f)));
	_environment_bottom_color->connect(StringName("color_changed"), callable_mp(this, &EditorPreviewEnvironment4D::_on_environment_color_changed));
	_environment_lit_sky_properties_vbox->add_child(_environment_bottom_color);
	Label *environment_energy_multiplier_label = memnew(Label);
	environment_energy_multiplier_label->set_text(TTR("Energy Multiplier"));
	_environment_lit_sky_properties_vbox->add_child(environment_energy_multiplier_label);
	_environment_energy_multiplier = memnew(EditorSpinSlider);
	_environment_energy_multiplier->set_min(0.1);
	_environment_energy_multiplier->set_max(2.0);
	_environment_energy_multiplier->set_step(0.01);
	_environment_energy_multiplier->set_exp_ratio(true);
	_environment_energy_multiplier->set_allow_greater(true);
	_environment_energy_multiplier->set_allow_lesser(true);
	_environment_energy_multiplier->set_value(p_config_file->get_value("preview_environment", "energy_multiplier", 1.0));
	_environment_energy_multiplier->connect(StringName("value_changed"), callable_mp(this, &EditorPreviewEnvironment4D::_on_environment_energy_multiplier_changed));
	_environment_lit_sky_properties_vbox->add_child(_environment_energy_multiplier);

	_environment_properties_vbox->add_spacer(false)->set_v_size_flags(SIZE_EXPAND_FILL);
	HBoxContainer *environment_action_hbox = memnew(HBoxContainer);
	_environment_properties_vbox->add_child(environment_action_hbox);
	Button *environment_reset_button = memnew(Button);
	environment_reset_button->set_text(TTR("Reset"));
	environment_reset_button->set_h_size_flags(SIZE_EXPAND_FILL);
	environment_reset_button->connect(StringName("pressed"), callable_mp(this, &EditorPreviewEnvironment4D::_reset_environment));
	environment_action_hbox->add_child(environment_reset_button);
	Button *environment_add_to_scene_button = memnew(Button);
	environment_add_to_scene_button->set_text(TTR("Add Environment to Scene"));
	environment_add_to_scene_button->set_tooltip_text(TTR("Adds a WorldEnvironment4D node matching the preview environment settings to the current scene.\nHold Shift while clicking to also add the preview sun to the current scene."));
	environment_add_to_scene_button->set_h_size_flags(SIZE_EXPAND_FILL);
	environment_add_to_scene_button->connect(StringName("pressed"), callable_mp(this, &EditorPreviewEnvironment4D::_add_environment_to_scene).bind(false));
	environment_action_hbox->add_child(environment_add_to_scene_button);

	apply_to_nodes();
	_update_environment();
}

void EditorPreviewEnvironment4D::write_to_config_file() const {
	ERR_FAIL_COND(_4d_editor_config_file.is_null());
	if (_4d_editor_config_file->has_section("preview_environment")) {
		_4d_editor_config_file->erase_section("preview_environment");
	}
	if (!_toggle_preview_sun_button->is_pressed()) {
		_4d_editor_config_file->set_value("preview_environment", "sun_enabled", false);
	}
	if (!_toggle_preview_environment_button->is_pressed()) {
		_4d_editor_config_file->set_value("preview_environment", "environment_enabled", false);
	}
	if (!Math::is_equal_approx(_sun_angle_altitude->get_value(), 60.0)) {
		_4d_editor_config_file->set_value("preview_environment", "sun_angle_altitude", _sun_angle_altitude->get_value());
	}
	if (!Math::is_equal_approx(_sun_angle_azimuth_zx->get_value(), 30.0)) {
		_4d_editor_config_file->set_value("preview_environment", "sun_angle_azimuth_zx", _sun_angle_azimuth_zx->get_value());
	}
	if (!Math::is_zero_approx(_sun_angle_azimuth_zw->get_value())) {
		_4d_editor_config_file->set_value("preview_environment", "sun_angle_azimuth_zw", _sun_angle_azimuth_zw->get_value());
	}
	if (!_sun_color->get_pick_color().is_equal_approx(Color(1.0f, 1.0f, 1.0f))) {
		_4d_editor_config_file->set_value("preview_environment", "sun_color", _sun_color->get_pick_color());
	}
	if (!Math::is_equal_approx(_sun_energy->get_value(), 1.0)) {
		_4d_editor_config_file->set_value("preview_environment", "sun_energy", _sun_energy->get_value());
	}
	if (!_environment_single_color->get_pick_color().is_equal_approx(Color(0.0f, 0.0f, 0.0f))) {
		_4d_editor_config_file->set_value("preview_environment", "single_color", _environment_single_color->get_pick_color());
	}
	if (!_environment_top_color->get_pick_color().is_equal_approx(Color(0.385f, 0.454f, 0.55f))) {
		_4d_editor_config_file->set_value("preview_environment", "top_color", _environment_top_color->get_pick_color());
	}
	if (!_environment_horizon_color->get_pick_color().is_equal_approx(Color(0.6463f, 0.6558f, 0.6708f))) {
		_4d_editor_config_file->set_value("preview_environment", "horizon_color", _environment_horizon_color->get_pick_color());
	}
	if (!_environment_bottom_color->get_pick_color().is_equal_approx(Color(0.2f, 0.169f, 0.133f))) {
		_4d_editor_config_file->set_value("preview_environment", "bottom_color", _environment_bottom_color->get_pick_color());
	}
	if (!Math::is_equal_approx(_environment_energy_multiplier->get_value(), 1.0)) {
		_4d_editor_config_file->set_value("preview_environment", "energy_multiplier", _environment_energy_multiplier->get_value());
	}
	_4d_editor_config_file->save(_4d_editor_config_file_path);
}
