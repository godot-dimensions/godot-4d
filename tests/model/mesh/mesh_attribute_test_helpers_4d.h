#pragma once

#include "../../../model/mesh/tetra/array_tetra_mesh_4d.h"

namespace TestMeshAttributes4D {

inline PackedVector4Array sample_normal_values(const Ref<TetraMesh4D> &p_mesh) {
	const PackedInt32Array indices = p_mesh->get_simplex_cell_normal_indices();
	const PackedVector4Array values = p_mesh->get_normal_values();
	PackedVector4Array sampled;
	for (const int32_t index : indices) {
		sampled.append(values[index]);
	}
	return sampled;
}

inline PackedVector3Array sample_texture_map_values(const Ref<TetraMesh4D> &p_mesh) {
	const PackedInt32Array indices = p_mesh->get_simplex_cell_texture_map_indices();
	const PackedVector3Array values = p_mesh->get_texture_map_values();
	PackedVector3Array sampled;
	for (const int32_t index : indices) {
		sampled.append(values[index]);
	}
	return sampled;
}

inline void set_simplex_normal_values(const Ref<ArrayTetraMesh4D> &p_mesh, const PackedVector4Array &p_values) {
	PackedInt32Array indices;
	for (int64_t i = 0; i < p_values.size(); i++) {
		indices.append(i);
	}
	p_mesh->set_normal_values(p_values);
	p_mesh->set_simplex_cell_normal_indices(indices);
}

inline void set_simplex_texture_map_values(const Ref<ArrayTetraMesh4D> &p_mesh, const PackedVector3Array &p_values) {
	PackedInt32Array indices;
	for (int64_t i = 0; i < p_values.size(); i++) {
		indices.append(i);
	}
	p_mesh->set_texture_map_values(p_values);
	p_mesh->set_simplex_cell_texture_map_indices(indices);
}

} // namespace TestMeshAttributes4D
