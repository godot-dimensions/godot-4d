#pragma once

#include "../../../model/g4mf/g4mf_state_4d.h"
#include "../../../model/mesh/poly/box_poly_mesh_4d.h"

#include "tests/test_macros.h"

namespace TestG4MFMeshSurface4D {
static Ref<ArrayPolyMesh4D> make_poly_mesh() {
	Ref<BoxPolyMesh4D> box;
	box.instantiate();
	return box->to_array_poly_mesh();
}

static int make_empty_accessor(const Ref<G4MFState4D> &p_state, const int p_vector_size = 1) {
	Ref<G4MFAccessor4D> accessor = G4MFAccessor4D::make_new_accessor_without_data(p_vector_size == 1 ? "uint8" : "float64", p_vector_size);
	accessor->set_buffer_view_index(G4MFBufferView4D::write_new_buffer_view_into_state(p_state, PackedByteArray(), 8));
	TypedArray<G4MFAccessor4D> accessors = p_state->get_g4mf_accessors();
	const int index = accessors.size();
	accessors.append(accessor);
	p_state->set_g4mf_accessors(accessors);
	return index;
}

TEST_CASE("[G4MFMeshSurface4D] Poly bindings retain missing cell positions on round trip") {
	for (const bool deduplicate : { false, true }) {
		for (int missing_pattern = 0; missing_pattern < 5; missing_pattern++) {
			CAPTURE(deduplicate);
			CAPTURE(missing_pattern);
			Ref<ArrayPolyMesh4D> source = make_poly_mesh();
			Vector<PackedVector4Array> normals = source->get_poly_cell_vertex_normals();
			Vector<PackedVector3Array> texture = source->get_poly_cell_texture_map();
			const int64_t cell_count = normals.size();
			REQUIRE(cell_count > 3);
			REQUIRE(texture.size() == cell_count);
			if (missing_pattern == 1) {
				normals.clear();
				texture.clear();
			} else if (missing_pattern == 2) {
				for (int64_t i = 0; i < cell_count; i++) {
					normals.set(i, PackedVector4Array());
					texture.set(i, PackedVector3Array());
				}
			} else if (missing_pattern == 3) {
				normals.set(0, PackedVector4Array());
				normals.set(cell_count - 1, PackedVector4Array());
				texture.set(0, PackedVector3Array());
				texture.set(cell_count - 1, PackedVector3Array());
			} else if (missing_pattern == 4) {
				normals.set(cell_count / 2, PackedVector4Array());
				texture.set(cell_count / 2, PackedVector3Array());
			}
			source->set_poly_cell_vertex_normals(normals);
			source->set_poly_cell_texture_map(texture);
			Ref<G4MFState4D> state;
			state.instantiate();
			if (missing_pattern >= 3) {
				ERR_PRINT_OFF; // Sampling a mixture of populated and missing cells intentionally warns.
			}
			const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::convert_mesh_surface_for_state(state, source, deduplicate);
			if (missing_pattern >= 3) {
				ERR_PRINT_ON;
			}
			REQUIRE(surface.is_valid());
			const bool has_binding = missing_pattern != 1 && missing_pattern != 2;
			CHECK(surface->get_normals_binding().is_valid() == has_binding);
			CHECK(surface->get_texture_map_binding().is_valid() == has_binding);
			if (has_binding) {
				for (const Ref<G4MFMeshSurfaceBinding4D> &binding : { surface->get_normals_binding(), surface->get_texture_map_binding() }) {
					const PackedInt32Array packed = binding->load_geometry_binding_indices(state, 3, 0);
					int64_t offset = 0;
					for (int64_t cell = 0; cell < cell_count; cell++) {
						REQUIRE(offset < packed.size());
						CHECK(packed[offset] == normals[cell].size());
						offset += packed[offset] + 1;
					}
					CHECK(offset == packed.size());
				}
			}
			if (missing_pattern >= 3) {
				ERR_PRINT_OFF; // Import validation samples the deliberately partial texture mapping.
			}
			const Ref<ArrayPolyMesh4D> imported = surface->generate_poly_mesh_surface(state, source->get_vertex_positions());
			if (missing_pattern >= 3) {
				ERR_PRINT_ON;
			}
			REQUIRE(imported.is_valid());
			CHECK(imported->get_poly_cell_vertex_positions() == source->get_vertex_positions());
			CHECK(imported->get_poly_cell_indices() == source->get_poly_cell_indices());
			CHECK(imported->get_poly_cell_vertex_normals() == (has_binding ? normals : Vector<PackedVector4Array>()));
			CHECK(imported->get_poly_cell_texture_map() == (has_binding ? texture : Vector<PackedVector3Array>()));
			CHECK(imported->is_poly_mesh_data_valid());
			if (missing_pattern == 0) {
				const Ref<ArrayTetraMesh4D> tetra = surface->generate_tetra_mesh_surface(state, source->get_vertex_positions());
				REQUIRE(tetra.is_valid());
				CHECK(tetra->get_simplex_cell_vertex_indices() == source->get_simplex_cell_vertex_indices());
				CHECK(tetra->get_simplex_cell_vertex_normals() == source->get_simplex_cell_vertex_normals());
				CHECK(tetra->get_simplex_cell_texture_map() == source->get_simplex_cell_texture_map());
			}
		}
	}
}

TEST_CASE("[G4MFMeshSurface4D] Invalid packed binding counts and indices are rejected") {
	const Vector<PackedInt32Array> malformed_bindings = {
		PackedInt32Array{ -1 },
		PackedInt32Array{ INT32_MAX },
		PackedInt32Array{ 4, 0, 1 },
		PackedInt32Array{ 1, -1, 0, 0, 0, 0, 0, 0, 0 },
		PackedInt32Array{ 1, INT32_MAX, 0, 0, 0, 0, 0, 0, 0 },
		PackedInt32Array{ 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // Full outer count, wrong populated inner count.
		PackedInt32Array{ 0 }, // Missing boundary cells.
		PackedInt32Array{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Too many boundary cells.
	};
	for (const bool normal_binding : { false, true }) {
		for (const PackedInt32Array &packed : malformed_bindings) {
			CAPTURE(normal_binding);
			CAPTURE(packed);
			Ref<G4MFState4D> state;
			state.instantiate();
			const Ref<ArrayPolyMesh4D> source = make_poly_mesh();
			const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::convert_mesh_surface_for_state(state, source);
			const Ref<G4MFMeshSurfaceBinding4D> binding = normal_binding ? surface->get_normals_binding() : surface->get_texture_map_binding();
			const Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_binding = binding->get_geometry_bindings()[0];
			geometry_binding->set_indices_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, packed));
			ERR_PRINT_OFF;
			const Ref<ArrayPolyMesh4D> poly_mesh_surface = surface->generate_poly_mesh_surface(state, source->get_vertex_positions());
			ERR_PRINT_ON;
			CHECK(poly_mesh_surface.is_null());
		}
	}
}

TEST_CASE("[G4MFMeshSurface4D] Missing accessor objects and truncated buffers fail safely") {
	for (const bool normal_binding : { false, true }) {
		for (int corruption = 0; corruption < 4; corruption++) {
			CAPTURE(normal_binding);
			CAPTURE(corruption);
			Ref<G4MFState4D> state;
			state.instantiate();
			const Ref<ArrayPolyMesh4D> source = make_poly_mesh();
			const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::convert_mesh_surface_for_state(state, source);
			const Ref<G4MFMeshSurfaceBinding4D> binding = normal_binding ? surface->get_normals_binding() : surface->get_texture_map_binding();
			const Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_binding = binding->get_geometry_bindings()[0];
			const int accessor_index = geometry_binding->get_indices_accessor_index();
			if (corruption == 0) {
				TypedArray<G4MFAccessor4D> accessors = state->get_g4mf_accessors();
				accessors[accessor_index] = Ref<G4MFAccessor4D>();
				state->set_g4mf_accessors(accessors);
			} else if (corruption == 1) {
				const Ref<G4MFAccessor4D> accessor = state->get_g4mf_accessors()[accessor_index];
				TypedArray<G4MFBufferView4D> buffer_views = state->get_g4mf_buffer_views();
				buffer_views[accessor->get_buffer_view_index()] = Ref<G4MFBufferView4D>();
				state->set_g4mf_buffer_views(buffer_views);
			} else if (corruption == 2) {
				// Keep an accessor with a complete scalar index type, but truncate its packed cell record.
				const Ref<G4MFAccessor4D> accessor = state->get_g4mf_accessors()[accessor_index];
				const Ref<G4MFBufferView4D> buffer_view = state->get_g4mf_buffer_views()[accessor->get_buffer_view_index()];
				buffer_view->set_byte_length(buffer_view->get_byte_length() - accessor->get_bytes_per_component());
			} else {
				TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry;
				geometry.append(Ref<G4MFMeshSurfaceBindingGeometry4D>());
				binding->set_geometry_bindings(geometry);
			}
			if (corruption < 2) {
				ERR_PRINT_OFF;
				const PackedInt32Array indices = geometry_binding->load_indices(state);
				ERR_PRINT_ON;
				CHECK(indices.is_empty());
			} else {
				ERR_PRINT_OFF;
				Ref<ArrayPolyMesh4D> poly_mesh_surface = surface->generate_poly_mesh_surface(state, source->get_vertex_positions());
				ERR_PRINT_ON;
				CHECK(poly_mesh_surface.is_null());
			}
		}
	}
}

TEST_CASE("[G4MFMeshSurface4D] Declared zero-length accessors preserve empty geometry and attributes") {
	Ref<G4MFState4D> state;
	state.instantiate();
	const int empty_indices = make_empty_accessor(state);
	const int empty_normals = make_empty_accessor(state, 4);
	const int empty_texture = make_empty_accessor(state, 3);
	Ref<G4MFMeshSurface4D> surface;
	surface.instantiate();
	surface->set_simplexes_accessor_index(empty_indices);
	surface->set_edges_accessor_index(empty_indices);
	surface->set_geometry_accessor_indices(PackedInt32Array{ empty_indices, empty_indices });
	Ref<G4MFMeshSurfaceBinding4D> normals;
	normals.instantiate();
	normals->set_values_accessor_index(empty_normals);
	normals->set_simplexes_accessor_index(empty_indices);
	Ref<G4MFMeshSurfaceBinding4D> texture;
	texture.instantiate();
	texture->set_values_accessor_index(empty_texture);
	texture->set_simplexes_accessor_index(empty_indices);
	Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_binding;
	geometry_binding.instantiate();
	geometry_binding->set_geometry_dimension(3);
	geometry_binding->set_indices_accessor_index(empty_indices);
	TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_bindings;
	geometry_bindings.append(geometry_binding);
	normals->set_geometry_bindings(geometry_bindings);
	texture->set_geometry_bindings(geometry_bindings);
	surface->set_normals_binding(normals);
	surface->set_texture_map_binding(texture);
	const PackedVector4Array vertices = { Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8) };
	const Ref<ArrayTetraMesh4D> tetra = surface->generate_tetra_mesh_surface(state, vertices);
	REQUIRE(tetra.is_valid());
	CHECK(tetra->get_vertex_positions() == vertices);
	CHECK(tetra->get_simplex_cell_vertex_indices().is_empty());
	CHECK(tetra->get_simplex_cell_vertex_normals().is_empty());
	CHECK(tetra->get_simplex_cell_texture_map().is_empty());
	const Ref<ArrayPolyMesh4D> poly = surface->generate_poly_mesh_surface(state, vertices);
	REQUIRE(poly.is_valid());
	CHECK(poly->get_poly_cell_vertex_positions() == vertices);
	CHECK(poly->get_poly_cell_vertex_normals().is_empty());
	CHECK(poly->get_poly_cell_texture_map().is_empty());
	const Vector<Vector<PackedInt32Array>> separated = surface->load_geometry_separated(state);
	REQUIRE(separated.size() == 2);
	CHECK(separated[0].is_empty());
	CHECK(separated[1].is_empty());
	const TypedArray<Array> separated_bind = surface->load_geometry_separated_bind(state);
	REQUIRE(separated_bind.size() == 2);
	CHECK(Array(separated_bind[0]).is_empty());
	CHECK(Array(separated_bind[1]).is_empty());
	const Ref<ArrayWireMesh4D> wire = surface->generate_wire_mesh_surface(state, vertices);
	REQUIRE(wire.is_valid());
	CHECK(wire->get_vertex_positions() == vertices);
}

