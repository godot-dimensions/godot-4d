#pragma once

#include "../../../model/g4mf/g4mf_state_4d.h"
#include "../../../model/mesh/poly/box_poly_mesh_4d.h"
#include "../../../model/mesh/wire/array_wire_mesh_4d.h"

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

// Exported bindings may hold several geometry bindings, such as (3, 0) vertex normals next to (3, 3)
// boundary normals, in HashMap order. Look up the wanted one by its dimensions instead of by position.
static Ref<G4MFMeshSurfaceBindingGeometry4D> find_geometry_binding(const Ref<G4MFMeshSurfaceBinding4D> &p_binding, const int p_geometry_dimension, const int p_decompose_dimension) {
	REQUIRE(p_binding.is_valid());
	const TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_bindings = p_binding->get_geometry_bindings();
	for (int i = 0; i < geometry_bindings.size(); i++) {
		const Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_binding = geometry_bindings[i];
		if (geometry_binding.is_valid() && geometry_binding->get_geometry_dimension() == p_geometry_dimension && geometry_binding->get_decompose_dimension() == p_decompose_dimension) {
			return geometry_binding;
		}
	}
	return Ref<G4MFMeshSurfaceBindingGeometry4D>();
}

TEST_CASE("[G4MFMeshSurface4D] Poly bindings retain missing cell positions on round trip") {
	for (const bool deduplicate : { false, true }) {
		for (int missing_pattern = 0; missing_pattern < 5; missing_pattern++) {
			CAPTURE(deduplicate);
			CAPTURE(missing_pattern);
			Ref<ArrayPolyMesh4D> source = make_poly_mesh();
			Vector<PackedVector4Array> normals = source->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
			Vector<PackedVector3Array> texture = source->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
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
			source->set_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY, normals);
			source->set_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY, texture);
			Ref<G4MFState4D> state;
			state.instantiate();
			if (missing_pattern >= 3) {
				ERR_PRINT_OFF; // Sampling a mixture of populated and missing cells intentionally warns.
			}
			PackedVector4Array shared_vertices;
			const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(state, source, shared_vertices, deduplicate);
			if (missing_pattern >= 3) {
				ERR_PRINT_ON;
			}
			REQUIRE(surface.is_valid());
			const bool has_vertex_binding = missing_pattern != 1 && missing_pattern != 2;
			// Boundary normals are always exported as a (3, 3) binding, so the normals binding exists
			// even without vertex normals. Texture maps have no such fallback.
			CHECK(surface->get_normals_binding().is_valid());
			CHECK(find_geometry_binding(surface->get_normals_binding(), 3, 3).is_valid());
			CHECK(find_geometry_binding(surface->get_normals_binding(), 3, 0).is_valid() == has_vertex_binding);
			CHECK(surface->get_texture_map_binding().is_valid() == has_vertex_binding);
			if (has_vertex_binding) {
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
			const Ref<ArrayPolyMesh4D> imported = surface->import_generate_poly_mesh_surface(state, source->get_vertex_positions());
			if (missing_pattern >= 3) {
				ERR_PRINT_ON;
			}
			REQUIRE(imported.is_valid());
			CHECK(imported->get_poly_cell_vertex_positions() == source->get_vertex_positions());
			CHECK(imported->get_poly_cell_indices() == source->get_poly_cell_indices());
			CHECK(imported->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY) == (has_vertex_binding ? normals : Vector<PackedVector4Array>()));
			CHECK(imported->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY) == (has_vertex_binding ? texture : Vector<PackedVector3Array>()));
			CHECK(imported->get_poly_cell_boundary_normals() == source->get_poly_cell_boundary_normals());
			CHECK(imported->is_poly_mesh_data_valid());
			if (missing_pattern == 0) {
				const Ref<ArrayTetraMesh4D> tetra = surface->import_generate_tetra_mesh_surface(state, source->get_vertex_positions());
				REQUIRE(tetra.is_valid());
				CHECK(tetra->get_simplex_cell_vertex_indices() == source->get_simplex_cell_vertex_indices());
				CHECK(tetra->get_normal_values() == source->get_normal_values());
				CHECK(tetra->get_simplex_cell_normal_indices() == source->get_simplex_cell_normal_indices());
				CHECK(tetra->get_texture_map_values() == source->get_texture_map_values());
				CHECK(tetra->get_simplex_cell_texture_map_indices() == source->get_simplex_cell_texture_map_indices());
			}
			CHECK(shared_vertices == source->get_vertex_positions());
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
		PackedInt32Array{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Too many boundary cells. Fewer is allowed, see the short bindings test.
	};
	for (const bool normal_binding : { false, true }) {
		for (const PackedInt32Array &packed : malformed_bindings) {
			CAPTURE(normal_binding);
			CAPTURE(packed);
			Ref<G4MFState4D> state;
			state.instantiate();
			const Ref<ArrayPolyMesh4D> source = make_poly_mesh();
			PackedVector4Array shared_vertices;
			const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(state, source, shared_vertices);
			const Ref<G4MFMeshSurfaceBinding4D> binding = normal_binding ? surface->get_normals_binding() : surface->get_texture_map_binding();
			const Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_binding = find_geometry_binding(binding, 3, 0);
			REQUIRE(geometry_binding.is_valid());
			geometry_binding->set_indices_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, packed, 1));
			ERR_PRINT_OFF;
			const Ref<ArrayPolyMesh4D> poly_mesh_surface = surface->import_generate_poly_mesh_surface(state, source->get_vertex_positions());
			ERR_PRINT_ON;
			CHECK(poly_mesh_surface.is_null());
			CHECK(shared_vertices == source->get_vertex_positions());
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
			PackedVector4Array shared_vertices;
			const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(state, source, shared_vertices);
			const Ref<G4MFMeshSurfaceBinding4D> binding = normal_binding ? surface->get_normals_binding() : surface->get_texture_map_binding();
			const Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_binding = find_geometry_binding(binding, 3, 0);
			REQUIRE(geometry_binding.is_valid());
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
				Ref<ArrayPolyMesh4D> poly_mesh_surface = surface->import_generate_poly_mesh_surface(state, source->get_vertex_positions());
				ERR_PRINT_ON;
				CHECK(poly_mesh_surface.is_null());
			}
			CHECK(shared_vertices == source->get_vertex_positions());
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
	const Ref<ArrayTetraMesh4D> tetra = surface->import_generate_tetra_mesh_surface(state, vertices);
	REQUIRE(tetra.is_valid());
	CHECK(tetra->get_vertex_positions() == vertices);
	CHECK(tetra->get_simplex_cell_vertex_indices().is_empty());
	CHECK(tetra->get_simplex_cell_normal_indices().is_empty());
	CHECK(tetra->get_normal_values().is_empty());
	CHECK(tetra->get_simplex_cell_texture_map_indices().is_empty());
	CHECK(tetra->get_texture_map_values().is_empty());
	const Ref<ArrayPolyMesh4D> poly = surface->import_generate_poly_mesh_surface(state, vertices);
	REQUIRE(poly.is_valid());
	CHECK(poly->get_poly_cell_vertex_positions() == vertices);
	CHECK(poly->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY).is_empty());
	CHECK(poly->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY).is_empty());
	const Vector<Vector<PackedInt32Array>> separated = surface->load_geometry_separated(state);
	REQUIRE(separated.size() == 2);
	CHECK(separated[0].is_empty());
	CHECK(separated[1].is_empty());
	const TypedArray<Array> separated_bind = surface->load_geometry_separated_bind(state);
	REQUIRE(separated_bind.size() == 2);
	CHECK(Array(separated_bind[0]).is_empty());
	CHECK(Array(separated_bind[1]).is_empty());
	const Ref<ArrayWireMesh4D> wire = surface->import_generate_wire_mesh_surface(state, vertices);
	REQUIRE(wire.is_valid());
	CHECK(wire->get_vertex_positions() == vertices);
}

