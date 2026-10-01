#include "voxel_world_4d.h"

#include "edit/voxel_edit.h"

void VoxelWorld4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_voxel_data"), &VoxelWorld4D::get_voxel_data);
	ClassDB::bind_method(D_METHOD("apply_edit", "edit"), &VoxelWorld4D::apply_edit);

	ClassDB::bind_method(D_METHOD("get_generator"), &VoxelWorld4D::get_generator);
	ClassDB::bind_method(D_METHOD("set_generator", "generator"), &VoxelWorld4D::set_generator);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "generator", PROPERTY_HINT_RESOURCE_TYPE, "VoxelGenerator"), "set_generator", "get_generator");

	ClassDB::bind_method(D_METHOD("get_material_palette"), &VoxelWorld4D::get_material_palette);
	ClassDB::bind_method(D_METHOD("set_material_palette", "material_palette"), &VoxelWorld4D::set_material_palette);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material_palette", PROPERTY_HINT_RESOURCE_TYPE, "VoxelMaterialPalette"), "set_material_palette", "get_material_palette");

	ClassDB::bind_method(D_METHOD("get_allow_projection"), &VoxelWorld4D::get_allow_projection);
	ClassDB::bind_method(D_METHOD("set_allow_projection", "allow_projection"), &VoxelWorld4D::set_allow_projection);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_projection"), "set_allow_projection", "get_allow_projection");

	ClassDB::bind_method(D_METHOD("is_world_bounds_enabled"), &VoxelWorld4D::is_world_bounds_enabled);
	ClassDB::bind_method(D_METHOD("set_world_bounds_enabled", "enabled"), &VoxelWorld4D::set_world_bounds_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "world_bounds_enabled"), "set_world_bounds_enabled", "is_world_bounds_enabled");

	ClassDB::bind_method(D_METHOD("get_world_bounds_position"), &VoxelWorld4D::get_world_bounds_position);
	ClassDB::bind_method(D_METHOD("set_world_bounds_position", "position"), &VoxelWorld4D::set_world_bounds_position);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4I, "world_bounds_position"), "set_world_bounds_position", "get_world_bounds_position");

	ClassDB::bind_method(D_METHOD("get_world_bounds_size"), &VoxelWorld4D::get_world_bounds_size);
	ClassDB::bind_method(D_METHOD("set_world_bounds_size", "size"), &VoxelWorld4D::set_world_bounds_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4I, "world_bounds_size"), "set_world_bounds_size", "get_world_bounds_size");
}

void VoxelWorld4D::_validate_property(PropertyInfo &p_property) const {
	if (!_world_bounds_enabled && (p_property.name == StringName("world_bounds_position") || p_property.name == StringName("world_bounds_size"))) {
		p_property.usage = PROPERTY_USAGE_NO_EDITOR;
	}
}

void VoxelWorld4D::set_material_palette(const Ref<VoxelMaterialPalette> &p_material_palette) {
	if (_material_palette == p_material_palette) {
		return;
	}
	const Callable update_callable = callable_mp(this, &VoxelWorld4D::_update_material_texture);
	if (_material_palette.is_valid()) {
		_material_palette->disconnect("changed", update_callable);
	}
	_material_palette = p_material_palette;
	if (_material_palette.is_valid()) {
		_material_palette->connect("changed", update_callable);
	}
	_update_material_texture();
}

void VoxelWorld4D::set_allow_projection(const bool p_allow_projection) {
	if (_allow_projection == p_allow_projection) {
		return;
	}
	_allow_projection = p_allow_projection;
	_mesh_handler.update_chunk_allow_projection();
}

void VoxelWorld4D::_update_material_texture() {
	_mesh_material->set_albedo_texture_3d(VoxelMaterialPalette::bake_texture(_material_palette));
}

void VoxelWorld4D::apply_edit(const Ref<VoxelEdit> &p_edit) {
	ERR_FAIL_COND(p_edit.is_null());
	_voxel_data->apply_edit(p_edit);
	_mesh_handler.mark_region_dirty(p_edit->get_bounds());
}

void VoxelWorld4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			// Generally the world will be empty at this point, but just in case it isn't:
			_mesh_handler.mark_region_dirty(_voxel_data->get_bounds());
			set_process(true);
		} break;
		case NOTIFICATION_PROCESS: {
			_chunk_loader->update_loaded_chunks();
			_voxel_data->merge_edited_constants();
			_mesh_handler.update_dirty_meshes();
		} break;
	}
}

VoxelWorld4D::VoxelWorld4D() :
		_mesh_handler(this) {
	_voxel_data.instantiate();
	_chunk_loader = memnew(VoxelChunkLoader(this));
	_mesh_material.instantiate();
	_mesh_material->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_TEXTURE3D_CELL_UVW_ONLY);
	_update_material_texture();
}

VoxelWorld4D::~VoxelWorld4D() {
	memdelete(_chunk_loader);
}
