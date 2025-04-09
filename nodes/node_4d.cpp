#include "node_4d.h"

// Rotation edit mode.

Node4D::RotationEditMode Node4D::get_rotation_edit_mode() const {
	return _rotation_edit_mode;
}

void Node4D::set_rotation_edit_mode(const RotationEditMode p_rotation_edit_mode) {
	_rotation_edit_mode = p_rotation_edit_mode;
	notify_property_list_changed();
}

// Transform altering methods.

void Node4D::apply_scale(const Vector4 &p_amount) {
	_transform.scale_global(p_amount);
}

void Node4D::translate_local(const Vector4 &p_amount) {
	_transform.translate_local(p_amount);
}

void Node4D::rotate_euler(const Euler4D &p_euler) {
	Basis4D rot = p_euler.to_basis();
	set_basis(rot * get_basis());
}

void Node4D::rotate_euler_bind(const AABB &p_euler) {
	rotate_euler(p_euler);
}

void Node4D::rotate_euler_local(const Euler4D &p_euler_local) {
	Basis4D rot = p_euler_local.to_basis();
	set_basis(get_basis() * rot);
}

void Node4D::rotate_euler_local_bind(const AABB &p_euler_local) {
	rotate_euler_local(p_euler_local);
}

// Geometric algebra rotation altering methods.

void Node4D::rotate_bivector_magnitude(const Bivector4D &p_bivector) {
	Rotor4D rot = Rotor4D::from_bivector_magnitude(p_bivector);
	set_basis(rot.rotate_basis(get_basis()));
}

void Node4D::rotate_bivector_magnitude_bind(const AABB &p_bivector) {
	rotate_bivector_magnitude(p_bivector);
}

void Node4D::rotate_bivector_magnitude_local(const Bivector4D &p_bivector_local) {
	Basis4D rot = Rotor4D::from_bivector_magnitude(p_bivector_local).get_rotation_basis();
	set_basis(get_basis() * rot);
}

void Node4D::rotate_bivector_magnitude_local_bind(const AABB &p_bivector_local) {
	rotate_bivector_magnitude_local(p_bivector_local);
}

void Node4D::rotate_rotor(const Rotor4D &p_rotor) {
	set_basis(p_rotor.rotate_basis(get_basis()));
}

void Node4D::rotate_rotor_bind(const Ref<godot_4d_bind::Rotor4D> &p_rotor) {
	rotate_rotor(p_rotor->get_rotor());
}

void Node4D::rotate_rotor_local(const Rotor4D &p_rotor_local) {
	set_basis(get_basis() * p_rotor_local.get_rotation_basis());
}

void Node4D::rotate_rotor_local_bind(const Ref<godot_4d_bind::Rotor4D> &p_rotor_local) {
	rotate_rotor_local(p_rotor_local->get_rotor());
}

// Local transform and basis.

Transform4D Node4D::get_transform() const {
	return _transform;
}

void Node4D::set_transform(const Transform4D &p_transform) {
	_transform = p_transform;
#ifdef CACHE_ROTATION_AND_SCALE
	_euler_cache_dirty = true;
	_scale_cache_dirty = true;
#endif // CACHE_ROTATION_AND_SCALE
}

PackedRealArray Node4D::get_transform_array() const {
	return _transform.to_array();
}

void Node4D::set_transform_array(const PackedRealArray &p_transform_array) {
	_transform = Transform4D::from_array(p_transform_array);
#ifdef CACHE_ROTATION_AND_SCALE
	_euler_cache_dirty = true;
	_scale_cache_dirty = true;
#endif // CACHE_ROTATION_AND_SCALE
}

Ref<godot_4d_bind::Transform4D> Node4D::get_transform_bind() const {
	Ref<godot_4d_bind::Transform4D> transform_bind;
	transform_bind.instantiate();
	transform_bind->set_transform(_transform);
	return transform_bind;
}

void Node4D::set_transform_bind(const Ref<godot_4d_bind::Transform4D> &p_transform) {
	_transform = p_transform->get_transform();
}

Basis4D Node4D::get_basis() const {
	return _transform.basis;
}