TEST_CASE("[G4MFMeshSurface4D] Zero-count cell bindings may reference an empty values accessor") {
	for (const bool normal_binding : { false, true }) {
		Ref<G4MFState4D> state;
		state.instantiate();
		const Ref<ArrayPolyMesh4D> source = make_poly_mesh();
		PackedVector4Array shared_vertices;
		const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(state, source, shared_vertices);
		const Ref<G4MFMeshSurfaceBinding4D> binding = normal_binding ? surface->get_normals_binding() : surface->get_texture_map_binding();
		const Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_binding = find_geometry_binding(binding, 3, 0);
		REQUIRE(geometry_binding.is_valid());
		geometry_binding->set_indices_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, PackedInt32Array{ 0, 0, 0, 0, 0, 0, 0, 0 }, 1));
		// Keep only the zero-count binding. The exported (3, 3) boundary normals reference real values,
		// which an empty values accessor could not satisfy, and they are not what this test is about.
		TypedArray<G4MFMeshSurfaceBindingGeometry4D> only_zero_count;
		only_zero_count.append(geometry_binding);
		binding->set_geometry_bindings(only_zero_count);
		binding->set_values_accessor_index(make_empty_accessor(state, normal_binding ? 4 : 3));
		const Ref<ArrayPolyMesh4D> imported = surface->import_generate_poly_mesh_surface(state, source->get_vertex_positions());
		REQUIRE(imported.is_valid());
		if (normal_binding) {
			const Vector<PackedVector4Array> normals = imported->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
			REQUIRE(normals.size() == 8);
			for (const PackedVector4Array &cell : normals) {
				CHECK(cell.is_empty());
			}
		} else {
			const Vector<PackedVector3Array> texture = imported->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
			REQUIRE(texture.size() == 8);
			for (const PackedVector3Array &cell : texture) {
				CHECK(cell.is_empty());
			}
		}
		CHECK(shared_vertices == source->get_vertex_positions());
	}
}