TEST_CASE("[G4MFMeshSurface4D] Zero-count cell bindings may reference an empty values accessor") {
	for (const bool normal_binding : { false, true }) {
		Ref<G4MFState4D> state;
		state.instantiate();
		const Ref<ArrayPolyMesh4D> source = make_poly_mesh();
		const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::convert_mesh_surface_for_state(state, source);
		const Ref<G4MFMeshSurfaceBinding4D> binding = normal_binding ? surface->get_normals_binding() : surface->get_texture_map_binding();
		const Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_binding = binding->get_geometry_bindings()[0];
		geometry_binding->set_indices_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, PackedInt32Array{ 0, 0, 0, 0, 0, 0, 0, 0 }));
		binding->set_values_accessor_index(make_empty_accessor(state, normal_binding ? 4 : 3));
		const Ref<ArrayPolyMesh4D> imported = surface->generate_poly_mesh_surface(state, source->get_vertex_positions());
		REQUIRE(imported.is_valid());
		if (normal_binding) {
			const Vector<PackedVector4Array> normals = imported->get_poly_cell_vertex_normals();
			REQUIRE(normals.size() == 8);
			for (const PackedVector4Array &cell : normals) {
				CHECK(cell.is_empty());
			}
		} else {
			const Vector<PackedVector3Array> texture = imported->get_poly_cell_texture_map();
			REQUIRE(texture.size() == 8);
			for (const PackedVector3Array &cell : texture) {
				CHECK(cell.is_empty());
			}
		}
	}
}