void Node4D::set_basis(const Basis4D &p_basis) {
	_transform.basis = p_basis;
#ifdef CACHE_ROTATION_AND_SCALE
	_euler_cache_dirty = true;
	_scale_cache_dirty = true;
#endif // CACHE_ROTATION_AND_SCALE
}

Projection Node4D::get_basis_bind() const {
	return _transform.basis;
}

void Node4D::set_basis_bind(const Projection &p_basis) {
	_transform.basis = p_basis;
#ifdef CACHE_ROTATION_AND_SCALE
	_euler_cache_dirty = true;
	_scale_cache_dirty = true;
#endif // CACHE_ROTATION_AND_SCALE
}

// Local transform components.

Vector4 Node4D::get_position() const {
	return _transform.get_origin();
}

void Node4D::set_position(const Vector4 &p_position) {
	_transform.set_origin(p_position);
}

#ifdef CACHE_ROTATION_AND_SCALE
Euler4D Node4D::get_rotation() {
	if (_euler_cache_dirty) {
		_euler_cache = Euler4D::from_basis(_transform.basis);
		_euler_cache_dirty = false;
	}
	return _euler_cache;
}

AABB Node4D::get_rotation_bind() {
	return get_rotation();
}

Ref<godot_4d_bind::Euler4D> Node4D::get_rotation_euler_bind() {
	Ref<godot_4d_bind::Euler4D> rotation_euler_bind;
	rotation_euler_bind.instantiate();
	rotation_euler_bind->set_euler(get_rotation());
	return rotation_euler_bind;
}

Euler4D Node4D::get_rotation_degrees() {
	return get_rotation().rad_to_deg();
}

AABB Node4D::get_rotation_degrees_bind() {
	return get_rotation_degrees();
}

Ref<godot_4d_bind::Euler4D> Node4D::get_rotation_degrees_euler_bind() {
	Ref<godot_4d_bind::Euler4D> rotation_degrees_euler_bind;
	rotation_degrees_euler_bind.instantiate();
	rotation_degrees_euler_bind->set_euler(get_rotation_degrees());
	return rotation_degrees_euler_bind;
}

Vector4 Node4D::get_scale() {
	if (_scale_cache_dirty) {
		_scale_cache = _transform.get_scale();
		_scale_cache_dirty = false;
	}
	return _scale_cache;
}
#else
Euler4D Node4D::get_rotation() const {
	return Euler4D::from_basis(_transform.basis);
}

AABB Node4D::get_rotation_bind() const {
	return get_rotation();
}

Ref<godot_4d_bind::Euler4D> Node4D::get_rotation_euler_bind() const {
	Ref<godot_4d_bind::Euler4D> rotation_euler_bind;
	rotation_euler_bind.instantiate();
	rotation_euler_bind->set_euler(get_rotation());
	return rotation_euler_bind;
}

Euler4D Node4D::get_rotation_degrees() const {
	return get_rotation().rad_to_deg();
}

AABB Node4D::get_rotation_degrees_bind() const {
	return get_rotation_degrees();
}

Ref<godot_4d_bind::Euler4D> Node4D::get_rotation_degrees_euler_bind() const {
	Ref<godot_4d_bind::Euler4D> rotation_degrees_euler_bind;
	rotation_degrees_euler_bind.instantiate();
	rotation_degrees_euler_bind->set_euler(get_rotation_degrees());
	return rotation_degrees_euler_bind;
}

Vector4 Node4D::get_scale() const {
	return _transform.get_scale();
}
#endif // CACHE_ROTATION_AND_SCALE

real_t Node4D::get_uniform_scale() const {
	return _transform.basis.get_uniform_scale();
}

void Node4D::set_rotation(const Euler4D &p_euler) {
#ifdef CACHE_ROTATION_AND_SCALE
	_euler_cache = p_euler;
	if (_scale_cache_dirty) {
		_scale_cache = _transform.basis.get_scale();
		_scale_cache_dirty = false;
	}
	_transform.basis = p_euler.to_basis().scaled_local(_scale_cache);
#else
	Vector4 scale = _transform.basis.get_scale();
	_transform.basis = p_euler.to_basis().scaled_local(scale);
#endif // CACHE_ROTATION_AND_SCALE
}

