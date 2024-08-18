#include "tetra_mesh_4d.h"

#include "array_tetra_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#elif GODOT_MODULE
#include "scene/resources/mesh.h"
#include "scene/resources/surface_tool.h"
#endif

Ref<ArrayMesh> TetraMesh4D::export_uvw_map_mesh() {
	const PackedVector3Array uvw_map = get_cell_uvw_map();
	Ref<SurfaceTool> surface_tool;
	surface_tool.instantiate();
	surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
	surface_tool->set_smooth_group(-1);
	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_flag(StandardMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	surface_tool->set_material(material);
	const int size = uvw_map.size();
	float hue = 0.0f;
	for (int i = 0; i < size - 3; i += 4) {
		surface_tool->set_color(Color::from_hsv(hue, 1.0f, 1.0f));
		surface_tool->add_vertex(uvw_map[i]);
		surface_tool->add_vertex(uvw_map[i + 1]);
		surface_tool->add_vertex(uvw_map[i + 2]);
		surface_tool->add_vertex(uvw_map[i]);
		surface_tool->add_vertex(uvw_map[i + 2]);
		surface_tool->add_vertex(uvw_map[i + 3]);
		surface_tool->add_vertex(uvw_map[i]);
		surface_tool->add_vertex(uvw_map[i + 3]);
		surface_tool->add_vertex(uvw_map[i + 1]);
		surface_tool->add_vertex(uvw_map[i + 1]);
		surface_tool->add_vertex(uvw_map[i + 3]);
		surface_tool->add_vertex(uvw_map[i + 2]);
		// Any irrational number will do here.
		hue += 0.045f * Math_E;
	}
	surface_tool->generate_normals();
	return surface_tool->commit();
}

void TetraMesh4D::tetra_mesh_clear_cache() {
	_edge_positions_cache.clear();
	_edge_indices_cache.clear();
}

Ref<ArrayTetraMesh4D> TetraMesh4D::to_array_tetra_mesh() {
	Ref<ArrayTetraMesh4D> array_mesh;
	array_mesh.instantiate();
	array_mesh->set_vertices(get_vertices());
	array_mesh->set_cell_indices(get_cell_indices());
	array_mesh->set_cell_normals(get_cell_normals());
	array_mesh->set_cell_uvw_map(get_cell_uvw_map());
	array_mesh->set_material(get_material());
	return array_mesh;
}

PackedInt32Array TetraMesh4D::get_cell_indices() {
	PackedInt32Array indices;
	GDVIRTUAL_CALL(_get_cell_indices, indices);
	return indices;
}

PackedVector4Array TetraMesh4D::get_cell_positions() {
	PackedVector4Array positions;
	GDVIRTUAL_CALL(_get_cell_positions, positions);
	return positions;
}

PackedVector4Array TetraMesh4D::get_cell_normals() {
	PackedVector4Array normals;
	GDVIRTUAL_CALL(_get_cell_normals, normals);
	return normals;
}

PackedVector3Array TetraMesh4D::get_cell_uvw_map() {
	PackedVector3Array uvw_map;
	GDVIRTUAL_CALL(_get_cell_uvw_map, uvw_map);
	return uvw_map;
}

PackedInt32Array TetraMesh4D::get_edge_indices() {
	if (_edge_indices_cache.is_empty()) {
		const PackedInt32Array cell_indices = get_cell_indices();
		const int stop = cell_indices.size() / 4;
		for (int i = 0; i < stop; i++) {
			const int cell_index = i * 4;
			_edge_indices_cache.append(cell_indices[cell_index + 0]);
			_edge_indices_cache.append(cell_indices[cell_index + 1]);
			_edge_indices_cache.append(cell_indices[cell_index + 0]);
			_edge_indices_cache.append(cell_indices[cell_index + 2]);
			_edge_indices_cache.append(cell_indices[cell_index + 0]);
			_edge_indices_cache.append(cell_indices[cell_index + 3]);
			_edge_indices_cache.append(cell_indices[cell_index + 1]);
			_edge_indices_cache.append(cell_indices[cell_index + 2]);
			_edge_indices_cache.append(cell_indices[cell_index + 1]);
			_edge_indices_cache.append(cell_indices[cell_index + 3]);
			_edge_indices_cache.append(cell_indices[cell_index + 2]);
			_edge_indices_cache.append(cell_indices[cell_index + 3]);
		}
	}
	_edge_indices_cache = deduplicate_edge_indices(_edge_indices_cache);
	return _edge_indices_cache;
}

PackedVector4Array TetraMesh4D::get_edge_positions() {
	if (_edge_positions_cache.is_empty()) {
		const PackedInt32Array edge_indices = get_edge_indices();
		const PackedVector4Array vertices = get_vertices();
		for (const int edge_index : edge_indices) {
			_edge_positions_cache.append(vertices[edge_index]);
		}
	}
	return _edge_positions_cache;
}

void TetraMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("export_uvw_map_mesh"), &TetraMesh4D::export_uvw_map_mesh);
	ClassDB::bind_method(D_METHOD("tetra_mesh_clear_cache"), &TetraMesh4D::tetra_mesh_clear_cache);
	ClassDB::bind_method(D_METHOD("to_array_tetra_mesh"), &TetraMesh4D::to_array_tetra_mesh);

	ClassDB::bind_method(D_METHOD("get_cell_indices"), &TetraMesh4D::get_cell_indices);
	ClassDB::bind_method(D_METHOD("get_cell_positions"), &TetraMesh4D::get_cell_positions);
	ClassDB::bind_method(D_METHOD("get_cell_normals"), &TetraMesh4D::get_cell_normals);
	ClassDB::bind_method(D_METHOD("get_cell_uvw_map"), &TetraMesh4D::get_cell_uvw_map);

	GDVIRTUAL_BIND(_get_cell_indices);
	GDVIRTUAL_BIND(_get_cell_positions);
	GDVIRTUAL_BIND(_get_cell_normals);
	GDVIRTUAL_BIND(_get_cell_uvw_map);
}