TEST_CASE("[G4MFMeshSurface4D] Packed geometry and simplex references are checked before sampling") {
	for (const PackedInt32Array &packed : { PackedInt32Array{ -1 }, PackedInt32Array{ 3, 0, 1 }, PackedInt32Array{ 3, 0, 1, 2, INT32_MAX } }) {
		Ref<G4MFState4D> state;
		state.instantiate();
		Ref<G4MFMeshSurface4D> surface;
		surface.instantiate();
		surface->set_geometry_accessor_indices(PackedInt32Array{ G4MFAccessor4D::encode_new_accessor_from_int32s(state, packed, 1) });
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
		// The export refuses to encode a count that is not a multiple of the vector size, so store
		// a wrong count as scalars to ensure the accessor exists and the import has to reject it.
		const int vector_size = indices.size() % 4 == 0 ? 4 : 1;
		surface->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, indices, vector_size));
		ERR_PRINT_OFF;
		const Ref<ArrayTetraMesh4D> tetra_mesh_surface = surface->import_generate_tetra_mesh_surface(state, vertices);
		ERR_PRINT_ON;
		CHECK(tetra_mesh_surface.is_null());
		ERR_PRINT_OFF;
		const Ref<ArrayWireMesh4D> wire_mesh_surface = surface->import_generate_wire_mesh_surface(state, vertices);
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
			PackedVector4Array shared_vertices;
			const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(state, source, shared_vertices);
			const Ref<G4MFMeshSurfaceBinding4D> binding = normal_binding ? surface->get_normals_binding() : surface->get_texture_map_binding();
			PackedInt32Array indices = binding->load_simplex_indices(state);
			REQUIRE(indices.size() > 1);
			if (corruption == 0) {
				indices.resize(indices.size() - 1);
			} else if (corruption == 1) {
				indices.append(indices[0]);
			} else if (corruption == 2) {
				indices.set(0, -1);
			} else {
				indices.set(0, INT32_MAX);
			}
			// The export refuses to encode a count that is not a multiple of the vector size, so store
			// a wrong count as scalars to ensure the accessor exists and the import has to reject it.
			const int vector_size = corruption < 2 ? 1 : 4;
			binding->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, indices, vector_size));
			ERR_PRINT_OFF;
			const Ref<ArrayTetraMesh4D> tetra_mesh_surface = surface->import_generate_tetra_mesh_surface(state, source->get_vertex_positions());
			ERR_PRINT_ON;
			CHECK(tetra_mesh_surface.is_null());
			CHECK(shared_vertices == source->get_vertex_positions());
		}
	}
}