void Node4D::set_rotation_bind(const AABB &p_euler) {
	set_rotation(p_euler);
}

void Node4D::set_rotation_euler_bind(const Ref<godot_4d_bind::Euler4D> &p_euler) {
	set_rotation(p_euler->get_euler());
}

void Node4D::set_rotation_degrees(const Euler4D &p_euler) {
	set_rotation(p_euler.deg_to_rad());
}

void Node4D::set_rotation_degrees_bind(const AABB &p_euler) {
	set_rotation_degrees(p_euler);
}

void Node4D::set_rotation_degrees_euler_bind(const Ref<godot_4d_bind::Euler4D> &p_euler) {
	set_rotation_degrees(p_euler->get_euler());
}

void Node4D::set_scale(const Vector4 &p_scale) {
#ifdef CACHE_ROTATION_AND_SCALE
	_scale_cache = p_scale;
#endif // CACHE_ROTATION_AND_SCALE
	_transform.set_scale(p_scale);
}

void Node4D::set_uniform_scale(const real_t p_uniform_scale) {
#ifdef CACHE_ROTATION_AND_SCALE
	const real_t scale_abs = ABS(p_uniform_scale);
	_scale_cache = Vector4(scale_abs, scale_abs, scale_abs, p_uniform_scale);
#endif // CACHE_ROTATION_AND_SCALE
	_transform.basis.set_uniform_scale(p_uniform_scale);
}

// Geometric algebra local rotation properties.

void Node4D::set_rotation_bivector_magnitude(const Bivector4D &p_bivector) {
	const Vector4 scale = get_scale();
	Rotor4D rot = Rotor4D::from_bivector_magnitude(p_bivector);
	set_basis(rot.get_rotation_basis().scaled_local(scale));
}

void Node4D::set_rotation_bivector_magnitude_bind(const AABB &p_bivector) {
	set_rotation_bivector_magnitude(p_bivector);
}

void Node4D::set_rotation_rotor(const Rotor4D &p_rotor) {
	const Vector4 scale = get_scale();
	set_basis(p_rotor.get_rotation_basis().scaled_local(scale));
}

void Node4D::set_rotation_rotor_bind(const Ref<godot_4d_bind::Rotor4D> &p_rotor) {
	set_rotation_rotor(p_rotor->get_rotor());
}

// Global transform and basis.

Transform4D Node4D::get_global_transform() const {
	Node4D *node_4d_parent = Object::cast_to<Node4D>(get_parent());
	if (node_4d_parent) {
		return node_4d_parent->get_global_transform() * _transform;
	} else {
		return _transform;
	}
}

void Node4D::set_global_transform(const Transform4D &p_transform) {
	Node4D *node_4d_parent = Object::cast_to<Node4D>(get_parent());
	if (node_4d_parent) {
		set_transform(node_4d_parent->get_global_transform().inverse() * p_transform);
	} else {
		set_transform(p_transform);
	}
}

PackedRealArray Node4D::get_global_transform_array() const {
	return get_global_transform().to_array();
}

void Node4D::set_global_transform_array(const PackedRealArray &p_transform_array) {
	set_global_transform(Transform4D::from_array(p_transform_array));
}

Ref<godot_4d_bind::Transform4D> Node4D::get_global_transform_bind() const {
	Ref<godot_4d_bind::Transform4D> transform_bind;
	transform_bind.instantiate();
	transform_bind->set_transform(get_global_transform());
	return transform_bind;
}

void Node4D::set_global_transform_bind(const Ref<godot_4d_bind::Transform4D> &p_transform) {
	set_global_transform(p_transform->get_transform());
}

Basis4D Node4D::get_global_basis() const {
	Node4D *node_4d_parent = Object::cast_to<Node4D>(get_parent());
	if (node_4d_parent) {
		return node_4d_parent->get_global_basis() * _transform.basis;
	} else {
		return _transform.basis;
	}
}

void Node4D::set_global_basis(const Basis4D &p_basis) {
	Node4D *node_4d_parent = Object::cast_to<Node4D>(get_parent());
	if (node_4d_parent) {
		set_basis(node_4d_parent->get_global_basis().inverse() * p_basis);
	} else {
		set_basis(p_basis);
	}
}

