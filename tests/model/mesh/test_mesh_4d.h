#pragma once

#include "../../../model/mesh/poly/array_poly_mesh_4d.h"
#include "../../../model/mesh/poly/box_poly_mesh_4d.h"
#include "../../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../../model/mesh/tetra/box_tetra_mesh_4d.h"
#include "../../../model/mesh/wire/array_wire_mesh_4d.h"
#include "../../../model/mesh/wire/box_wire_mesh_4d.h"

#include "tests/test_macros.h"

namespace TestMesh4D {
static const char *VALIDATION_RESET = "mesh_data_validation_reset";
static const char *PROXY_DIRTY = "proxy_mesh_3d_marked_dirty";

// Both signals have no arguments, so each emission is recorded as an empty argument list.
static Array _emissions(const int p_count) {
	Array emissions;
	for (int i = 0; i < p_count; i++) {
		emissions.push_back(Array());
	}
	return emissions;
}

static void _watch_signals(Mesh4D *p_mesh) {
	SIGNAL_WATCH(p_mesh, VALIDATION_RESET);
	SIGNAL_WATCH(p_mesh, PROXY_DIRTY);
}

static void _unwatch_signals(Mesh4D *p_mesh) {
	SIGNAL_UNWATCH(p_mesh, VALIDATION_RESET);
	SIGNAL_UNWATCH(p_mesh, PROXY_DIRTY);
}

static Ref<ArrayWireMesh4D> _make_wire_mesh() {
	Ref<ArrayWireMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertex_positions({ Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0) });
	mesh->set_edge_indices({ 0, 1 });
	return mesh;
}

static Ref<ArrayTetraMesh4D> _make_tetra_mesh() {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertex_positions({ Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0) });
	mesh->set_simplex_cell_vertex_indices({ 0, 1, 2, 3 });
	return mesh;
}

static Ref<ArrayPolyMesh4D> _make_poly_mesh() {
	Ref<BoxPolyMesh4D> box;
	box.instantiate();
	return box->to_array_poly_mesh();
}

static PackedStringArray signal_order;
static void _record_validation_reset() {
	signal_order.push_back(VALIDATION_RESET);
}
static void _record_proxy_dirty() {
	signal_order.push_back(PROXY_DIRTY);
}

TEST_CASE("[Mesh4D] Resetting validation also marks the proxy mesh dirty") {
	Ref<ArrayWireMesh4D> mesh = _make_wire_mesh();
	_watch_signals(mesh.ptr());
	mesh->reset_mesh_data_validation();
	SIGNAL_CHECK(VALIDATION_RESET, _emissions(1));
	SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
	_unwatch_signals(mesh.ptr());
}

TEST_CASE("[Mesh4D] Validation reset is emitted before the proxy mesh is marked dirty") {
	Ref<ArrayWireMesh4D> mesh = _make_wire_mesh();
	signal_order.clear();
	mesh->connect(VALIDATION_RESET, callable_mp_static(&_record_validation_reset));
	mesh->connect(PROXY_DIRTY, callable_mp_static(&_record_proxy_dirty));
	mesh->set_vertex_positions({ Vector4(0, 0, 0, 0), Vector4(2, 0, 0, 0) });
	CHECK_MESSAGE(signal_order == PackedStringArray({ VALIDATION_RESET, PROXY_DIRTY }), "Listeners that care about validity should be notified before listeners that only rebuild proxy meshes.");
	mesh->disconnect(VALIDATION_RESET, callable_mp_static(&_record_validation_reset));
	mesh->disconnect(PROXY_DIRTY, callable_mp_static(&_record_proxy_dirty));
}