TEST_CASE("[G4MFMeshSurface4D] Packed geometry and simplex references are checked before sampling") {
	for (const PackedInt32Array &packed : { PackedInt32Array{ -1 }, PackedInt32Array{ 3, 0, 1 }, PackedInt32Array{ 3, 0, 1, 2, INT32_MAX } }) {
		Ref<G4MFState4D> state;
		state.instantiate();
		Ref<G4MFMeshSurface4D> surface;
		surface.instantiate();
		surface->set_geometry_accessor_indices(PackedInt32Array{ G4MFAccessor4D::encode_new_accessor_from_int32s(state, packed) });
		ERR_PRINT_OFF;
		const Vector<Vector<PackedInt32Array>> separated = surface->load_geometry_separated(state);
		ERR_PRINT_ON;
		CHECK(separated.is_empty());
		ERR_PRINT_OFF;
		const TypedArray<Array> separated_bind = surface->load_geometry_separated_bind(state);
		ERR_PRINT_ON;
		CHECK(separated_bind.is_empty());
	}
	const PackedVector4Array vertices = { Vector4(), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0) };
	for (const PackedInt32Array &indices : { PackedInt32Array{ 0, 1, 2 }, PackedInt32Array{ 0, 1, 2, 4 }, PackedInt32Array{ 0, 1, 2, -1 } }) {
		Ref<G4MFState4D> state;
		state.instantiate();
		Ref<G4MFMeshSurface4D> surface;
		surface.instantiate();
		surface->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, indices));
		ERR_PRINT_OFF;
		const Ref<ArrayTetraMesh4D> tetra_mesh_surface = surface->generate_tetra_mesh_surface(state, vertices);
		ERR_PRINT_ON;
		CHECK(tetra_mesh_surface.is_null());
		ERR_PRINT_OFF;
		const Ref<ArrayWireMesh4D> wire_mesh_surface = surface->generate_wire_mesh_surface(state, vertices);
		ERR_PRINT_ON;
		CHECK(wire_mesh_surface.is_null());
	}
}