Projection Node4D::get_global_basis_bind() const {
	return get_global_basis();
}

void Node4D::set_global_basis_bind(const Projection &p_basis) {
	set_global_basis(p_basis);
}

// Global transform components.

Vector4 Node4D::get_global_position() const {
	Vector4 position = _transform.get_origin();
	Node4D *node_4d_parent = Object::cast_to<Node4D>(get_parent());
	while (node_4d_parent) {
		position = node_4d_parent->get_transform().xform(position);
		node_4d_parent = Object::cast_to<Node4D>(node_4d_parent->get_parent());
	}
	return position;
}

void Node4D::set_global_position(const Vector4 &p_global_position) {
	Node4D *node_4d_parent = Object::cast_to<Node4D>(get_parent());
	if (node_4d_parent) {
		set_position(node_4d_parent->get_transform().xform_inv(p_global_position));
	} else {
		set_position(p_global_position);
	}
}

#ifdef CACHE_ROTATION_AND_SCALE
Euler4D Node4D::get_global_rotation() {
	Node4D *node_4d_parent = Object::cast_to<Node4D>(get_parent());
	if (node_4d_parent) {
		return Euler4D::from_basis(get_global_basis());
	} else {
		return get_rotation();
	}
}

AABB Node4D::get_global_rotation_bind() {
	return get_global_rotation();
}

Ref<godot_4d_bind::Euler4D> Node4D::get_global_rotation_euler_bind() {
	Ref<godot_4d_bind::Euler4D> rotation_euler_bind;
	rotation_euler_bind.instantiate();
	rotation_euler_bind->set_euler(get_global_rotation());
	return rotation_euler_bind;
}

Euler4D Node4D::get_global_rotation_degrees() {
	return get_global_rotation().rad_to_deg();
}

AABB Node4D::get_global_rotation_degrees_bind() {
	return get_global_rotation_degrees();
}

Ref<godot_4d_bind::Euler4D> Node4D::get_global_rotation_degrees_euler_bind() {
	Ref<godot_4d_bind::Euler4D> rotation_degrees_euler_bind;
	rotation_degrees_euler_bind.instantiate();
	rotation_degrees_euler_bind->set_euler(get_global_rotation_degrees());
	return rotation_degrees_euler_bind;
}

Vector4 Node4D::get_global_scale() {
	Node4D *node_4d_parent = Object::cast_to<Node4D>(get_parent());
	if (node_4d_parent) {
		return get_global_basis().get_scale();
	} else {
		return get_scale();
	}
}
#else
Euler4D Node4D::get_global_rotation() const {
	return Euler4D::from_basis(get_global_basis());
}

AABB Node4D::get_global_rotation_bind() const {
	return get_global_rotation();
}

Ref<godot_4d_bind::Euler4D> Node4D::get_global_rotation_euler_bind() const {
	Ref<godot_4d_bind::Euler4D> rotation_euler_bind;
	rotation_euler_bind.instantiate();
	rotation_euler_bind->set_euler(get_global_rotation());
	return rotation_euler_bind;
}

Euler4D Node4D::get_global_rotation_degrees() const {
	return get_global_rotation().rad_to_deg();
}

AABB Node4D::get_global_rotation_degrees_bind() const {
	return get_global_rotation_degrees();
}

Ref<godot_4d_bind::Euler4D> Node4D::get_global_rotation_degrees_euler_bind() const {
	Ref<godot_4d_bind::Euler4D> rotation_degrees_euler_bind;
	rotation_degrees_euler_bind.instantiate();
	rotation_degrees_euler_bind->set_euler(get_global_rotation_degrees());
	return rotation_degrees_euler_bind;
}

Vector4 Node4D::get_global_scale() const {
	return get_global_basis().get_scale();
}
#endif // CACHE_ROTATION_AND_SCALE

real_t Node4D::get_global_uniform_scale() const {
	return get_global_basis().get_uniform_scale();
}

void Node4D::set_global_rotation(const Euler4D &p_global_euler) {
	Node4D *node_4d_parent = Object::cast_to<Node4D>(get_parent());
	if (node_4d_parent) {
		set_basis(node_4d_parent->get_global_basis().inverse() * p_global_euler.to_basis());
	} else {
		set_rotation(p_global_euler);
	}
}

