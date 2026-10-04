#include "layered_voxel_generator_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/local_vector.hpp>
#elif GODOT_MODULE
#include "core/templates/local_vector.h"
#endif

VoxelMaterial4D LayeredVoxelGenerator4D::get_material(const Vector4i &p_voxel) const {
	for (int64_t i = _layers.size() - 1; i >= 0; i--) {
		const Ref<VoxelGenerator4D> &layer = _layers[i];
		if (layer.is_null()) {
			continue;
		}
		const VoxelMaterial4D material = layer->get_material(p_voxel);
		if (material != VoxelMaterial4D::UNDEFINED) {
			return material;
		}
	}
	return VoxelMaterial4D::UNDEFINED;
}

VoxelEdgeData4D LayeredVoxelGenerator4D::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	Vector4i upper_voxel = p_voxel;
	upper_voxel[p_axis]++;
	// The layers that can affect the edge: scanning from the top, each
	// layer's content claims the part of the edge it covers, from a defined
	// end up to its surface crossing, or all of it when both ends are
	// defined. Once the whole edge is claimed, deeper layers cannot affect
	// the result: not its materials, and not its surface either, since the
	// rules below only let a crossing survive the layers above it when it
	// lies strictly inside the part of the edge they leave unclaimed.
	struct LayerSample4D {
		const VoxelGenerator4D *layer;
		VoxelMaterial4D lower;
		VoxelMaterial4D upper;
		VoxelEdgeData4D edge_data;
		bool has_edge_data = false;
	};
	LocalVector<LayerSample4D> samples; // In top-to-bottom order.
	real_t unclaimed_begin = 0.0f;
	real_t unclaimed_end = 1.0f;
	// The claims reaching the farthest from each end, whose crossings are the
	// surfaces of the combined content on an edge left open at the other end.
	VoxelEdgeData4D begin_claim;
	bool has_begin_claim = false;
	VoxelEdgeData4D end_claim;
	bool has_end_claim = false;
	for (int64_t i = _layers.size() - 1; i >= 0 && unclaimed_begin < unclaimed_end; i--) {
		const Ref<VoxelGenerator4D> &layer = _layers[i];
		if (layer.is_null()) {
			continue;
		}
		LayerSample4D sample;
		sample.layer = layer.ptr();
		sample.lower = layer->get_material(p_voxel);
		sample.upper = layer->get_material(upper_voxel);
		const bool lower_defined = sample.lower != VoxelMaterial4D::UNDEFINED;
		const bool upper_defined = sample.upper != VoxelMaterial4D::UNDEFINED;
		if (!lower_defined && !upper_defined) {
			continue;
		}
		if (lower_defined && upper_defined) {
			unclaimed_begin = unclaimed_end;
		} else {
			sample.edge_data = layer->get_edge_data(p_voxel, p_axis);
			sample.has_edge_data = true;
			if (lower_defined) {
				if (!has_begin_claim || sample.edge_data.position > unclaimed_begin) {
					begin_claim = sample.edge_data;
					has_begin_claim = true;
				}
				unclaimed_begin = MAX(unclaimed_begin, sample.edge_data.position);
			} else {
				if (!has_end_claim || sample.edge_data.position < unclaimed_end) {
					end_claim = sample.edge_data;
					has_end_claim = true;
				}
				unclaimed_end = MIN(unclaimed_end, sample.edge_data.position);
			}
		}
		samples.push_back(sample);
	}
	// Replay those layers bottom-up under the rules that
	// VoxelDataNeighborhood4D::apply_edit uses for the surface data of an
	// edited edge, so that layered content matches the same content applied
	// as a series of edits.
	VoxelEdgeData4D edge_data;
	bool has_edge_data = false;
	VoxelMaterial4D lower_material = VoxelMaterial4D::UNDEFINED;
	VoxelMaterial4D upper_material = VoxelMaterial4D::UNDEFINED;
	for (int64_t i = (int64_t)samples.size() - 1; i >= 0; i--) {
		const LayerSample4D &sample = samples[i];
		const VoxelMaterial4D new_lower = VoxelMaterialUtil4D::overlay_material(sample.lower, lower_material);
		const VoxelMaterial4D new_upper = VoxelMaterialUtil4D::overlay_material(sample.upper, upper_material);
		if (new_lower != VoxelMaterial4D::UNDEFINED && new_upper != VoxelMaterial4D::UNDEFINED) {
			if (new_lower == new_upper) {
				has_edge_data = false;
			} else {
				const VoxelEdgeData4D layer_edge_data = sample.has_edge_data ? sample.edge_data : sample.layer->get_edge_data(p_voxel, p_axis);
				const bool lower_defined = sample.lower != VoxelMaterial4D::UNDEFINED;
				const bool upper_defined = sample.upper != VoxelMaterial4D::UNDEFINED;
				bool keep_old = false;
				if (lower_defined != upper_defined && has_edge_data) {
					// A layer writing a material over itself extends its
					// region, so the combined region's surface is whichever
					// crossing lies farther from the layer-defined end.
					const bool same_material = lower_defined ? sample.lower == lower_material : sample.upper == upper_material;
					if (same_material) {
						keep_old = lower_defined ? edge_data.position > layer_edge_data.position : edge_data.position < layer_edge_data.position;
					}
				}
				if (!keep_old) {
					edge_data = layer_edge_data;
					has_edge_data = true;
				}
			}
		}
		lower_material = new_lower;
		upper_material = new_upper;
	}
	if (lower_material != VoxelMaterial4D::UNDEFINED && upper_material != VoxelMaterial4D::UNDEFINED) {
		// Active edges always get data in the replay above.
		return edge_data;
	}
	// With one end undefined, no layer claims anything from it, so the
	// combined content is claimed entirely from the defined end and its
	// surface is the farthest of those claims.
	if (lower_material != VoxelMaterial4D::UNDEFINED && has_begin_claim) {
		return begin_claim;
	}
	if (upper_material != VoxelMaterial4D::UNDEFINED && has_end_claim) {
		return end_claim;
	}
	// Both ends undefined: no meaningful surface exists to return.
	return edge_data;
}

void LayeredVoxelGenerator4D::set_layers(const Vector<Ref<VoxelGenerator4D>> &p_layers) {
	_layers = p_layers;
	emit_changed();
}

TypedArray<VoxelGenerator4D> LayeredVoxelGenerator4D::get_layers_bind() const {
	TypedArray<VoxelGenerator4D> bind;
	bind.resize(_layers.size());
	for (int64_t i = 0; i < _layers.size(); i++) {
		bind[i] = _layers[i];
	}
	return bind;
}

void LayeredVoxelGenerator4D::set_layers_bind(const TypedArray<VoxelGenerator4D> &p_layers) {
	_layers.resize(p_layers.size());
	for (int64_t i = 0; i < p_layers.size(); i++) {
		_layers.set(i, p_layers[i]);
	}
	emit_changed();
}

void LayeredVoxelGenerator4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_layers"), &LayeredVoxelGenerator4D::get_layers_bind);
	ClassDB::bind_method(D_METHOD("set_layers", "layers"), &LayeredVoxelGenerator4D::set_layers_bind);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "layers", PROPERTY_HINT_ARRAY_TYPE, "VoxelGenerator4D"), "set_layers", "get_layers");
}