TEST_CASE("[G4MFMeshSurface4D] Simplex attribute counts and value ranges are validated") {
	for (const bool normal_binding : { false, true }) {
		for (int corruption = 0; corruption < 4; corruption++) {
			CAPTURE(normal_binding);
			CAPTURE(corruption);
			Ref<G4MFState4D> state;
			state.instantiate();
			const Ref<ArrayPolyMesh4D> source = make_poly_mesh();
			const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::convert_mesh_surface_for_state(state, source);
			const Ref<G4MFMeshSurfaceBinding4D> binding = normal_binding ? surface->get_normals_binding() : surface->get_texture_map_binding();
			PackedInt32Array indices = binding->load_simplex_indices(state);
			REQUIRE(indices.size() > 1);
			if (corruption == 0) {
				indices.resize(indices.size() - 1);
			} else if (corruption == 1) {
				indices.append(indices[0]);
			} else {
				indices.set(0, corruption == 2 ? -1 : INT32_MAX);
			}
			binding->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, indices));
			ERR_PRINT_OFF;
			const Ref<ArrayTetraMesh4D> tetra_mesh_surface = surface->generate_tetra_mesh_surface(state, source->get_vertex_positions());
			ERR_PRINT_ON;
			CHECK(tetra_mesh_surface.is_null());
		}
	}
}