TEST_CASE("[G4MFMeshSurface4D] Hidden boundary cells retain geometry binding values") {
	Ref<ArrayPolyMesh4D> source = make_poly_mesh();
	Vector<Vector<PackedInt32Array>> poly = source->get_poly_cell_indices();
	poly.ptrw()[2].append(poly[2][0]);
	source->set_poly_cell_indices(poly);
	REQUIRE(source->is_poly_mesh_data_valid());
	REQUIRE(source->get_simplex_cell_vertex_indices().is_empty());
	for (const bool deduplicate : { false, true }) {
		Ref<G4MFState4D> state;
		state.instantiate();
		PackedVector4Array shared_vertices;
		const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(state, source, shared_vertices, deduplicate);
		REQUIRE(surface.is_valid());
		CHECK(surface->get_simplexes_accessor_index() == -1);
		const Ref<ArrayPolyMesh4D> imported = surface->import_generate_poly_mesh_surface(state, source->get_poly_cell_vertex_positions());
		REQUIRE(imported.is_valid());
		CHECK(imported->get_poly_cell_normal_values() == source->get_poly_cell_normal_values());
		CHECK(imported->get_poly_cell_texture_map_values() == source->get_poly_cell_texture_map_values());
		CHECK(imported->get_poly_cell_normal_indices() == source->get_poly_cell_normal_indices());
		CHECK(imported->get_poly_cell_texture_map_indices() == source->get_poly_cell_texture_map_indices());
		CHECK(shared_vertices == source->get_vertex_positions());
	}
}

TEST_CASE("[G4MFMeshSurface4D] Unreferenced value pools do not create bindings") {
	Ref<ArrayTetraMesh4D> source;
	source.instantiate();
	source->set_vertex_positions(PackedVector4Array{ Vector4(), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0) });
	source->set_normal_values(PackedVector4Array{ Vector4(0, 0, 0, 1) });
	source->set_texture_map_values(PackedVector3Array{ Vector3(1, 2, 3) });
	source->set_simplex_cell_vertex_indices(PackedInt32Array{ 0, 1, 2, 3 });
	for (const bool deduplicate : { false, true }) {
		Ref<G4MFState4D> state;
		state.instantiate();
		PackedVector4Array shared_vertices;
		const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(state, source, shared_vertices, deduplicate);
		REQUIRE(surface.is_valid());
		CHECK(surface->get_simplexes_accessor_index() >= 0);
		CHECK(surface->get_normals_binding().is_null());
		CHECK(surface->get_texture_map_binding().is_null());
		CHECK(shared_vertices == source->get_vertex_positions());
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
	const Ref<G4MFMeshSurface4D> stored_surface = stored_mesh->get_surfaces()[0];
	const PackedVector4Array stored_vertices = stored_mesh->load_vertices(state);
	const Ref<TetraMesh4D> imported = stored_surface->import_generate_tetra_mesh_surface(state, stored_vertices);
	REQUIRE(imported.is_valid());
	CHECK(imported->get_vertex_positions() == vertices);
	CHECK(imported->get_simplex_cell_vertex_indices().is_empty());
	const Ref<ArrayTetraMesh4D> converted = imported->to_array_tetra_mesh();
	CHECK(converted->get_vertex_positions() == vertices);
	Ref<G4MFMeshSurface4D> surface;
	surface.instantiate();
	for (const bool with_edges : { false, true }) {
		if (with_edges) {
			surface->set_edges_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, PackedInt32Array{ 0, 1, 1, 2, 2, 0 }, 2));
			surface->convert_separated_geometry_into_packed(state, Vector<Vector<PackedInt32Array>>{ Vector<PackedInt32Array>{ PackedInt32Array{ 0, 1, 2 } } }, true);
		}
		const Ref<ArrayPolyMesh4D> poly = surface->import_generate_poly_mesh_surface(state, vertices);
		REQUIRE(poly.is_valid());
		CHECK(poly->get_poly_cell_vertex_positions() == vertices);
		CHECK(poly->get_simplex_cell_vertex_indices().is_empty());
		const Ref<ArrayWireMesh4D> wire = surface->import_generate_wire_mesh_surface(state, vertices);
		REQUIRE(wire.is_valid());
		CHECK(wire->get_vertex_positions() == vertices);
	}
}