void Node4D::set_global_rotation_bind(const AABB &p_global_euler) {
	set_global_rotation(p_global_euler);
}

void Node4D::set_global_rotation_euler_bind(const Ref<godot_4d_bind::Euler4D> &p_global_euler) {
	set_global_rotation(p_global_euler->get_euler());
}

void Node4D::set_global_rotation_degrees(const Euler4D &p_global_euler) {
	set_global_rotation(p_global_euler.deg_to_rad());
}

void Node4D::set_global_rotation_degrees_bind(const AABB &p_global_euler) {
	set_global_rotation_degrees(p_global_euler);
}

void Node4D::set_global_rotation_degrees_euler_bind(const Ref<godot_4d_bind::Euler4D> &p_global_euler) {
	set_global_rotation_degrees(p_global_euler->get_euler());
}

void Node4D::set_global_scale(const Vector4 &p_global_scale) {
	Basis4D global_basis = get_global_basis();
	global_basis.set_scale(p_global_scale);
	set_global_transform(global_basis);
}

void Node4D::set_global_uniform_scale(const real_t p_global_uniform_scale) {
	Basis4D global_basis = get_global_basis();
	global_basis.set_uniform_scale(p_global_uniform_scale);
	set_global_basis(global_basis);
}

// Geometric algebra global rotation properties.

void Node4D::set_global_rotation_bivector_magnitude(const Bivector4D &p_bivector) {
	const Vector4 global_scale = get_global_scale();
	Rotor4D rot = Rotor4D::from_bivector_magnitude(p_bivector);
	set_global_basis(rot.get_rotation_basis().scaled_local(global_scale));
}

void Node4D::set_global_rotation_bivector_magnitude_bind(const AABB &p_bivector) {
	set_global_rotation_bivector_magnitude(p_bivector);
}

void Node4D::set_global_rotation_rotor(const Rotor4D &p_rotor) {
	const Vector4 global_scale = get_global_scale();
	set_global_basis(p_rotor.get_rotation_basis().scaled_local(global_scale));
}

void Node4D::set_global_rotation_rotor_bind(const Ref<godot_4d_bind::Rotor4D> &p_rotor) {
	set_global_rotation_rotor(p_rotor->get_rotor());
}

// Visibility.

bool Node4D::is_visible() const {
	return _is_visible;
}

bool Node4D::is_visible_in_tree() const {
	if (!_is_visible) {
		return false;
	}
	Node4D *node_4d_parent = Object::cast_to<Node4D>(get_parent());
	if (node_4d_parent) {
		return node_4d_parent->is_visible_in_tree();
	}
	return true;
}

void Node4D::set_visible(const bool p_visible) {
	_is_visible = p_visible;
}

void Node4D::_propagate_visibility_changed() {
	emit_signal(StringName("visibility_changed"));
	TypedArray<Node> children = get_children();
	for (int i = 0; i < children.size(); ++i) {
		Node4D *child = Object::cast_to<Node4D>(children[i]);
		if (!child || !child->is_visible()) {
			continue;
		}
		child->_propagate_visibility_changed();
	}
}

// Rect bounds.

Rect4 Node4D::get_rect_bounds(const Transform4D &p_inv_relative_to) const {
	PackedVector4Array result;
	GDVIRTUAL_CALL(_get_rect_bounds, p_inv_relative_to.basis, p_inv_relative_to.origin, result);
	if (result.size() != 2) {
		return Rect4(p_inv_relative_to * get_global_position(), Vector4());
	}
	return Rect4(result[0], result[1]);
}

PackedVector4Array Node4D::get_rect_bounds_bind(const Projection &p_inv_relative_to_basis, const Vector4 &p_inv_relative_to_origin) const {
	Rect4 bounds = get_rect_bounds(Transform4D(p_inv_relative_to_basis, p_inv_relative_to_origin));
	return PackedVector4Array({ bounds.position, bounds.size });
}

