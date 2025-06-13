#include "editor_4d_icons.h"

#include "editor_4d_icons.gen.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/image.hpp>
#elif GODOT_MODULE
#include "editor/editor_interface.h"
#include "editor/themes/editor_color_map.h"
#include "editor/themes/editor_theme_manager.h"

// SVG loading.
#include "modules/modules_enabled.gen.h" // For svg.
#ifdef MODULE_SVG_ENABLED
#include "modules/svg/image_loader_svg.h"
#endif // MODULE_SVG_ENABLED
#endif

Ref<ImageTexture> _generate_editor_4d_icon(const String &p_icon_name) {
	// Get the index of the icon's source SVG string.
	size_t source_index = 0;
	while (source_index < editor_4d_icons_count) {
		if (String(editor_4d_icons_names[source_index]) == p_icon_name) {
			break;
		}
		source_index++;
	}
	if (source_index == editor_4d_icons_count) {
		WARN_PRINT("Godot 4D: Icon not found: '" + p_icon_name + "'.");
		return Ref<ImageTexture>();
	}
	// Actually generate the icon.
	const double scale = EditorInterface::get_singleton()->get_editor_scale();
	Ref<Image> img;
	img.instantiate();
#ifdef MODULE_SVG_ENABLED
	const HashMap<Color, Color> &color_map = EditorThemeManager::is_dark_theme() ? HashMap<Color, Color>() : EditorColorMap::get_color_conversion_map();
	Error err = ImageLoaderSVG::create_image_from_string(img, editor_4d_icons_sources[source_index], scale, false, color_map);
#else
	// If there's no SVG module this may be because the editor was compiled
	// without SVG, but it's more likely because this is a GDExtension.
	// Regardless, for both cases, try Image's SVG load function.
	Error err = img->load_svg_from_string(editor_4d_icons_sources[source_index], scale);
#endif
	ERR_FAIL_COND_V_MSG(err != OK, Ref<ImageTexture>(), "Godot 4D: Failed generating SVG icon.");
	return ImageTexture::create_from_image(img);
}