TEST_CASE("[G4MFMeshSurface4D] Bindings shorter than their element counts import with default values") {
	Ref<G4MFState4D> state;
	state.instantiate();
	Ref<ArrayPolyMesh4D> source = make_poly_mesh();
	// Give the box per-edge-vertex normals so the (1, 0) dense-pair layout is exercised.
	const PackedInt32Array edge_indices = source->get_edge_indices();
	const int64_t edge_count = edge_indices.size() / 2;
	REQUIRE(edge_count > 2);
	const PackedVector4Array vertices = source->get_vertex_positions();
	Vector<PackedVector4Array> edge_vertex_normals;
	for (int64_t edge = 0; edge < edge_count; edge++) {
		edge_vertex_normals.push_back({ vertices[edge_indices[edge * 2]].normalized(), vertices[edge_indices[edge * 2 + 1]].normalized() });
	}
	source->set_poly_cell_dense_normals(Vector2i(1, 0), edge_vertex_normals);
	REQUIRE(source->is_mesh_data_valid());
	PackedVector4Array shared_vertices;
	const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(state, source, shared_vertices);
	REQUIRE(surface.is_valid());
	const Ref<G4MFMeshSurfaceBinding4D> normals_binding = surface->get_normals_binding();
	REQUIRE(normals_binding.is_valid());

	SUBCASE("A short edge-vertex geometry binding keeps the edges it covers") {
		const Ref<G4MFMeshSurfaceBindingGeometry4D> edge_binding = find_geometry_binding(normals_binding, 1, 0);
		REQUIRE(edge_binding.is_valid());
		PackedInt32Array indices = edge_binding->load_indices(state);
		REQUIRE(indices.size() == edge_count * 2);
		const int64_t kept_edges = edge_count / 2;
		indices.resize(kept_edges * 2);
		edge_binding->set_indices_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, indices, 2, false));
		const Ref<ArrayPolyMesh4D> imported = surface->import_generate_poly_mesh_surface(state, shared_vertices);
		REQUIRE(imported.is_valid());
		CHECK(imported->is_mesh_data_valid());
		const Vector<PackedVector4Array> imported_edge_normals = imported->get_poly_cell_dense_normals(Vector2i(1, 0));
		REQUIRE(imported_edge_normals.size() == kept_edges);
		for (int64_t edge = 0; edge < kept_edges; edge++) {
			CHECK(imported_edge_normals[edge] == edge_vertex_normals[edge]);
		}
	}

	SUBCASE("An odd number of edge-vertex indices is rejected") {
		const Ref<G4MFMeshSurfaceBindingGeometry4D> edge_binding = find_geometry_binding(normals_binding, 1, 0);
		REQUIRE(edge_binding.is_valid());
		PackedInt32Array indices = edge_binding->load_indices(state);
		indices.resize(indices.size() - 1);
		edge_binding->set_indices_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, indices, 1, false));
		ERR_PRINT_OFF;
		const Ref<ArrayPolyMesh4D> imported = surface->import_generate_poly_mesh_surface(state, shared_vertices);
		ERR_PRINT_ON;
		CHECK(imported.is_null());
	}

	SUBCASE("A short simplex corner binding pads the missing corners with a zero value") {
		REQUIRE(normals_binding->get_simplexes_accessor_index() >= 0);
		PackedInt32Array corner_indices = normals_binding->load_simplex_indices(state);
		const int64_t corner_count = source->get_simplex_cell_vertex_indices().size();
		REQUIRE(corner_indices.size() == corner_count);
		const int64_t kept_corners = corner_count / 2;
		corner_indices.resize(kept_corners);
		normals_binding->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, corner_indices, 4, false));
		const Ref<ArrayTetraMesh4D> imported = surface->import_generate_tetra_mesh_surface(state, shared_vertices);
		REQUIRE(imported.is_valid());
		CHECK(imported->is_mesh_data_valid());
		const PackedInt32Array imported_indices = imported->get_simplex_cell_normal_indices();
		const PackedVector4Array imported_values = imported->get_normal_values();
		REQUIRE(imported_indices.size() == corner_count);
		const PackedVector4Array source_values = source->get_normal_values();
		for (int64_t i = 0; i < corner_count; i++) {
			if (i < kept_corners) {
				CHECK(imported_values[imported_indices[i]] == source_values[corner_indices[i]]);
			} else {
				CHECK(imported_values[imported_indices[i]] == Vector4());
			}
		}
	}

	SUBCASE("A short boundary cell vertex binding is padded with cells that have no data") {
		const Ref<G4MFMeshSurfaceBindingGeometry4D> cell_vertex_binding = find_geometry_binding(normals_binding, 3, 0);
		REQUIRE(cell_vertex_binding.is_valid());
		const int64_t cell_count = source->get_poly_cell_indices()[1].size();
		REQUIRE(cell_count == 8);
		const PackedInt32Array packed = cell_vertex_binding->load_indices(state);
		// Each packed record is a member count followed by the members, so walk the records to cut between them.
		int64_t cut = 0;
		const int64_t kept_cells = 5;
		for (int64_t record = 0; record < kept_cells; record++) {
			cut += 1 + packed[cut];
		}
		cell_vertex_binding->set_indices_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, packed.slice(0, cut), 1, false));
		ERR_PRINT_OFF; // Validation samples the simplex normals, and the padded cells intentionally have none.
		const Ref<ArrayPolyMesh4D> imported = surface->import_generate_poly_mesh_surface(state, shared_vertices);
		REQUIRE(imported.is_valid());
		CHECK(imported->is_mesh_data_valid());
		ERR_PRINT_ON;
		const Vector<PackedInt32Array> imported_indices = imported->get_poly_cell_normal_indices();
		const Vector<PackedInt32Array> source_indices = source->get_poly_cell_normal_indices();
		REQUIRE_MESSAGE(imported_indices.size() == cell_count, "The importer should pad the binding with one empty record per missing cell.");
		for (int64_t cell = 0; cell < cell_count; cell++) {
			if (cell < kept_cells) {
				CHECK(imported_indices[cell] == source_indices[cell]);
			} else {
				CHECK(imported_indices[cell].is_empty());
			}
		}
	}

	SUBCASE("More cell records than boundary cells are rejected") {
		const Ref<G4MFMeshSurfaceBindingGeometry4D> cell_vertex_binding = find_geometry_binding(normals_binding, 3, 0);
		REQUIRE(cell_vertex_binding.is_valid());
		PackedInt32Array long_packed = cell_vertex_binding->load_indices(state);
		long_packed.append(0); // One extra record with zero members.
		cell_vertex_binding->set_indices_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, long_packed, 1, false));
		ERR_PRINT_OFF;
		const Ref<ArrayPolyMesh4D> imported = surface->import_generate_poly_mesh_surface(state, shared_vertices);
		ERR_PRINT_ON;
		CHECK(imported.is_null());
	}

	SUBCASE("A short boundary normals binding is padded and the missing normals are recalculated") {
		const Ref<G4MFMeshSurfaceBindingGeometry4D> boundary_normals_binding = find_geometry_binding(normals_binding, 3, 3);
		REQUIRE(boundary_normals_binding.is_valid());
		PackedInt32Array indices = boundary_normals_binding->load_indices(state);
		REQUIRE(indices.size() == source->get_poly_cell_indices()[1].size());
		indices.resize(indices.size() / 2);
		boundary_normals_binding->set_indices_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, indices, 1, false));
		const Ref<ArrayPolyMesh4D> imported = surface->import_generate_poly_mesh_surface(state, shared_vertices);
		REQUIRE(imported.is_valid());
		CHECK(imported->is_mesh_data_valid());
		// The box's exported normals came from its cell orientations, so recalculating the missing half reproduces them.
		CHECK(imported->get_poly_cell_boundary_normals() == source->get_poly_cell_boundary_normals());
	}

	SUBCASE("Simplex corner bindings with a partial simplex or too many corners are rejected") {
		REQUIRE(normals_binding->get_simplexes_accessor_index() >= 0);
		const PackedInt32Array corner_indices = normals_binding->load_simplex_indices(state);
		const int64_t corner_count = source->get_simplex_cell_vertex_indices().size();
		REQUIRE(corner_indices.size() == corner_count);
		PackedInt32Array partial_simplex = corner_indices;
		partial_simplex.resize(corner_count - 1);
		normals_binding->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, partial_simplex, 1, false));
		ERR_PRINT_OFF;
		CHECK(surface->import_generate_tetra_mesh_surface(state, shared_vertices).is_null());
		ERR_PRINT_ON;
		PackedInt32Array extra_simplex = corner_indices;
		for (int i = 0; i < 4; i++) {
			extra_simplex.append(0);
		}
		normals_binding->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, extra_simplex, 4, false));
		ERR_PRINT_OFF;
		CHECK(surface->import_generate_tetra_mesh_surface(state, shared_vertices).is_null());
		ERR_PRINT_ON;
	}
}