Rect4 Node4D::get_rect_bounds_recursive(const Transform4D &p_inv_relative_to) const {
	Rect4 bounds = get_rect_bounds(p_inv_relative_to);
	const int child_count = get_child_count();
	for (int i = 0; i < child_count; i++) {
		Node4D *child_4d = Object::cast_to<Node4D>(get_child(i));
		if (child_4d != nullptr) {
			bounds = bounds.merge(child_4d->get_rect_bounds_recursive(p_inv_relative_to));
		}
	}
	return bounds;
}

PackedVector4Array Node4D::get_rect_bounds_recursive_bind(const Projection &p_inv_relative_to_basis, const Vector4 &p_inv_relative_to_origin) const {
	Rect4 bounds = get_rect_bounds_recursive(Transform4D(p_inv_relative_to_basis, p_inv_relative_to_origin));
	return PackedVector4Array({ bounds.position, bounds.size });
}

// Bindings.

void Node4D::_bind_methods() {
	// Rotation edit mode.
	ClassDB::bind_method(D_METHOD("get_rotation_edit_mode"), &Node4D::get_rotation_edit_mode);
	ClassDB::bind_method(D_METHOD("set_rotation_edit_mode", "rotation_edit_mode"), &Node4D::set_rotation_edit_mode);
	// Transform altering methods.
	ClassDB::bind_method(D_METHOD("apply_scale", "ratio"), &Node4D::apply_scale);
	ClassDB::bind_method(D_METHOD("translate_local", "offset"), &Node4D::translate_local);
	ClassDB::bind_method(D_METHOD("rotate_euler", "euler"), &Node4D::rotate_euler_bind);
	ClassDB::bind_method(D_METHOD("rotate_euler_local", "euler_local"), &Node4D::rotate_euler_local_bind);
	// Geometric algebra rotation altering methods.
	ClassDB::bind_method(D_METHOD("rotate_bivector_magnitude", "bivector"), &Node4D::rotate_bivector_magnitude_bind);
	ClassDB::bind_method(D_METHOD("rotate_bivector_magnitude_local", "bivector_local"), &Node4D::rotate_bivector_magnitude_local_bind);
	ClassDB::bind_method(D_METHOD("rotate_rotor", "rotor"), &Node4D::rotate_rotor_bind);
	ClassDB::bind_method(D_METHOD("rotate_rotor_local", "rotor_local"), &Node4D::rotate_rotor_local_bind);
	// Local transform and basis.
	ClassDB::bind_method(D_METHOD("get_transform"), &Node4D::get_transform_bind);
	ClassDB::bind_method(D_METHOD("set_transform", "transform"), &Node4D::set_transform_bind);
	ClassDB::bind_method(D_METHOD("get_transform_array"), &Node4D::get_transform_array);
	ClassDB::bind_method(D_METHOD("set_transform_array", "transform_array"), &Node4D::set_transform_array);
	ClassDB::bind_method(D_METHOD("get_basis"), &Node4D::get_basis_bind);
	ClassDB::bind_method(D_METHOD("set_basis", "basis"), &Node4D::set_basis_bind);
	// Local transform components.
	ClassDB::bind_method(D_METHOD("get_position"), &Node4D::get_position);
	ClassDB::bind_method(D_METHOD("set_position", "position"), &Node4D::set_position);
	ClassDB::bind_method(D_METHOD("get_rotation"), &Node4D::get_rotation_bind);
	ClassDB::bind_method(D_METHOD("set_rotation", "euler"), &Node4D::set_rotation_bind);
	ClassDB::bind_method(D_METHOD("get_rotation_euler"), &Node4D::get_rotation_euler_bind);
	ClassDB::bind_method(D_METHOD("set_rotation_euler", "euler"), &Node4D::set_rotation_euler_bind);
	ClassDB::bind_method(D_METHOD("get_rotation_degrees"), &Node4D::get_rotation_degrees_bind);
	ClassDB::bind_method(D_METHOD("set_rotation_degrees", "euler"), &Node4D::set_rotation_degrees_bind);
	ClassDB::bind_method(D_METHOD("get_rotation_degrees_euler"), &Node4D::get_rotation_degrees_euler_bind);
	ClassDB::bind_method(D_METHOD("set_rotation_degrees_euler", "euler"), &Node4D::set_rotation_degrees_euler_bind);
	ClassDB::bind_method(D_METHOD("get_scale"), &Node4D::get_scale);
	ClassDB::bind_method(D_METHOD("set_scale", "scale"), &Node4D::set_scale);
	ClassDB::bind_method(D_METHOD("get_uniform_scale"), &Node4D::get_uniform_scale);
	ClassDB::bind_method(D_METHOD("set_uniform_scale", "uniform_scale"), &Node4D::set_uniform_scale);
	// Geometric algebra local rotation properties.
	ClassDB::bind_method(D_METHOD("set_rotation_bivector_magnitude", "bivector"), &Node4D::set_rotation_bivector_magnitude_bind);
	ClassDB::bind_method(D_METHOD("set_rotation_rotor", "rotor"), &Node4D::set_rotation_rotor_bind);
	// Global transform and basis.
	ClassDB::bind_method(D_METHOD("get_global_transform"), &Node4D::get_global_transform_bind);
	ClassDB::bind_method(D_METHOD("set_global_transform", "global_transform"), &Node4D::set_global_transform_bind);
	ClassDB::bind_method(D_METHOD("get_global_transform_array"), &Node4D::get_global_transform_array);
	ClassDB::bind_method(D_METHOD("set_global_transform_array", "global_transform_array"), &Node4D::set_global_transform_array);
	ClassDB::bind_method(D_METHOD("get_global_basis"), &Node4D::get_global_basis_bind);
	ClassDB::bind_method(D_METHOD("set_global_basis", "global_basis"), &Node4D::set_global_basis_bind);
	// Global transform components.
	ClassDB::bind_method(D_METHOD("get_global_position"), &Node4D::get_global_position);
	ClassDB::bind_method(D_METHOD("set_global_position", "global_position"), &Node4D::set_global_position);
	ClassDB::bind_method(D_METHOD("get_global_rotation"), &Node4D::get_global_rotation_bind);
	ClassDB::bind_method(D_METHOD("set_global_rotation", "global_euler"), &Node4D::set_global_rotation_bind);
	ClassDB::bind_method(D_METHOD("get_global_rotation_euler"), &Node4D::get_global_rotation_euler_bind);
	ClassDB::bind_method(D_METHOD("set_global_rotation_euler", "global_euler"), &Node4D::set_global_rotation_euler_bind);
	ClassDB::bind_method(D_METHOD("get_global_rotation_degrees"), &Node4D::get_global_rotation_degrees_bind);
	ClassDB::bind_method(D_METHOD("set_global_rotation_degrees", "global_euler"), &Node4D::set_global_rotation_degrees_bind);
	ClassDB::bind_method(D_METHOD("get_global_rotation_degrees_euler"), &Node4D::get_global_rotation_degrees_euler_bind);
	ClassDB::bind_method(D_METHOD("set_global_rotation_degrees_euler", "global_euler"), &Node4D::set_global_rotation_degrees_euler_bind);
	ClassDB::bind_method(D_METHOD("get_global_scale"), &Node4D::get_global_scale);
	ClassDB::bind_method(D_METHOD("set_global_scale", "global_scale"), &Node4D::set_global_scale);
	ClassDB::bind_method(D_METHOD("get_global_uniform_scale"), &Node4D::get_global_uniform_scale);
	ClassDB::bind_method(D_METHOD("set_global_uniform_scale", "global_uniform_scale"), &Node4D::set_global_uniform_scale);
	// Geometric algebra global rotation properties.
	ClassDB::bind_method(D_METHOD("set_global_rotation_bivector_magnitude", "global_bivector"), &Node4D::set_global_rotation_bivector_magnitude_bind);
	ClassDB::bind_method(D_METHOD("set_global_rotation_rotor", "global_rotor"), &Node4D::set_global_rotation_rotor_bind);
	// Visibility.
	ClassDB::bind_method(D_METHOD("is_visible"), &Node4D::is_visible);
	ClassDB::bind_method(D_METHOD("is_visible_in_tree"), &Node4D::is_visible_in_tree);
	ClassDB::bind_method(D_METHOD("set_visible", "visible"), &Node4D::set_visible);
	// Rect bounds.
	ClassDB::bind_method(D_METHOD("get_rect_bounds", "inv_relative_to_basis", "inv_relative_to_origin"), &Node4D::get_rect_bounds_bind, DEFVAL(Projection()), DEFVAL(Vector4()));
	ClassDB::bind_method(D_METHOD("get_rect_bounds_recursive", "inv_relative_to_basis", "inv_relative_to_origin"), &Node4D::get_rect_bounds_recursive_bind, DEFVAL(Projection()), DEFVAL(Vector4()));
	GDVIRTUAL_BIND(_get_rect_bounds, "inv_relative_to_basis", "inv_relative_to_origin");

#ifdef REAL_T_IS_DOUBLE
#define PACKED_REAL_ARRAY Variant::PACKED_FLOAT64_ARRAY
#else
#define PACKED_REAL_ARRAY Variant::PACKED_FLOAT32_ARRAY
#endif
	ADD_GROUP("Transform", "");
	ADD_PROPERTY(PropertyInfo(PACKED_REAL_ARRAY, "transform_array", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_transform_array", "get_transform_array");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "position", PROPERTY_HINT_NONE, "suffix:m"), "set_position", "get_position");
	ADD_PROPERTY(PropertyInfo(Variant::AABB, "rotation", PROPERTY_HINT_NONE, "radians_as_degrees", PROPERTY_USAGE_NONE), "set_rotation", "get_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::AABB, "rotation_degrees", PROPERTY_HINT_NONE, U"suffix:\u00B0", PROPERTY_USAGE_EDITOR), "set_rotation_degrees", "get_rotation_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "scale", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_scale", "get_scale");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "uniform_scale", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_uniform_scale", "get_uniform_scale");
	ADD_PROPERTY(PropertyInfo(Variant::PROJECTION, "basis", PROPERTY_HINT_NONE, ""), "set_basis", "get_basis");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rotation_edit_mode", PROPERTY_HINT_ENUM, "Euler4D,Euler4D Uniform,Basis4D"), "set_rotation_edit_mode", "get_rotation_edit_mode");
	// Global transform properties.
	ADD_PROPERTY(PropertyInfo(PACKED_REAL_ARRAY, "global_transform_array", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_global_transform_array", "get_global_transform_array");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "global_position", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_global_position", "get_global_position");
	ADD_PROPERTY(PropertyInfo(Variant::AABB, "global_rotation", PROPERTY_HINT_NONE, "radians_as_degrees", PROPERTY_USAGE_NONE), "set_global_rotation", "get_global_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::AABB, "global_rotation_degrees", PROPERTY_HINT_NONE, U"suffix:\u00B0", PROPERTY_USAGE_NONE), "set_global_rotation_degrees", "get_global_rotation_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "global_scale", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_global_scale", "get_global_scale");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "global_uniform_scale", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_global_uniform_scale", "get_global_uniform_scale");
	ADD_PROPERTY(PropertyInfo(Variant::PROJECTION, "global_basis", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_global_basis", "get_global_basis");
	// Visibility.
	ADD_GROUP("Visibility", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visible"), "set_visible", "is_visible");
	ADD_SIGNAL(MethodInfo("visibility_changed"));
#undef PACKED_REAL_ARRAY
	BIND_ENUM_CONSTANT(ROTATION_EDIT_MODE_EULER4D);
	BIND_ENUM_CONSTANT(ROTATION_EDIT_MODE_EULER4D_UNIFORM);
	BIND_ENUM_CONSTANT(ROTATION_EDIT_MODE_BASIS4D);
}

void Node4D::_validate_property(PropertyInfo &p_property) const {
	if (_rotation_edit_mode != RotationEditMode::ROTATION_EDIT_MODE_BASIS4D) {
		if (p_property.name == StringName("basis")) {
			p_property.usage = PROPERTY_USAGE_STORAGE;
		}
	} else {
		if (p_property.name == StringName("rotation_degrees")) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	}
	if (_rotation_edit_mode != RotationEditMode::ROTATION_EDIT_MODE_EULER4D) {
		if (p_property.name == StringName("scale")) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	}
	if (_rotation_edit_mode != RotationEditMode::ROTATION_EDIT_MODE_EULER4D_UNIFORM) {
		if (p_property.name == StringName("uniform_scale")) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	}
}