TEST_CASE("[G4MFMeshSurface4D] Empty-cell imports and conversions preserve vertex positions") {
	const PackedVector4Array vertices = { Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, 12) };
	Ref<G4MFState4D> state;
	state.instantiate();
	Ref<ArrayTetraMesh4D> source;
	source.instantiate();
	source->set_vertex_positions(vertices);
	const int mesh_index = G4MFMesh4D::export_convert_mesh_into_state(state, source);
	REQUIRE(mesh_index >= 0);
	const Ref<G4MFMesh4D> stored_mesh = state->get_g4mf_meshes()[mesh_index];
	const Ref<TetraMesh4D> imported = stored_mesh->import_generate_tetra_mesh(state);
	REQUIRE(imported.is_valid());
	CHECK(imported->get_vertex_positions() == vertices);
	CHECK(imported->get_simplex_cell_vertex_indices().is_empty());
	const Ref<ArrayTetraMesh4D> converted = imported->to_array_tetra_mesh();
	CHECK(converted->get_vertex_positions() == vertices);
	Ref<G4MFMeshSurface4D> surface;
	surface.instantiate();
	for (const bool with_edges : { false, true }) {
		if (with_edges) {
			surface->set_edges_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, PackedInt32Array{ 0, 1, 1, 2, 2, 0 }));
			surface->convert_separated_geometry_into_packed(state, Vector<Vector<PackedInt32Array>>{ Vector<PackedInt32Array>{ PackedInt32Array{ 0, 1, 2 } } }, true);
		}
		const Ref<ArrayPolyMesh4D> poly = surface->generate_poly_mesh_surface(state, vertices);
		REQUIRE(poly.is_valid());
		CHECK(poly->get_poly_cell_vertex_positions() == vertices);
		CHECK(poly->get_simplex_cell_vertex_indices().is_empty());
		const Ref<ArrayWireMesh4D> wire = surface->generate_wire_mesh_surface(state, vertices);
		REQUIRE(wire.is_valid());
		CHECK(wire->get_vertex_positions() == vertices);
	}
}
} // namespace TestG4MFMeshSurface4D
