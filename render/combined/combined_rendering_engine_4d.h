#pragma once

#include "../cross_section/cross_section_rendering_engine_4d.h"
#include "../projected/projected_rendering_engine_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/texture2drd.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/variant/rid.hpp>
#elif GODOT_MODULE
#include "core/templates/rid.h"
#include "scene/gui/texture_rect.h"
#include "scene/main/canvas_layer.h"
#include "scene/resources/3d/world_3d.h"
#include "scene/resources/texture_rd.h"
#include "servers/rendering/storage/render_data.h"
#endif

// Combines CrossSectionRenderingEngine4D and ProjectedRenderingEngine4D: each renders into its
// own off-screen SubViewport (so the projected pass's transparency-normalizing compositor effect
// only ever operates on its own buffer), and the results are composited back onto the real
// viewport as two ordinary alpha-blended CanvasItems.
class CombinedRenderingEngine4D : public RenderingEngine4D {
	GDCLASS(CombinedRenderingEngine4D, RenderingEngine4D);

private:
	Ref<CrossSectionRenderingEngine4D> _cross_section_engine;
	Ref<ProjectedRenderingEngine4D> _projected_engine;

	// _cross_section_viewport is nested inside _projected_viewport, which is nested inside the
	// real viewport, purely so Godot's automatic parent-viewport detection (based on the nearest
	// ancestor Viewport) makes cross-section render before projected, and projected render before
	// the real viewport.
	SubViewport *_cross_section_viewport = nullptr;
	SubViewport *_projected_viewport = nullptr;
	CanvasLayer *_combine_canvas_layer = nullptr;
	TextureRect *_projected_rect = nullptr;

	// The cross-section pass's real depth buffer is a combined depth+stencil format with no
	// Image::Format equivalent and no storage-image usage, so it can't be wrapped by Texture2DRD.
	// Putting the data in a texture is the only way to pass it to the projected sub-viewport.
	// _depth_capture_shader is used to put the data in a separate buffer in a suitable format.
	RID _depth_capture_shader;
	RID _depth_capture_pipeline;
	RID _depth_capture_sampler;
	RID _depth_capture_output_texture;
	Size2i _depth_capture_output_size;
	Ref<Texture2DRD> _cross_section_depth_texture;
	RID _depth_capture_compositor;
	RID _depth_capture_compositor_effect;

	void _ensure_helpers_created();
	void _sync_viewport_sizes();
	void _ensure_depth_capture_output_texture(const Size2i &p_size);
	void _cleanup_render_resources();

protected:
	static void _bind_methods();

public:
	CombinedRenderingEngine4D();
	void set_inner_engines(const Ref<CrossSectionRenderingEngine4D> &p_cross_section_engine, const Ref<ProjectedRenderingEngine4D> &p_projected_engine);
	virtual String get_friendly_name() const override { return "Combined"; }
	virtual void setup_for_viewport() override;
	virtual void cleanup_for_viewport() override;
	virtual void render_frame() override;
	void depth_capture_callback(int64_t p_effect_callback_type, RenderData *p_render_data);

	virtual ~CombinedRenderingEngine4D();
};