TEST_CASE("[Mesh4D] Structural changes emit both signals exactly once") {
	SUBCASE("ArrayWireMesh4D") {
		Ref<ArrayWireMesh4D> mesh = _make_wire_mesh();
		_watch_signals(mesh.ptr());
		mesh->set_vertex_positions({ Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) });
		SIGNAL_CHECK(VALIDATION_RESET, _emissions(1));
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		mesh->append_edge_indices(1, 2);
		SIGNAL_CHECK(VALIDATION_RESET, _emissions(1));
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		mesh->append_vertex(Vector4(0, 0, 1, 0));
		SIGNAL_CHECK(VALIDATION_RESET, _emissions(1));
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		mesh->subdivide_edges(2);
		SIGNAL_CHECK(VALIDATION_RESET, _emissions(1));
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		_unwatch_signals(mesh.ptr());
	}
	SUBCASE("ArrayTetraMesh4D") {
		Ref<ArrayTetraMesh4D> mesh = _make_tetra_mesh();
		_watch_signals(mesh.ptr());
		mesh->set_simplex_cell_vertex_indices({ 0, 2, 1, 3 });
		SIGNAL_CHECK(VALIDATION_RESET, _emissions(1));
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		mesh->set_normal_values({ Vector4(0, 0, 0, 1) });
		SIGNAL_CHECK(VALIDATION_RESET, _emissions(1));
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		mesh->append_vertex(Vector4(0, 0, 0, 1));
		SIGNAL_CHECK(VALIDATION_RESET, _emissions(1));
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		_unwatch_signals(mesh.ptr());
	}
	SUBCASE("ArrayPolyMesh4D") {
		Ref<ArrayPolyMesh4D> mesh = _make_poly_mesh();
		_watch_signals(mesh.ptr());
		mesh->set_poly_cell_vertex_positions(mesh->get_poly_cell_vertex_positions());
		SIGNAL_CHECK(VALIDATION_RESET, _emissions(1));
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		mesh->set_poly_cell_normal_values(mesh->get_poly_cell_normal_values());
		SIGNAL_CHECK(VALIDATION_RESET, _emissions(1));
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		mesh->append_vertex(Vector4(5, 5, 5, 5));
		SIGNAL_CHECK(VALIDATION_RESET, _emissions(1));
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		_unwatch_signals(mesh.ptr());
	}
}

TEST_CASE("[Mesh4D] Transforming the mesh marks the proxy dirty without resetting validation") {
	const Transform4D translation = Transform4D(Basis4D(), Vector4(10, 0, 0, 0));
	SUBCASE("ArrayWireMesh4D") {
		Ref<ArrayWireMesh4D> mesh = _make_wire_mesh();
		REQUIRE(mesh->is_mesh_data_valid());
		const Rect4 bounds_before = mesh->get_rect_bounds();
		_watch_signals(mesh.ptr());
		mesh->transform_mesh(translation);
		SIGNAL_CHECK_FALSE(VALIDATION_RESET);
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		CHECK_MESSAGE(mesh->get_rect_bounds().get_end().x == doctest::Approx(bounds_before.get_end().x + 10.0), "Transforming the mesh should mark the rect bounds dirty.");
		_unwatch_signals(mesh.ptr());
	}
	SUBCASE("ArrayTetraMesh4D") {
		Ref<ArrayTetraMesh4D> mesh = _make_tetra_mesh();
		REQUIRE(mesh->is_mesh_data_valid());
		const Rect4 bounds_before = mesh->get_rect_bounds();
		_watch_signals(mesh.ptr());
		mesh->transform_mesh(translation);
		SIGNAL_CHECK_FALSE(VALIDATION_RESET);
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		CHECK_MESSAGE(mesh->get_rect_bounds().get_end().x == doctest::Approx(bounds_before.get_end().x + 10.0), "Transforming the mesh should mark the rect bounds dirty.");
		_unwatch_signals(mesh.ptr());
	}
	SUBCASE("ArrayPolyMesh4D") {
		Ref<ArrayPolyMesh4D> mesh = _make_poly_mesh();
		REQUIRE(mesh->is_mesh_data_valid());
		const Rect4 bounds_before = mesh->get_rect_bounds();
		_watch_signals(mesh.ptr());
		mesh->transform_mesh(translation);
		SIGNAL_CHECK_FALSE(VALIDATION_RESET);
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		CHECK_MESSAGE(mesh->get_rect_bounds().get_end().x == doctest::Approx(bounds_before.get_end().x + 10.0), "Transforming the mesh should mark the rect bounds dirty.");
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "Transforming the mesh should not reset the poly mesh validation either.");
		_unwatch_signals(mesh.ptr());
	}
}