TEST_CASE("[G4MFMeshSurface4D] Surface vertices are appended to the shared vertices and remapped") {
	// The third vertex duplicates the first, so deduplication merges them and remaps the second edge onto the first vertex.
	Ref<ArrayWireMesh4D> source;
	source.instantiate();
	source->set_vertex_positions({ Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 0, 0, 0) });
	source->set_edge_indices({ 0, 1, 1, 2 });
	REQUIRE(source->is_mesh_data_valid());
	for (const bool deduplicate : { false, true }) {
		CAPTURE(deduplicate);
		Ref<G4MFState4D> state;
		state.instantiate();
		// Pretend another surface already contributed a vertex, so the remapping has a non-zero offset to get right.
		PackedVector4Array shared_vertices = { Vector4(9, 9, 9, 9) };
		const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(state, source, shared_vertices, deduplicate);
		REQUIRE(surface.is_valid());
		if (deduplicate) {
			CHECK(shared_vertices == PackedVector4Array({ Vector4(9, 9, 9, 9), Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0) }));
			CHECK(surface->load_edge_indices(state) == PackedInt32Array({ 1, 2, 2, 1 }));
		} else {
			CHECK(shared_vertices == PackedVector4Array({ Vector4(9, 9, 9, 9), Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 0, 0, 0) }));
			CHECK(surface->load_edge_indices(state) == PackedInt32Array({ 1, 2, 2, 3 }));
		}
		// Importing against the shared array gives back a valid wire mesh with both edges.
		const Ref<ArrayWireMesh4D> imported = surface->import_generate_wire_mesh_surface(state, shared_vertices);
		REQUIRE(imported.is_valid());
		CHECK(imported->is_mesh_data_valid());
		CHECK(imported->get_vertex_positions() == shared_vertices);
		CHECK(imported->get_edge_indices().size() == 4);
	}
}
} // namespace TestG4MFMeshSurface4D