TEST_CASE("[Mesh4D] Appending only duplicate vertices emits nothing") {
	SUBCASE("ArrayWireMesh4D") {
		Ref<ArrayWireMesh4D> mesh = _make_wire_mesh();
		_watch_signals(mesh.ptr());
		mesh->append_vertices(mesh->get_vertex_positions(), true);
		mesh->append_vertices(PackedVector4Array(), true);
		SIGNAL_CHECK_FALSE(VALIDATION_RESET);
		SIGNAL_CHECK_FALSE(PROXY_DIRTY);
		_unwatch_signals(mesh.ptr());
	}
	SUBCASE("ArrayTetraMesh4D") {
		Ref<ArrayTetraMesh4D> mesh = _make_tetra_mesh();
		_watch_signals(mesh.ptr());
		mesh->append_vertices(mesh->get_vertex_positions(), true);
		mesh->append_vertices(PackedVector4Array(), true);
		SIGNAL_CHECK_FALSE(VALIDATION_RESET);
		SIGNAL_CHECK_FALSE(PROXY_DIRTY);
		_unwatch_signals(mesh.ptr());
	}
	SUBCASE("ArrayPolyMesh4D") {
		Ref<ArrayPolyMesh4D> mesh = _make_poly_mesh();
		_watch_signals(mesh.ptr());
		mesh->append_vertices(mesh->get_poly_cell_vertex_positions(), true);
		mesh->append_vertices(PackedVector4Array(), true);
		SIGNAL_CHECK_FALSE(VALIDATION_RESET);
		SIGNAL_CHECK_FALSE(PROXY_DIRTY);
		_unwatch_signals(mesh.ptr());
	}
}

TEST_CASE("[ArrayWireMesh4D] Appending a vertex updates the rect bounds") {
	Ref<ArrayWireMesh4D> mesh = _make_wire_mesh();
	CHECK(mesh->get_rect_bounds().get_end() == Vector4(1, 0, 0, 0));
	mesh->append_vertex(Vector4(0, 5, 0, 0));
	CHECK_MESSAGE(mesh->get_rect_bounds().get_end() == Vector4(1, 5, 0, 0), "A vertex without any edges still counts towards the rect bounds.");
}

TEST_CASE("[Mesh4D] Primitive size changes mark bounds and proxy dirty without resetting validation") {
	SUBCASE("BoxWireMesh4D") {
		Ref<BoxWireMesh4D> mesh;
		mesh.instantiate();
		REQUIRE(mesh->is_mesh_data_valid());
		_watch_signals(mesh.ptr());
		mesh->set_size(Vector4(2, 4, 6, 8));
		SIGNAL_CHECK_FALSE(VALIDATION_RESET);
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		CHECK(mesh->get_rect_bounds().get_end() == Vector4(1, 2, 3, 4));
		_unwatch_signals(mesh.ptr());
	}
	SUBCASE("BoxTetraMesh4D") {
		Ref<BoxTetraMesh4D> mesh;
		mesh.instantiate();
		REQUIRE(mesh->is_mesh_data_valid());
		_watch_signals(mesh.ptr());
		mesh->set_size(Vector4(2, 4, 6, 8));
		SIGNAL_CHECK_FALSE(VALIDATION_RESET);
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		CHECK(mesh->get_rect_bounds().get_end() == Vector4(1, 2, 3, 4));
		_unwatch_signals(mesh.ptr());
	}
	SUBCASE("BoxPolyMesh4D") {
		Ref<BoxPolyMesh4D> mesh;
		mesh.instantiate();
		REQUIRE(mesh->is_mesh_data_valid());
		_watch_signals(mesh.ptr());
		mesh->set_size(Vector4(2, 4, 6, 8));
		SIGNAL_CHECK_FALSE(VALIDATION_RESET);
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		CHECK(mesh->get_rect_bounds().get_end() == Vector4(1, 2, 3, 4));
		_unwatch_signals(mesh.ptr());
	}
}
} // namespace TestMesh4D
