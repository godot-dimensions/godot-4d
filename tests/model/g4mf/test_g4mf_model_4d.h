#pragma once

#include "g4mf_test_temp_directory.h"

#include "../../../model/g4mf/g4mf_document_4d.h"
#include "../../../model/mesh/mesh_instance_4d.h"
#include "../../../model/mesh/poly/box_poly_mesh_4d.h"

#include "core/config/project_settings.h"
#include "core/core_bind.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "tests/test_macros.h"
#include "tests/test_tools.h"

#ifdef MODULE_GLTF_ENABLED
#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/3d/primitive_meshes.h"
#endif

namespace TestG4MFModel4D {
class FilePreparationProbe : public G4MFFileReference4D {
public:
	int prepare_count = 0;
	Error fail_with = OK;

	virtual Error _export_prepare_for_serialization_internal(const Ref<G4MFState4D> &p_state, const bool p_deduplicate) override {
		prepare_count++;
		const Error err = G4MFFileReference4D::_export_prepare_for_serialization_internal(p_state, p_deduplicate);
		return err != OK ? err : fail_with;
	}
};

class StatePreparationProbe : public G4MFState4D {
public:
	int prepare_count = 0;
	bool reenter = false;
	Error fail_with = OK;

protected:
	virtual Error _export_prepare_for_serialization_internal(const Ref<G4MFState4D> &p_state, const bool p_deduplicate) override {
		prepare_count++;
		if (reenter) {
			return export_prepare_for_serialization(p_state, p_deduplicate);
		}
		return fail_with;
	}
};

static Node4D *make_instance(Node *p_parent, const String &p_name, const String &p_source_path) {
	Node4D *instance = memnew(Node4D);
	instance->set_name(p_name);
	instance->set_scene_file_path(p_source_path);
	p_parent->add_child(instance);
	instance->set_owner(p_parent);
	return instance;
}

TEST_CASE("[G4MFFileReference4D] Preparation assigns file names independently of their URI") {
	String uri = "images/leaf.detail.png";
	SUBCASE("External URI") {}
	SUBCASE("Embedded file without a URI") {
		uri = String();
	}
	SUBCASE("Data URI") {
		uri = "data:application/octet-stream;base64,AQID";
	}
	Ref<G4MFState4D> state;
	state.instantiate();
	Ref<FilePreparationProbe> file;
	file.instantiate();
	file->set_file_uri(uri);
	file->set_export_preferred_path_stem("images/leaf.detail");
	Ref<FilePreparationProbe> second;
	second.instantiate();
	state->assign_unique_item_name(second, "leaf_detail");
	TypedArray<G4MFFileReference4D> files;
	files.append(file);
	files.append(second);
	state->set_g4mf_file_references(files);
	REQUIRE(state->export_prepare_for_serialization(state) == OK);
	CHECK(!file->get_item_name().is_empty());
	CHECK(file->get_item_name() != second->get_item_name());
	CHECK(file->get_file_uri() == uri);
	CHECK(second->get_item_name() == "leaf_detail");
	CHECK(state->get_item_by_name(file->get_item_name()) == file);
	CHECK(file->write_file_reference_entries_to_dictionary().has("name"));
	const String assigned_name = file->get_item_name();
	second->fail_with = ERR_INVALID_DATA;
	const Error failed_err = state->export_prepare_for_serialization(state);
	CHECK(failed_err == ERR_INVALID_DATA);
	second->fail_with = OK;
	REQUIRE(state->export_prepare_for_serialization(state) == OK);
	CHECK(file->get_item_name() == assigned_name);
	REQUIRE(file->export_prepare_for_serialization(state) == OK);
	CHECK(file->get_item_name() == assigned_name);
	Ref<G4MFFileReference4D> unnamed;
	unnamed.instantiate();
	files.append(unnamed);
	state->set_g4mf_file_references(files);
	REQUIRE(unnamed->export_prepare_for_serialization(state) == OK);
	CHECK(unnamed->get_item_name() == "file2");
}

TEST_CASE("[G4MFState4D] Preparation propagates failures and rejects reentry through the item interface") {
	Ref<StatePreparationProbe> state;
	state.instantiate();
	Ref<FilePreparationProbe> file;
	file.instantiate();
	file->set_file_uri("probe.bin");
	TypedArray<G4MFFileReference4D> files;
	files.append(file);
	state->set_g4mf_file_references(files);
	Error expected = OK;
	SUBCASE("Success") {}
	SUBCASE("Native hook failure") {
		state->fail_with = expected = ERR_INVALID_DATA;
	}
	SUBCASE("Child failure") {
		file->fail_with = expected = ERR_INVALID_DATA;
	}
	SUBCASE("Native reentry") {
		state->reenter = true;
		expected = ERR_CYCLIC_LINK;
	}
	const Ref<G4MFItem4D> item = state;
	ERR_PRINT_OFF;
	const Error err = item->export_prepare_for_serialization(state);
	ERR_PRINT_ON;
	CHECK(err == expected);
	CHECK(state->prepare_count == 1);
	CHECK(file->prepare_count == (state->fail_with == OK && !state->reenter ? 1 : 0));
	const int child_prepare_count = file->prepare_count;
	state->fail_with = file->fail_with = OK;
	state->reenter = false;
	REQUIRE(item->export_prepare_for_serialization(state) == OK);
	CHECK(state->prepare_count == 2);
	CHECK(file->prepare_count == child_prepare_count + 1);
}

TEST_CASE("[G4MFModel4D] Editable instances neither reuse nor provide shared models") {
	bool editable_first = false;
	SUBCASE("Editable instance is encountered first") {
		editable_first = true;
	}
	SUBCASE("Uneditable instance is encountered first") {}
	const bool editable[4] = { editable_first, !editable_first, !editable_first, editable_first };
	Node4D *scene = memnew(Node4D);
	scene->set_name("Root");
	scene->set_scene_file_path("res://root.tscn");
	for (int i = 0; i < 4; i++) {
		Node4D *instance = make_instance(scene, "Instance" + itos(i), "res://model.tscn");
		scene->set_editable_instance(instance, editable[i]);
		Node4D *content = make_instance(instance, "Content", String());
		content->set_position(Vector4(editable[i] ? 20 + i : 10, 0, 0, 0));
	}
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	const Error convert_err = document->export_append_from_godot_scene(state, scene);
	memdelete(scene);
	REQUIRE(convert_err == OK);
	REQUIRE(state->get_g4mf_models().size() == 3);
	const TypedArray<G4MFNode4D> nodes = state->get_g4mf_nodes();
	REQUIRE(nodes.size() == 5);
	int model_indices[4];
	for (int i = 0; i < 4; i++) {
		model_indices[i] = Ref<G4MFNode4D>(nodes[i + 1])->get_model_index();
		REQUIRE(model_indices[i] >= 0);
	}
	const int plain_first = editable_first ? 1 : 0;
	const int plain_second = editable_first ? 2 : 3;
	const int edited_first = editable_first ? 0 : 1;
	const int edited_second = editable_first ? 3 : 2;
	CHECK(model_indices[plain_first] == model_indices[plain_second]);
	CHECK(model_indices[edited_first] != model_indices[edited_second]);
	CHECK(model_indices[edited_first] != model_indices[plain_first]);
	CHECK(model_indices[edited_second] != model_indices[plain_first]);
	// Check the serialized content too, so assigning separate indices cannot hide lost edits.
	for (int i = 0; i < state->get_g4mf_models().size(); i++) {
		const Ref<G4MFModel4D> model = state->get_g4mf_models()[i];
		state->assign_unique_item_name(model, "Model" + itos(i));
	}
	const PackedByteArray bytes = document->export_write_to_byte_array(state);
	REQUIRE(!bytes.is_empty());
	Ref<G4MFState4D> imported_state;
	imported_state.instantiate();
	REQUIRE(document->import_read_from_byte_array(imported_state, bytes) == OK);
	Node *imported = document->import_generate_godot_scene(imported_state);
	REQUIRE(imported != nullptr);
	for (int i = 0; i < 4; i++) {
		Node4D *content = Object::cast_to<Node4D>(imported->get_node(NodePath("Instance" + itos(i) + "/Content")));
		CHECK(content != nullptr);
		if (content != nullptr) {
			CHECK(content->get_position() == Vector4(editable[i] ? 20 + i : 10, 0, 0, 0));
		}
	}
	memdelete(imported);
}

TEST_CASE("[G4MFModel4D] Nested editable instances prevent sharing an enclosing model") {
	bool editable_first = false;
	SUBCASE("Nested editable instance is encountered first") {
		editable_first = true;
	}
	SUBCASE("Uneditable enclosing model is encountered first") {}
	const bool editable[3] = { editable_first, !editable_first, false };
	Node4D *scene = memnew(Node4D);
	scene->set_name("Root");
	scene->set_scene_file_path("res://root.tscn");
	for (int i = 0; i < 3; i++) {
		Node4D *outer = make_instance(scene, "Outer" + itos(i), "res://outer.tscn");
		Node4D *wrapper = make_instance(outer, "Wrapper", String());
		Node4D *inner = make_instance(wrapper, "Inner", "res://inner.tscn");
		outer->set_editable_instance(inner, editable[i]);
		Node4D *content = make_instance(inner, "Content", String());
		content->set_position(Vector4(editable[i] ? 20 : 10, 0, 0, 0));
	}
	Ref<G4MFDocument4D> document;
	document.instantiate();
	// Even when the nested instance is flattened, its customizations belong to the enclosing model.
	document->set_max_nested_scene_depth(1);
	Ref<G4MFState4D> state;
	state.instantiate();
	const Error convert_err = document->export_append_from_godot_scene(state, scene);
	memdelete(scene);
	REQUIRE(convert_err == OK);
	REQUIRE(state->get_g4mf_models().size() == 2);
	const TypedArray<G4MFNode4D> nodes = state->get_g4mf_nodes();
	REQUIRE(nodes.size() == 4);
	const int plain_first = editable_first ? 1 : 0;
	const int edited = editable_first ? 0 : 1;
	CHECK(Ref<G4MFNode4D>(nodes[plain_first + 1])->get_model_index() == Ref<G4MFNode4D>(nodes[3])->get_model_index());
	CHECK(Ref<G4MFNode4D>(nodes[edited + 1])->get_model_index() != Ref<G4MFNode4D>(nodes[3])->get_model_index());
	for (int i = 0; i < state->get_g4mf_models().size(); i++) {
		const Ref<G4MFModel4D> model = state->get_g4mf_models()[i];
		state->assign_unique_item_name(model, "Model" + itos(i));
	}
	const PackedByteArray bytes = document->export_write_to_byte_array(state);
	REQUIRE(!bytes.is_empty());
	Ref<G4MFState4D> imported_state;
	imported_state.instantiate();
	REQUIRE(document->import_read_from_byte_array(imported_state, bytes) == OK);
	Node *imported = document->import_generate_godot_scene(imported_state);
	REQUIRE(imported != nullptr);
	for (int i = 0; i < 3; i++) {
		Node4D *content = Object::cast_to<Node4D>(imported->get_node(NodePath("Outer" + itos(i) + "/Wrapper/Inner/Content")));
		CHECK(content != nullptr);
		if (content != nullptr) {
			CHECK(content->get_position() == Vector4(editable[i] ? 20 : 10, 0, 0, 0));
		}
	}
	memdelete(imported);
}

TEST_CASE("[G4MFModel4D] Explicitly independent models are not reused by later instances") {
	Node4D *scene = memnew(Node4D);
	Node4D *instance = make_instance(scene, "Instance", "res://model.tscn");
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_source_path("res://root.tscn");
	const int independent = G4MFModel4D::export_pack_nodes_into_model(document, state, instance, false);
	const int shared_first = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	const int shared_second = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	memdelete(scene);
	REQUIRE(independent >= 0);
	REQUIRE(shared_first >= 0);
	CHECK(independent != shared_first);
	CHECK(shared_first == shared_second);
	CHECK(state->get_g4mf_models().size() == 2);
	CHECK(Ref<G4MFModel4D>(state->get_g4mf_models()[independent])->get_file_index() != Ref<G4MFModel4D>(state->get_g4mf_models()[shared_first])->get_file_index());
}

TEST_CASE("[G4MFModel4D] Customized descriptors share their base file without being reused as plain models") {
	bool use_additional_children = false;
	SUBCASE("Override dictionaries") {}
	SUBCASE("Additional children") {
		use_additional_children = true;
	}
	Node4D *scene = memnew(Node4D);
	Node4D *instance = make_instance(scene, "Instance", "res://model.tscn");
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_source_path("res://root.tscn");
	int customized[2];
	Dictionary customizations[2];
	for (int i = 0; i < 2; i++) {
		customized[i] = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
		REQUIRE(customized[i] >= 0);
		const Ref<G4MFModel4D> model = state->get_g4mf_models()[customized[i]];
		if (use_additional_children) {
			Ref<G4MFNode4D> child;
			child.instantiate();
			customizations[i]["Instance"] = PackedInt32Array{ state->append_g4mf_node(child) };
			model->set_node_additional_children(customizations[i]);
		} else {
			Dictionary properties;
			properties["visible"] = i == 0;
			customizations[i]["Instance"] = properties;
			model->set_overrides(customizations[i]);
		}
		state->assign_unique_item_name(model, "Customized" + itos(i));
	}
	const int plain = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	const int plain_again = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	memdelete(scene);
	REQUIRE(plain >= 0);
	CHECK(plain == plain_again);
	CHECK(customized[0] != customized[1]);
	CHECK(plain != customized[0]);
	CHECK(plain != customized[1]);
	CHECK(state->get_g4mf_models().size() == 3);
	CHECK(state->get_g4mf_file_references().size() == 1);
	for (int i = 0; i < 2; i++) {
		const Ref<G4MFModel4D> model = state->get_g4mf_models()[customized[i]];
		CHECK(model->get_file_index() == Ref<G4MFModel4D>(state->get_g4mf_models()[plain])->get_file_index());
		CHECK(model->get_overrides() == (use_additional_children ? Dictionary() : customizations[i]));
		CHECK(model->get_node_additional_children() == (use_additional_children ? customizations[i] : Dictionary()));
	}
	const Ref<G4MFModel4D> plain_model = state->get_g4mf_models()[plain];
	CHECK(plain_model->get_overrides().is_empty());
	CHECK(plain_model->get_node_additional_children().is_empty());
	state->assign_unique_item_name(plain_model, "Plain");
	const PackedByteArray bytes = document->export_write_to_byte_array(state);
	REQUIRE(!bytes.is_empty());
	Ref<G4MFState4D> imported;
	imported.instantiate();
	REQUIRE(document->import_read_from_byte_array(imported, bytes) == OK);
	CHECK(imported->get_g4mf_models().size() == 3);
	CHECK(imported->get_g4mf_file_references().size() == 1);
	for (int i = 0; i < 2; i++) {
		const Ref<G4MFModel4D> model = imported->get_g4mf_models()[customized[i]];
		CHECK(model->get_file_index() == plain_model->get_file_index());
		CHECK(model->get_overrides() == (use_additional_children ? Dictionary() : customizations[i]));
		CHECK(model->get_node_additional_children() == (use_additional_children ? customizations[i] : Dictionary()));
	}
}

#ifdef TOOLS_ENABLED
TEST_CASE("[G4MFModel4D] Additional children inside nested models remain independent on re-export") {
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("res://root.g4tf");
	const String model_json[] = {
		R"({"asset":{"dimension":4},"nodes":[{"name":"Outer"}]})",
		R"({"asset":{"dimension":4},"nodes":[{"name":"Nested","children":[1]},{"name":"Target"}]})",
	};
	TypedArray<G4MFFileReference4D> files;
	for (const String &json : model_json) {
		Ref<G4MFFileReference4D> file;
		file.instantiate();
		file->set_mime_type("model/g4mf+json");
		file->export_store_file_bytes(json.to_utf8_buffer());
		files.append(file);
	}
	state->set_g4mf_file_references(files);
	Ref<G4MFNode4D> root;
	root.instantiate();
	root->set_item_name("Root");
	root->set_children_indices(PackedInt32Array{ 1, 2 });
	state->append_g4mf_node(root);
	TypedArray<G4MFModel4D> models;
	for (int i = 0; i < 2; i++) {
		Ref<G4MFModel4D> model;
		model.instantiate();
		model->set_file_index(0);
		Dictionary additional_children;
		additional_children["Outer"] = PackedInt32Array{ 3 + i };
		additional_children["Target"] = PackedInt32Array{ 5 + i };
		model->set_node_additional_children(additional_children);
		models.append(model);
		Ref<G4MFNode4D> instance;
		instance.instantiate();
		instance->set_item_name(i == 0 ? "First" : "Second");
		instance->set_model_index(i);
		state->append_g4mf_node(instance);
	}
	Ref<G4MFModel4D> nested_model;
	nested_model.instantiate();
	nested_model->set_file_index(1);
	models.append(nested_model);
	state->set_g4mf_models(models);
	for (int i = 0; i < 2; i++) {
		Ref<G4MFNode4D> nested;
		nested.instantiate();
		nested->set_item_name(i == 0 ? "NestedFirst" : "NestedSecond");
		nested->set_model_index(2);
		state->append_g4mf_node(nested);
	}
	for (int i = 0; i < 2; i++) {
		Ref<G4MFNode4D> added;
		added.instantiate();
		added->set_item_name(i == 0 ? "AddedFirst" : "AddedSecond");
		added->set_transform(Transform4D(Basis4D(), Vector4(i + 1, 2, 3, 4)));
		state->append_g4mf_node(added);
	}
	for (int i = 0; i < models.size(); i++) {
		const Ref<G4MFModel4D> model = models[i];
		model->set_model_g4mf_document(document);
		REQUIRE(model->import_parse_file_data(state) == OK);
	}
	// The data is already preloaded. Give the files project URIs so generation retains scene boundaries.
	Ref<G4MFFileReference4D>(files[0])->set_file_uri("res://models/outer.g4tf");
	Ref<G4MFFileReference4D>(files[1])->set_file_uri("res://models/nested.g4tf");
	Node *scene = document->import_generate_godot_scene(state);
	REQUIRE(scene != nullptr);
	for (const String &path : { String("First/NestedFirst"), String("Second/NestedSecond") }) {
		Node *nested = scene->get_node_or_null(NodePath(path));
		REQUIRE(nested != nullptr);
		CHECK(scene->is_editable_instance(nested));
	}
	Ref<G4MFState4D> exported;
	exported.instantiate();
	exported->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES);
	const Error convert_err = document->export_append_from_godot_scene(exported, scene);
	memdelete(scene);
	REQUIRE(convert_err == OK);
	const G4MFTestTempDirectory output("g4mf_nested_additional_children");
	REQUIRE(output.get_error() == OK);
	const String destination = output.get_path().path_join("root.g4tf");
	ERR_PRINT_OFF; // Customized instances intentionally compete for the same source filenames.
	const Error write_err = document->export_write_to_file(exported, destination);
	ERR_PRINT_ON;
	REQUIRE(write_err == OK);
	Ref<G4MFState4D> reimported;
	reimported.instantiate();
	REQUIRE(document->import_read_from_file(reimported, destination) == OK);
	Node *round_trip = document->import_generate_godot_scene(reimported);
	REQUIRE(round_trip != nullptr);
	const String paths[] = { "First/NestedFirst/Target", "Second/NestedSecond/Target" };
	for (int i = 0; i < 2; i++) {
		Node *target = round_trip->get_node_or_null(NodePath(paths[i]));
		REQUIRE(target != nullptr);
		CHECK(target->get_child_count() == 1);
		Node4D *added = Object::cast_to<Node4D>(target->get_node_or_null(NodePath(i == 0 ? "AddedFirst" : "AddedSecond")));
		CHECK(added != nullptr);
		if (added != nullptr) {
			CHECK(added->get_position() == Vector4(i + 1, 2, 3, 4));
		}
	}
	memdelete(round_trip);
}
#endif // TOOLS_ENABLED

TEST_CASE("[G4MFModel4D] Shared embedded files respect byte deduplication and retry after failure") {
	bool deduplicate = true;
	SUBCASE("Byte deduplication enabled") {}
	SUBCASE("Byte deduplication disabled") {
		deduplicate = false;
	}
	Node4D *scene = memnew(Node4D);
	Node4D *instance = make_instance(scene, "Instance", "res://model.tscn");
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_source_path("res://root.tscn");
	const int first = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	REQUIRE(first >= 0);
	const Ref<G4MFModel4D> first_model = state->get_g4mf_models()[first];
	Dictionary overrides;
	overrides["Instance"] = Dictionary();
	first_model->set_overrides(overrides);
	const int second = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	memdelete(scene);
	REQUIRE(second >= 0);
	REQUIRE(first != second);
	const Ref<G4MFModel4D> second_model = state->get_g4mf_models()[second];
	REQUIRE(first_model->get_file_index() == second_model->get_file_index());
	state->assign_unique_item_name(first_model, "First");
	state->assign_unique_item_name(second_model, "Second");
	const Ref<G4MFFileReference4D> file_ref = state->get_g4mf_file_references()[first_model->get_file_index()];
	REQUIRE(state->export_prepare_for_serialization(state, deduplicate) == OK);
	CHECK(state->get_g4mf_buffer_views().size() == (deduplicate ? 1 : 2));
	REQUIRE(state->export_prepare_for_serialization(state, deduplicate) == OK);
	CHECK(state->get_g4mf_buffer_views().size() == (deduplicate ? 1 : 4));
	// A failure after preparing the first model must not prevent a later retry.
	second_model->set_file_index(-1);
	ERR_PRINT_OFF;
	const Error failed_err = state->export_prepare_for_serialization(state, deduplicate);
	ERR_PRINT_ON;
	CHECK(failed_err != OK);
	CHECK(state->get_g4mf_buffer_views().size() == (deduplicate ? 1 : 5));
	second_model->set_file_index(first_model->get_file_index());
	REQUIRE(state->export_prepare_for_serialization(state, deduplicate) == OK);
	CHECK(state->get_g4mf_buffer_views().size() == (deduplicate ? 1 : 7));
	CHECK(file_ref->get_buffer_view_index() == (deduplicate ? 0 : 6));
	// Independent callers can prepare models without an enclosing state pass.
	REQUIRE(first_model->export_prepare_for_serialization(state, deduplicate) == OK);
	REQUIRE(second_model->export_prepare_for_serialization(state, deduplicate) == OK);
	CHECK(state->get_g4mf_buffer_views().size() == (deduplicate ? 1 : 9));
}

TEST_CASE("[G4MFModel4D] Detached roots retain independent models when their flag cannot be checked") {
	Node4D *scene = memnew(Node4D);
	Node4D *instance = make_instance(scene, "Instance", "res://model.tscn");
	scene->set_editable_instance(instance, true);
	scene->remove_child(instance);
	memdelete(scene);
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_source_path("res://root.tscn");
	const int first = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	const int second = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	memdelete(instance);
	REQUIRE(first >= 0);
	REQUIRE(second >= 0);
	CHECK(first != second);
	CHECK(state->get_g4mf_models().size() == 2);
}
TEST_CASE("[G4MFModel4D] Deduplication retains source directories extensions and case") {
	const Vector<String> sources = {
		"res://a.scn", "res://a.tscn", "res://a.glb", "res://a.gltf", "res://a.g4b", "res://a.g4tf",
		"res://b/a.scn", "res://c/a.scn", "res://d/a.scn", "res://Case/a.scn", "res://case/a.scn",
		"res://CamelCase.scn", "res://camel_case.scn", "res://a.part.scn", "res://a.part.tscn"
	};
	Node4D *scene = memnew(Node4D);
	scene->set_name("Root");
	scene->set_scene_file_path("res://root.tscn");
	for (int i = 0; i < sources.size(); i++) {
		make_instance(scene, "First" + itos(i), sources[i]);
		make_instance(scene, "Second" + itos(i), sources[i]);
	}
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	const Error convert_err = document->export_append_from_godot_scene(state, scene);
	memdelete(scene);
	REQUIRE(convert_err == OK);
	CHECK(state->get_g4mf_models().size() == sources.size());
	CHECK(state->get_g4mf_file_references().size() == sources.size());
	const TypedArray<G4MFNode4D> nodes = state->get_g4mf_nodes();
	REQUIRE(nodes.size() == sources.size() * 2 + 1);
	for (int i = 0; i < sources.size(); i++) {
		const int first = Ref<G4MFNode4D>(nodes[i * 2 + 1])->get_model_index();
		const int second = Ref<G4MFNode4D>(nodes[i * 2 + 2])->get_model_index();
		CHECK(first >= 0);
		CHECK(first == second);
		for (int previous = 0; previous < i; previous++) {
			CHECK(first != Ref<G4MFNode4D>(nodes[previous * 2 + 1])->get_model_index());
		}
	}
}

TEST_CASE("[G4MFModel4D] Equivalent source path spellings share a model") {
	String root = "res://";
	SUBCASE("Project protocol") {}
	SUBCASE("User protocol") {
		root = "user://";
	}
	SUBCASE("Absolute filesystem path") {
		root = "/g4mf_source_test/";
	}
	SUBCASE("Windows drive") {
		root = "C:/g4mf_source_test/";
	}
	SUBCASE("Network share") {
		root = "//g4mf_source_server/share/";
	}
	const String source = root + "models/a.tscn";
	Vector<String> aliases = {
		source, root + "models/./a.tscn", root + "models/sub/../a.tscn",
		root + "models//a.tscn", source.replace("/", "\\")
	};
	Node4D *scene = memnew(Node4D);
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_source_path(root + "root.tscn");
	int first = -1;
	for (int i = 0; i < aliases.size(); i++) {
		Node4D *instance = make_instance(scene, "Alias" + itos(i), aliases[i]);
		const int model_index = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
		if (i == 0) {
			first = model_index;
		}
		CHECK(model_index >= 0);
		CHECK(model_index == first);
	}
	memdelete(scene);
	CHECK(state->get_g4mf_models().size() == 1);
}

TEST_CASE("[G4MFModel4D] Output URI changes do not change source identity") {
	Node4D *scene = memnew(Node4D);
	Node4D *instance = make_instance(scene, "Instance", "res://model.tscn");
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_source_path("res://root.tscn");
	const int first = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	REQUIRE(first >= 0);
	const Ref<G4MFModel4D> model = state->get_g4mf_models()[first];
	const Ref<G4MFFileReference4D> file_ref = state->get_g4mf_file_references()[model->get_file_index()];
	file_ref->set_file_uri("renamed.g4tf");
	const int renamed = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	file_ref->set_file_uri(String());
	const int embedded = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	memdelete(scene);
	CHECK(first == renamed);
	CHECK(first == embedded);
	CHECK(state->get_g4mf_models().size() == 1);
}

TEST_CASE("[G4MFModel4D] Models without source paths cannot be deduplicated by name") {
	Node4D *scene = memnew(Node4D);
	Node4D *instance = make_instance(scene, "Instance", String());
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_source_path("res://root.tscn");
	const int anonymous_first = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	const int anonymous_second = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	instance->set_scene_file_path("res://instance.tscn");
	const int known_first = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	const int known_second = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	memdelete(scene);
	CHECK(anonymous_first >= 0);
	CHECK(anonymous_second >= 0);
	CHECK(known_first >= 0);
	CHECK(anonymous_first != anonymous_second);
	CHECK(known_first != anonymous_first);
	CHECK(known_first != anonymous_second);
	CHECK(known_first == known_second);
	CHECK(state->get_g4mf_models().size() == 3);
}

TEST_CASE("[G4MFModel4D] Writing preserves the original source path") {
	Node4D *scene = memnew(Node4D);
	scene->set_name("Root");
	const String source = "res://source/root.tscn";
	scene->set_scene_file_path(source);
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	const Error convert_err = document->export_append_from_godot_scene(state, scene);
	memdelete(scene);
	REQUIRE(convert_err == OK);
	REQUIRE(state->get_source_path() == source);
	const G4MFTestTempDirectory output("g4mf_source_path");
	REQUIRE(output.get_error() == OK);
	const String destination = output.get_path().path_join("root.g4b");
	const Error write_err = document->export_write_to_file(state, destination);
	const Error remove_err = DirAccess::remove_absolute(destination);
	CHECK(write_err == OK);
	CHECK(remove_err == OK);
	CHECK(state->get_source_path() == source);
	CHECK(state->get_g4mf_file_path() == destination);
}

TEST_CASE("[G4MFModel4D] Model paths preserve folders below the common source directory") {
	// Project and user paths may map to the same filesystem root, depending on earlier tests or a loaded project.
	String mixed_root_stem = "model";
	const String project_file = ProjectSettings::get_singleton()->globalize_path("res://path/to/file.scn");
	const String user_file = ProjectSettings::get_singleton()->globalize_path("user://path/to/subfolder/model.scn");
	if (project_file.is_absolute_path() && user_file.is_absolute_path()) {
		String relative_path = project_file.get_base_dir().path_to_file(user_file);
		if (relative_path.is_relative_path()) {
			while (relative_path.begins_with("../")) {
				relative_path = relative_path.trim_prefix("../");
			}
			mixed_root_stem = G4MFFileReference4D::sanitize_file_path_uri(relative_path.get_basename(), false);
		}
	}
	struct PathCase {
		String parent;
		String source;
		String expected_stem;
	};
	const Vector<PathCase> paths = {
		{ "res://path/to/file.scn", "res://path/to/some/subfolder/model.scn", "some/subfolder/model" },
		{ "res://path/to/file.scn", "res://path/to/model.scn", "model" },
		{ "res://path/to/file.scn", "res://path/to/some/../subfolder//model.scn", "subfolder/model" },
		{ "res://path/base/../to/file.scn", "res://path/to/subfolder/model.scn", "subfolder/model" },
		{ "res://path/to/file.scn", "res://path\\to\\some\\model.scn", "some/model" },
		{ "res://path/to/file.scn", "res://path/to_other/subfolder/model.scn", "to_other/subfolder/model" },
		{ "res://path/to/file.scn", "res://path/shared/subfolder/model.scn", "shared/subfolder/model" },
		{ "res://path/to/file.scn", "res://path/model.scn", "model" },
		{ "res://path/to/file.scn", "user://path/to/subfolder/model.scn", mixed_root_stem },
		{ "", "res://path/to/subfolder/model.scn", "model" },
		{ "res://path/to/file.scn", "", "instance" },
		{ "res://file.scn", "res://some/subfolder/model.scn", "some/subfolder/model" },
		{ "user://path/to/file.scn", "user://path/to/some/model.scn", "some/model" },
		{ "C:/path/to/file.scn", "C:/path/to/some/model.scn", "some/model" },
		{ "C:/path/to/file.scn", "D:/path/to/some/model.scn", "model" },
		{ "C:/file.scn", "C:/some/model.scn", "some/model" },
		{ "/file.scn", "/some/model.scn", "some/model" },
		{ "//server/share/path/file.scn", "//server/share/path/some/model.scn", "some/model" },
		{ "//server/share/path/file.scn", "//server/other/path/some/model.scn", "model" },
	};
	const G4MFTestTempDirectory output("g4mf_relative_model_paths");
	REQUIRE(output.get_error() == OK);
	for (int i = 0; i < paths.size(); i++) {
		INFO("Parent: ", paths[i].parent, ", source: ", paths[i].source);
		Node4D *scene = memnew(Node4D);
		Node4D *instance = make_instance(scene, "Instance", paths[i].source);
		Node4D *content = make_instance(instance, "Content", String());
		const Vector4 position(i + 1, 2, 3, 4);
		content->set_position(position);
		Ref<G4MFDocument4D> document;
		document.instantiate();
		Ref<G4MFState4D> state;
		state.instantiate();
		state->set_source_path(paths[i].parent);
		// Standalone packing also supports models without source paths.
		const int model_index = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
		memdelete(scene);
		REQUIRE(model_index >= 0);
		const Ref<G4MFModel4D> model = state->get_g4mf_models()[model_index];
		state->assign_unique_item_name(model, "StableModel");
		const Ref<G4MFFileReference4D> file_ref = state->get_g4mf_file_references()[model->get_file_index()];
		// Check before writing so a path regression cannot write into a source directory.
		REQUIRE(file_ref->get_export_preferred_path_stem() == paths[i].expected_stem);
		CHECK(state->get_g4mf_file_path().is_empty());
		state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES);
		const String destination = output.get_path().path_join(itos(i) + "/root.g4tf");
		REQUIRE(document->export_write_to_file(state, destination) == OK);
		CHECK(file_ref->get_file_uri() == paths[i].expected_stem + ".g4tf");
		const String model_path = destination.get_base_dir().path_join(file_ref->get_file_uri());
		CHECK(FileAccess::exists(model_path));
		Ref<G4MFState4D> imported;
		imported.instantiate();
		REQUIRE(document->import_read_from_file(imported, model_path) == OK);
		Node *root = document->import_generate_godot_scene(imported);
		REQUIRE(root != nullptr);
		Node4D *imported_content = Object::cast_to<Node4D>(root->get_node_or_null(NodePath("Content")));
		CHECK(imported_content != nullptr);
		if (imported_content != nullptr) {
			CHECK(imported_content->get_position() == position);
		}
		memdelete(root);
	}
}

TEST_CASE("[G4MFModel4D] Nested model folders follow each containing source file across output locations") {
	String part_source = "res://path/to/some/subfolder/deeper/part.scn";
	String part_stem = "deeper/part";
	SUBCASE("Part beneath its containing model's source directory") {}
	SUBCASE("Part outside its containing model's source directory") {
		part_source = "res://path/to/shared/part.scn";
		part_stem = "shared/part";
	}
	Node4D *scene = memnew(Node4D);
	scene->set_name("Root");
	scene->set_scene_file_path("res://path/to/file.scn");
	Node4D *model_node = make_instance(scene, "Model", "res://path/to/some/subfolder/model.scn");
	Node4D *part = make_instance(model_node, "Part", part_source);
	Node4D *content = make_instance(part, "Content", String());
	const Vector4 position(1, 2, 3, 4);
	content->set_position(position);
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	REQUIRE(document->export_append_from_godot_scene(state, scene) == OK);
	memdelete(scene);
	REQUIRE(state->get_g4mf_models().size() == 1);
	const Ref<G4MFModel4D> model = state->get_g4mf_models()[0];
	state->assign_unique_item_name(model, "StableModel");
	state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES);
	const Ref<G4MFFileReference4D> file_ref = state->get_g4mf_file_references()[model->get_file_index()];
	const G4MFTestTempDirectory output("g4mf_nested_model_paths");
	REQUIRE(output.get_error() == OK);
	for (const String &relative_destination : Vector<String>{ "one/file.g4tf", "two/file.g4tf", "two/file.g4b" }) {
		// These destinations are independent exports; preserve conversion data but allocate new dependency paths.
		state->reset_path_context();
		const String destination = output.get_path().path_join(relative_destination);
		ERR_PRINT_OFF; // Generated nested-model names sanitize filename extensions.
		const Error write_err = document->export_write_to_file(state, destination);
		ERR_PRINT_ON;
		REQUIRE(write_err == OK);
		const String extension = relative_destination.ends_with(".g4tf") ? ".g4tf" : ".g4b";
		CHECK(file_ref->get_file_uri() == "some/subfolder/model" + extension);
		const String model_path = destination.get_base_dir().path_join(file_ref->get_file_uri());
		CHECK(FileAccess::exists(model_path));
		Ref<G4MFState4D> model_state;
		model_state.instantiate();
		REQUIRE(document->import_read_from_file(model_state, model_path) == OK);
		REQUIRE(model_state->get_g4mf_models().size() == 1);
		const Ref<G4MFModel4D> part_model = model_state->get_g4mf_models()[0];
		const Ref<G4MFFileReference4D> part_file = model_state->get_g4mf_file_references()[part_model->get_file_index()];
		CHECK(part_file->get_file_uri() == part_stem + extension);
		CHECK(FileAccess::exists(model_path.get_base_dir().path_join(part_file->get_file_uri())));
		Ref<G4MFState4D> imported;
		imported.instantiate();
		REQUIRE(document->import_read_from_file(imported, destination) == OK);
		Node *root = document->import_generate_godot_scene(imported);
		REQUIRE(root != nullptr);
		Node4D *imported_content = Object::cast_to<Node4D>(root->get_node_or_null(NodePath("Model/Part/Content")));
		CHECK(imported_content != nullptr);
		if (imported_content != nullptr) {
			CHECK(imported_content->get_position() == position);
		}
		memdelete(root);
	}
}

TEST_CASE("[G4MFModel4D] Reused nested models follow the current output policy in each destination") {
	G4MFState4D::ExternalDataMode conversion_mode = G4MFState4D::EXTERNAL_DATA_MODE_AUTOMATIC;
	SUBCASE("Converted with automatic output policy") {}
	SUBCASE("Converted with embedded output policy") {
		conversion_mode = G4MFState4D::EXTERNAL_DATA_MODE_EMBED_EVERYTHING;
	}
	Node4D *scene = memnew(Node4D);
	scene->set_name("Root");
	scene->set_scene_file_path("res://source/root.scn");
	Node4D *branch = make_instance(scene, "Branch", "res://source/models/branch.scn");
	Node4D *leaf = make_instance(branch, "Leaf", "res://source/models/detail/leaf.scn");
	const Vector4 position(1, 2, 3, 4);
	make_instance(leaf, "Content", String())->set_position(position);
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_external_data_mode(conversion_mode);
	// Convert before assigning any destination, and reuse these same models for every write.
	REQUIRE(document->export_append_from_godot_scene(state, scene) == OK);
	memdelete(scene);
	REQUIRE(state->get_g4mf_file_path().is_empty());
	const G4MFTestTempDirectory output("g4mf_nested_output_policy");
	REQUIRE(output.get_error() == OK);
	struct WriteCase {
		String destination;
		G4MFState4D::ExternalDataMode mode;
	};
	const Vector<WriteCase> writes = {
		{ "one/root.g4tf", G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES },
		{ "two/root.g4b", G4MFState4D::EXTERNAL_DATA_MODE_EMBED_EVERYTHING },
		{ "three/root.g4tf", G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES },
		{ "four/root.g4tf", G4MFState4D::EXTERNAL_DATA_MODE_EMBED_EVERYTHING },
		{ "five/root.g4b", G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_ALL_FILES },
		{ "six/root.g4b", G4MFState4D::EXTERNAL_DATA_MODE_EMBED_EVERYTHING },
		{ "seven/root.g4tf", G4MFState4D::EXTERNAL_DATA_MODE_AUTOMATIC },
	};
	for (const WriteCase &write : writes) {
		state->reset_path_context();
		const String destination = output.get_path().path_join(write.destination);
		state->set_external_data_mode(write.mode);
		ERR_PRINT_OFF; // Generated model names sanitize filename extensions.
		const Error write_err = document->export_write_to_file(state, destination);
		ERR_PRINT_ON;
		REQUIRE(write_err == OK);
		const bool embedded = write.mode == G4MFState4D::EXTERNAL_DATA_MODE_EMBED_EVERYTHING;
		const String extension = destination.ends_with(".g4tf") ? ".g4tf" : ".g4b";
		Ref<G4MFState4D> imported;
		imported.instantiate();
		ERR_PRINT_OFF; // Reusing embedded states leaves unused buffer views whose alignment is inferred.
		const Error import_err = document->import_read_from_file(imported, destination);
		ERR_PRINT_ON;
		REQUIRE(import_err == OK);
		REQUIRE(imported->get_g4mf_models().size() == 1);
		const Ref<G4MFModel4D> branch_model = imported->get_g4mf_models()[0];
		const Ref<G4MFFileReference4D> branch_file = imported->get_g4mf_file_references()[branch_model->get_file_index()];
		CHECK(branch_file->get_file_uri() == (embedded ? String() : "models/branch" + extension));
		const PackedByteArray branch_bytes = branch_file->import_read_file_data(imported);
		REQUIRE(!branch_bytes.is_empty());
		Ref<G4MFState4D> branch_state;
		branch_state.instantiate();
		branch_state->set_g4mf_file_path(embedded ? destination : branch_file->resolve_file_uri(imported->get_g4mf_base_path()));
		ERR_PRINT_OFF; // The reused branch also retains unused buffer views.
		const Error branch_import_err = document->import_read_from_byte_array(branch_state, branch_bytes);
		ERR_PRINT_ON;
		REQUIRE(branch_import_err == OK);
		REQUIRE(branch_state->get_g4mf_models().size() == 1);
		const Ref<G4MFModel4D> leaf_model = branch_state->get_g4mf_models()[0];
		const Ref<G4MFFileReference4D> leaf_file = branch_state->get_g4mf_file_references()[leaf_model->get_file_index()];
		CHECK(leaf_file->get_file_uri() == (embedded ? String() : "detail/leaf" + extension));
		if (embedded) {
			CHECK(branch_file->get_buffer_view_index() >= 0);
			CHECK(leaf_file->get_buffer_view_index() >= 0);
			const Ref<DirAccess> directory = DirAccess::open(destination.get_base_dir());
			REQUIRE(directory.is_valid());
			CHECK(directory->get_files().size() == 1);
			CHECK(directory->get_directories().is_empty());
		}
		Node *root = document->import_generate_godot_scene(imported);
		REQUIRE(root != nullptr);
		Node4D *content = Object::cast_to<Node4D>(root->get_node_or_null(NodePath("Branch/Leaf/Content")));
		REQUIRE(content != nullptr);
		CHECK(content->get_position() == position);
		memdelete(root);
	}
	// Without a destination, automatic output must also embed descendants rather than reuse old paths.
	state->set_g4mf_file_path(String());
	state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_AUTOMATIC);
	const PackedByteArray bytes = document->export_write_to_byte_array(state);
	REQUIRE(!bytes.is_empty());
	Ref<G4MFState4D> imported;
	imported.instantiate();
	REQUIRE(document->import_read_from_byte_array(imported, bytes) == OK);
	Node *root = document->import_generate_godot_scene(imported);
	REQUIRE(root != nullptr);
	CHECK(root->has_node(NodePath("Branch/Leaf/Content")));
	memdelete(root);
}

TEST_CASE("[G4MFModel4D] Nested writers protect the main file and coordinate colliding model paths") {
	Node4D *scene = memnew(Node4D);
	scene->set_name("Root");
	scene->set_scene_file_path("res://source/input.scn");
	Node4D *branch = make_instance(scene, "Branch", "res://source/branch.scn");
	Node4D *nested = make_instance(branch, "Nested", "res://source/root.scn");
	make_instance(nested, "Content", String())->set_position(Vector4(1, 2, 3, 4));
	Node4D *direct = make_instance(scene, "Direct", "res://source/root.tscn");
	make_instance(direct, "Content", String())->set_position(Vector4(5, 6, 7, 8));
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	REQUIRE(document->export_append_from_godot_scene(state, scene) == OK);
	memdelete(scene);
	REQUIRE(state->get_g4mf_models().size() == 2);
	const Ref<G4MFModel4D> direct_model = state->get_g4mf_models()[1];
	const Ref<G4MFFileReference4D> direct_file = state->get_g4mf_file_references()[direct_model->get_file_index()];
	state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES);
	// Fail after the models are written, then retry using the same converted states.
	Ref<FilePreparationProbe> failure;
	failure.instantiate();
	failure->fail_with = ERR_INVALID_DATA;
	state->get_g4mf_file_references().append(failure);
	const G4MFTestTempDirectory output("g4mf_shared_output_paths");
	REQUIRE(output.get_error() == OK);
	const String output_dir = output.get_path();
	ERR_PRINT_OFF;
	const Error failed_err = document->export_write_to_file(state, output_dir.path_join("one/root.g4tf"));
	ERR_PRINT_ON;
	REQUIRE(failed_err == ERR_INVALID_DATA);
	failure->fail_with = OK;
	for (const String &relative_destination : Vector<String>{ "one/root.g4tf", "two/root.g4tf", "two/other.g4tf", "two/root.g4b", "two/root.g4tf" }) {
		state->reset_path_context();
		const String destination = output_dir.path_join(relative_destination);
		ERR_PRINT_OFF; // The nested and direct models intentionally collide with the main filename.
		const Error write_err = document->export_write_to_file(state, destination);
		ERR_PRINT_ON;
		REQUIRE(write_err == OK);
		const String extension = relative_destination.ends_with(".g4tf") ? ".g4tf" : ".g4b";
		const bool main_uses_root = relative_destination.get_file() != "other.g4tf";
		// The branch's nested root claims root2, then the direct model must claim root3.
		// Renaming the main output frees root again on the next independent write.
		REQUIRE(direct_file->get_file_uri() == (main_uses_root ? "root3" : "root2") + extension);
		Ref<G4MFState4D> branch_state;
		branch_state.instantiate();
		REQUIRE(document->import_read_from_file(branch_state, destination.get_base_dir().path_join("branch" + extension)) == OK);
		REQUIRE(branch_state->get_g4mf_file_references().size() == 1);
		const Ref<G4MFFileReference4D> nested_file = branch_state->get_g4mf_file_references()[0];
		REQUIRE(nested_file->get_file_uri() == (main_uses_root ? "root2" : "root") + extension);
		Ref<G4MFState4D> imported;
		imported.instantiate();
		REQUIRE(document->import_read_from_file(imported, destination) == OK);
		Node *root = document->import_generate_godot_scene(imported);
		REQUIRE(root != nullptr);
		Node4D *nested_content = Object::cast_to<Node4D>(root->get_node_or_null(NodePath("Branch/Nested/Content")));
		Node4D *direct_content = Object::cast_to<Node4D>(root->get_node_or_null(NodePath("Direct/Content")));
		REQUIRE(nested_content != nullptr);
		REQUIRE(direct_content != nullptr);
		CHECK(nested_content->get_position() == Vector4(1, 2, 3, 4));
		CHECK(direct_content->get_position() == Vector4(5, 6, 7, 8));
		memdelete(root);
	}
}

TEST_CASE("[G4MFModel4D] Nested buffer sidecars cannot overwrite the main output file") {
	const G4MFTestTempDirectory output("g4mf_buffer_path_collision");
	REQUIRE(output.get_error() == OK);
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> source;
	source.instantiate();
	source->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_ALL_FILES);
	PackedByteArray data;
	data.resize(4096);
	data.fill(17);
	source->get_g4mf_buffers().append(data);
	Ref<G4MFNode4D> node;
	node.instantiate();
	source->append_g4mf_node(node);
	const String input_dir = output.get_path().path_join("input");
	// Import infers the child state's export policy from its buffer URIs.
	REQUIRE(document->export_write_to_file(source, input_dir.path_join("a.g4b")) == OK);
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path(input_dir.path_join("parent.g4tf"));
	state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_ALL_FILES);
	Ref<G4MFFileReference4D> file_ref;
	file_ref.instantiate();
	file_ref->set_mime_type("model/g4mf-binary");
	file_ref->set_file_uri("a.g4b");
	state->get_g4mf_file_references().append(file_ref);
	Ref<G4MFModel4D> model;
	model.instantiate();
	model->set_file_index(0);
	model->set_model_g4mf_document(document);
	state->get_g4mf_models().append(model);
	REQUIRE(model->import_parse_file_data(state) == OK);
	for (const String &folder : Vector<String>{ "one", "two", "one" }) {
		// .bin is still a valid binary G4MF filename. The child would normally use it for a buffer.
		const String destination = output.get_path().path_join(folder + "/a_buffer0.bin");
		ERR_PRINT_OFF; // The imported model's generated name sanitizes its filename extension.
		const Error write_err = document->export_write_to_file(state, destination);
		ERR_PRINT_ON;
		REQUIRE(write_err == OK);
		CHECK(file_ref->get_file_uri() == (folder == "two" ? "../one/a.g4b" : "a.g4b"));
		Ref<G4MFState4D> imported;
		imported.instantiate();
		REQUIRE(document->import_read_from_file(imported, destination) == OK);
		Ref<G4MFState4D> child;
		child.instantiate();
		REQUIRE(document->import_read_from_file(child, file_ref->resolve_file_uri(destination.get_base_dir())) == OK);
		REQUIRE(child->get_g4mf_buffers().size() == 1);
		CHECK(PackedByteArray(child->get_g4mf_buffers()[0]) == data);
		const Array buffers = child->get_g4mf_json()["buffers"];
		REQUIRE(buffers.size() == 1);
		CHECK(String(Dictionary(buffers[0])["uri"]) == "a_buffer02.bin");
	}
}

TEST_CASE("[G4MFState4D] Generated paths use lowercase snake_case without inserting separators next to digits") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("res://export/main.g4tf");
	CHECK(state->reserve_unique_path_uri_from_stem("ExistingFolder/MyModel", ".G4TF", false) == "existing_folder/my_model.g4tf");
	CHECK(state->reserve_unique_path_uri_from_stem("ExistingFolder/my_model", ".g4tf", false) == "existing_folder/my_model2.g4tf");
	CHECK(state->reserve_unique_path_uri_from_stem("EXISTING_FOLDER/MY_MODEL", ".g4tf", false) == "existing_folder/my_model3.g4tf");
	CHECK(state->reserve_unique_path_uri_from_stem("res://MixedCase/ArchiveName", ".TAR.GZ", false) == "res://mixed_case/archive_name.tar.gz");
	CHECK(state->reserve_unique_path_uri_from_stem("C:/ExistingFolder/BufferData", ".BIN", false) == "C:/existing_folder/buffer_data.bin");
	CHECK(state->reserve_unique_path_uri_from_stem("../ExistingFolder/TextureName", ".PNG", false) == "../existing_folder/texture_name.png");
	CHECK(state->reserve_unique_path_uri_from_stem("BufferData0", ".BIN", false) == "buffer_data0.bin");
	CHECK(state->reserve_unique_path_uri_from_stem("model0", ".g4tf", false) == "model0.g4tf");
	CHECK(state->reserve_unique_path_uri_from_stem("A 0 b0_c_0_HelloWorld/A 0 b0_c_0_HelloWorld", ".BIN", false) == "a_0_b0_c_0_hello_world/a_0_b0_c_0_hello_world.bin");
	CHECK(G4MFFileReference4D::sanitize_file_path_uri("ExistingFolder/MyModel.G4TF", false) == "existing_folder/my_model.g4tf");
	// Imported URIs preserve the spelling of existing files instead of sanitizing it.
	Dictionary file_dict;
	file_dict["uri"] = "ExistingFolder/MyModel.G4TF";
	const Ref<G4MFFileReference4D> imported_file = G4MFFileReference4D::file_reference_from_dictionary(file_dict);
	CHECK(imported_file->get_file_uri() == "ExistingFolder/MyModel.G4TF");
}

TEST_CASE("[G4MFModel4D] Diamond dependencies share eligible external files across nested states") {
	bool editable = false;
	String second_source = "res://source/shared/d.scn";
	SUBCASE("Same uneditable source") {}
	SUBCASE("Editable instances keep independent outputs") {
		editable = true;
	}
	SUBCASE("Different source extensions keep independent outputs") {
		second_source = "res://source/shared/d.tscn";
	}
	const bool shared = !editable && second_source.ends_with("d.scn");
	Node4D *scene = memnew(Node4D);
	scene->set_name("Root");
	scene->set_scene_file_path("res://source/root.scn");
	for (int i = 0; i < 2; i++) {
		Node4D *branch = make_instance(scene, i == 0 ? "B" : "C", i == 0 ? "res://source/b/b.scn" : "res://source/c/c.scn");
		Node4D *leaf = make_instance(branch, "D", i == 0 ? "res://source/shared/d.scn" : second_source);
		branch->set_editable_instance(leaf, editable);
		make_instance(leaf, "Content", String())->set_position(Vector4(shared ? 1 : i + 1, 2, 3, 4));
	}
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	REQUIRE(document->export_append_from_godot_scene(state, scene) == OK);
	memdelete(scene);
	CHECK(state->get_g4mf_file_path().is_empty());
	const G4MFTestTempDirectory output("g4mf_diamond_models");
	REQUIRE(output.get_error() == OK);
	// Fail after nested files have been written, then delete D to prove a retry writes it again.
	Ref<FilePreparationProbe> failure;
	failure.instantiate();
	failure->fail_with = ERR_INVALID_DATA;
	state->get_g4mf_file_references().append(failure);
	state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES);
	const String first_path = output.get_path().path_join("0/root.g4tf");
	ERR_PRINT_OFF;
	const Error failed_err = document->export_write_to_file(state, first_path);
	ERR_PRINT_ON;
	REQUIRE(failed_err == ERR_INVALID_DATA);
	REQUIRE(DirAccess::remove_absolute(first_path.get_base_dir().path_join("b/shared/d.g4tf")) == OK);
	failure->fail_with = OK;
	for (int step = 0; step < 4; step++) {
		state->reset_path_context();
		const bool embedded = step == 1;
		const String extension = step == 2 ? ".g4b" : ".g4tf";
		const String destination = output.get_path().path_join(itos(step) + "/root" + extension);
		state->set_external_data_mode(embedded ? G4MFState4D::EXTERNAL_DATA_MODE_EMBED_EVERYTHING : G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES);
		REQUIRE(document->export_write_to_file(state, destination) == OK);
		if (!embedded) {
			Ref<G4MFState4D> branches[2];
			Ref<G4MFFileReference4D> files[2];
			for (int i = 0; i < 2; i++) {
				branches[i].instantiate();
				const String branch_path = destination.get_base_dir().path_join((i == 0 ? "b/b" : "c/c") + extension);
				ERR_PRINT_OFF; // Earlier embedded writes leave unused branch buffer views.
				const Error branch_import_err = document->import_read_from_file(branches[i], branch_path);
				ERR_PRINT_ON;
				REQUIRE(branch_import_err == OK);
				REQUIRE(branches[i]->get_g4mf_models().size() == 1);
				const Ref<G4MFModel4D> model = branches[i]->get_g4mf_models()[0];
				files[i] = branches[i]->get_g4mf_file_references()[model->get_file_index()];
				CHECK(files[i]->get_file_uri() == (i == 1 && shared ? "../b/shared/d" : "shared/d") + extension);
				CHECK(files[i]->get_buffer_view_index() == -1);
			}
			CHECK(files[0] != files[1]);
			CHECK(FileAccess::exists(destination.get_base_dir().path_join("b/shared/d" + extension)));
			CHECK(FileAccess::exists(destination.get_base_dir().path_join("c/shared/d" + extension)) == !shared);
		}
		Ref<G4MFState4D> imported;
		imported.instantiate();
		ERR_PRINT_OFF; // Earlier embedded writes leave unused buffer views in the parent and branches.
		const Error import_err = document->import_read_from_file(imported, destination);
		ERR_PRINT_ON;
		REQUIRE(import_err == OK);
		Node *root = document->import_generate_godot_scene(imported);
		REQUIRE(root != nullptr);
		for (int i = 0; i < 2; i++) {
			Node4D *content = Object::cast_to<Node4D>(root->get_node_or_null(NodePath(i == 0 ? "B/D/Content" : "C/D/Content")));
			REQUIRE(content != nullptr);
			CHECK(content->get_position() == Vector4(shared ? 1 : i + 1, 2, 3, 4));
		}
		memdelete(root);
	}
}

TEST_CASE("[G4MFModel4D] Nested mesh preferences share source files and select representation during import") {
	Ref<BoxPolyMesh4D> source_mesh;
	source_mesh.instantiate();
	const PackedVector4Array source_vertices = source_mesh->get_vertex_positions();
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_source_path("res://source/root.scn");
	const G4MFMeshSurface4D::MeshSurfaceFormat preferences[2] = {
		G4MFMeshSurface4D::MESH_SURFACE_FORMAT_WIREFRAME,
		G4MFMeshSurface4D::MESH_SURFACE_FORMAT_TETRAHEDRAL,
	};
	Node4D *scene = memnew(Node4D);
	for (int i = 0; i < 2; i++) {
		Node4D *branch = make_instance(scene, i == 0 ? "B" : "C", i == 0 ? "res://source/b/b.scn" : "res://source/c/c.scn");
		Node4D *model = make_instance(branch, "D", "res://source/shared/d.scn");
		MeshInstance4D *geometry = memnew(MeshInstance4D);
		geometry->set_name("Geometry");
		geometry->set_mesh(source_mesh);
		model->add_child(geometry);
		geometry->set_owner(model);
		state->set_preferred_mesh_surface_format(preferences[i]);
		REQUIRE(G4MFModel4D::export_pack_nodes_into_model(document, state, branch) >= 0);
	}
	memdelete(scene);
	CHECK(state->get_g4mf_file_path().is_empty());
	state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES);
	const G4MFTestTempDirectory output("g4mf_nested_mesh_preference");
	REQUIRE(output.get_error() == OK);
	for (const String &extension : Vector<String>{ ".g4tf", ".g4b" }) {
		const String destination = output.get_path().path_join(extension.substr(1) + "/root" + extension);
		ERR_PRINT_OFF; // Generated branch and leaf names sanitize filename extensions.
		const Error write_err = document->export_write_to_file(state, destination);
		ERR_PRINT_ON;
		REQUIRE(write_err == OK);
		for (int i = 0; i < 2; i++) {
			Ref<G4MFState4D> imported;
			imported.instantiate();
			const String branch_path = destination.get_base_dir().path_join((i == 0 ? "b/b" : "c/c") + extension);
			REQUIRE(document->import_read_from_file(imported, branch_path) == OK);
			REQUIRE(imported->get_g4mf_models().size() == 1);
			const Ref<G4MFModel4D> model = imported->get_g4mf_models()[0];
			const Ref<G4MFFileReference4D> file = imported->get_g4mf_file_references()[model->get_file_index()];
			CHECK(file->get_file_uri() == (i == 0 ? "shared/d" : "../b/shared/d") + extension);
			// Both branches read one source file, while choosing their own Godot mesh representation.
			imported->set_preferred_mesh_surface_format(preferences[i]);
			Node *root = document->import_generate_godot_scene(imported);
			REQUIRE(root != nullptr);
			MeshInstance4D *geometry = Object::cast_to<MeshInstance4D>(root->get_node_or_null(NodePath("D/Geometry")));
			REQUIRE(geometry != nullptr);
			REQUIRE(geometry->get_mesh().is_valid());
			CHECK(geometry->get_mesh()->get_class() == (i == 0 ? "ArrayWireMesh4D" : "ArrayTetraMesh4D"));
			memdelete(root);
		}
		CHECK(!FileAccess::exists(destination.get_base_dir().path_join("c/shared/d" + extension)));
		// Export retains the source's polytope geometry, including when converted with a wire preference.
		Ref<G4MFState4D> shared;
		shared.instantiate();
		REQUIRE(document->import_read_from_file(shared, destination.get_base_dir().path_join("b/shared/d" + extension)) == OK);
		REQUIRE(shared->get_g4mf_meshes().size() == 1);
		const Ref<G4MFMesh4D> mesh = shared->get_g4mf_meshes()[0];
		CHECK(mesh->load_vertices(shared) == source_vertices);
		REQUIRE(mesh->get_surfaces().size() == 1);
		const Ref<G4MFMeshSurface4D> surface = mesh->get_surfaces()[0];
		CHECK(!surface->get_geometry_accessor_indices().is_empty());
	}
}

TEST_CASE("[G4MFModel4D] Sharing external sources respects different remaining nesting depths") {
	Node4D *scene = memnew(Node4D);
	scene->set_name("Root");
	scene->set_scene_file_path("res://source/root.scn");
	Node4D *branch = make_instance(scene, "Branch", "res://source/b/b.scn");
	for (Node *parent : Vector<Node *>{ branch, scene }) {
		Node4D *model = make_instance(parent, "D", "res://source/d.scn");
		make_instance(model, "E", "res://source/e.scn");
	}
	Ref<G4MFDocument4D> document;
	document.instantiate();
	document->set_max_nested_scene_depth(2);
	Ref<G4MFState4D> state;
	state.instantiate();
	REQUIRE(document->export_append_from_godot_scene(state, scene) == OK);
	memdelete(scene);
	state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES);
	const G4MFTestTempDirectory output("g4mf_shared_model_depth");
	REQUIRE(output.get_error() == OK);
	ERR_PRINT_OFF; // The generated branch name sanitizes its filename extension.
	const Error write_err = document->export_write_to_file(state, output.get_path().path_join("root.g4tf"));
	ERR_PRINT_ON;
	REQUIRE(write_err == OK);
	for (const String &model_path : Vector<String>{ "b/d.g4tf", "d.g4tf" }) {
		Ref<G4MFState4D> imported;
		imported.instantiate();
		REQUIRE(document->import_read_from_file(imported, output.get_path().path_join(model_path)) == OK);
		// D reached through B has no remaining nesting depth; direct D can still instance E.
		CHECK(imported->get_g4mf_models().size() == (model_path.begins_with("b/") ? 0 : 1));
	}
}

TEST_CASE("[SceneTree][G4MFModel4D] Converted models retain their preferred paths across output formats and folders") {
	bool use_gltf = false;
	SUBCASE("G4MF model") {}
#ifdef MODULE_GLTF_ENABLED
	SUBCASE("glTF model") {
		use_gltf = true;
	}
#endif
	Node4D *scene = memnew(Node4D);
	scene->set_name("Root");
	scene->set_scene_file_path("res://source/root.tscn");
	Node *instance = nullptr;
#ifdef MODULE_GLTF_ENABLED
	if (use_gltf) {
		MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
		Ref<BoxMesh> mesh;
		mesh.instantiate();
		mesh_instance->set_mesh(mesh);
		instance = mesh_instance;
	} else
#endif
	{
		instance = memnew(Node4D);
	}
	instance->set_name("Model");
	instance->set_scene_file_path("res://source/some/subfolder/model.detail.tscn");
	scene->add_child(instance);
	instance->set_owner(scene);
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	// Convert before choosing a destination, as the editor does.
	const Error convert_err = document->export_append_from_godot_scene(state, scene);
	REQUIRE(convert_err == OK);
	REQUIRE(state->get_g4mf_models().size() == 1);
	const Ref<G4MFModel4D> model = state->get_g4mf_models()[0];
	state->assign_unique_item_name(model, "StableModel");
	Dictionary override_properties;
	override_properties["visible"] = true;
	Dictionary overrides;
	overrides["Model"] = override_properties.duplicate();
	model->set_overrides(overrides);
	const int variant_index = G4MFModel4D::export_pack_nodes_into_model(document, state, instance);
	REQUIRE(variant_index > 0);
	const Ref<G4MFModel4D> variant = state->get_g4mf_models()[variant_index];
	override_properties["visible"] = false;
	Dictionary variant_overrides;
	variant_overrides["Model"] = override_properties.duplicate();
	variant->set_overrides(variant_overrides);
	state->assign_unique_item_name(variant, "Variant");
	CHECK(model->get_overrides() != variant->get_overrides());
	CHECK(variant->get_file_index() == model->get_file_index());
	CHECK(state->get_g4mf_file_references().size() == 1);
	const Ref<G4MFFileReference4D> file_ref = state->get_g4mf_file_references()[model->get_file_index()];
	CHECK(file_ref->get_file_uri().is_empty());
	const G4MFTestTempDirectory output("g4mf_model_paths");
	REQUIRE(output.get_error() == OK);
	const String output_dir = output.get_path();
	const Vector<String> destinations = {
		"one/root.g4b", "one/root.g4tf", "one/root.g4b", "one/embedded.g4tf",
		"two/root.g4tf", "two/root.g4b", "one/root.g4tf"
	};
	for (int i = 0; i < destinations.size(); i++) {
		state->reset_path_context();
		const bool embedded = i == 3;
		state->set_external_data_mode(embedded ? G4MFState4D::EXTERNAL_DATA_MODE_EMBED_EVERYTHING : G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES);
		const String destination = output_dir.path_join(destinations[i]);
		REQUIRE(document->export_write_to_file(state, destination) == OK);
		if (embedded) {
			CHECK(file_ref->get_file_uri().is_empty());
			CHECK(file_ref->get_buffer_view_index() >= 0);
		} else {
			const bool text = destinations[i].ends_with(".g4tf");
			const String extension = use_gltf ? (text ? ".gltf" : ".glb") : (text ? ".g4tf" : ".g4b");
			CHECK(file_ref->get_file_uri() == "some/subfolder/model.detail" + extension);
			CHECK(file_ref->get_buffer_view_index() == -1);
			CHECK(FileAccess::exists(destination.get_base_dir().path_join(file_ref->get_file_uri())));
		}
		CHECK(model->get_item_name() == "StableModel");
		CHECK(!file_ref->get_item_name().is_empty());
		CHECK(file_ref->get_item_name() != model->get_item_name());
		CHECK(file_ref->get_item_name() != variant->get_item_name());
		Ref<G4MFState4D> imported_state;
		imported_state.instantiate();
		ERR_PRINT_OFF; // Repeated embeddings retain unused buffer views whose alignment is inferred.
		const Error import_err = document->import_read_from_file(imported_state, destination);
		ERR_PRINT_ON;
		REQUIRE(import_err == OK);
		Node *imported = document->import_generate_godot_scene(imported_state);
		REQUIRE(imported != nullptr);
		CHECK(imported->get_node_or_null(NodePath("Model")) != nullptr);
#ifdef MODULE_GLTF_ENABLED
		if (use_gltf) {
			const TypedArray<Node> meshes = imported->find_children("*", "MeshInstance3D", true, false);
			CHECK(meshes.size() == 1);
			if (!meshes.is_empty()) {
				MeshInstance3D *imported_mesh = Object::cast_to<MeshInstance3D>(meshes[0]);
				CHECK(imported_mesh != nullptr);
				if (imported_mesh != nullptr) {
					CHECK(imported_mesh->get_mesh().is_valid());
				}
			}
		}
#endif
		memdelete(imported);
	}
	// glTF serialization may consult source nodes through document extensions.
	memdelete(scene);
}

TEST_CASE("[G4MFModel4D] Collision suffixes do not become preferred model names") {
	Node4D *scene = memnew(Node4D);
	scene->set_name("Root");
	scene->set_scene_file_path("res://source/root.tscn");
	make_instance(scene, "Model", "res://source/some/model.detail.tscn");
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	const Error convert_err = document->export_append_from_godot_scene(state, scene);
	memdelete(scene);
	REQUIRE(convert_err == OK);
	const Ref<G4MFModel4D> model = state->get_g4mf_models()[0];
	state->assign_unique_item_name(model, "StableModel");
	const Ref<G4MFFileReference4D> file_ref = state->get_g4mf_file_references()[model->get_file_index()];
	const G4MFTestTempDirectory output("g4mf_model_collision");
	REQUIRE(output.get_error() == OK);
	state->set_g4mf_file_path(output.get_path().path_join("root.g4b"));
	Ref<G4MFFileReference4D> blocker;
	blocker.instantiate();
	state->assign_unique_path_uri_from_stem(blocker, "some/model.detail", ".g4b", false);
	ERR_PRINT_OFF; // The first preparation intentionally encounters the reserved name.
	const Error first_err = model->export_prepare_for_serialization(state);
	ERR_PRINT_ON;
	REQUIRE(first_err == OK);
	CHECK(file_ref->get_file_uri() == "some/model2.detail.g4b");
	state->assign_unique_path_uri_from_stem(blocker, String(), String(), false);
	REQUIRE(model->export_prepare_for_serialization(state) == OK);
	CHECK(file_ref->get_file_uri() == "some/model2.detail.g4b");
	state->reset_path_context();
	REQUIRE(model->export_prepare_for_serialization(state) == OK);
	CHECK(file_ref->get_file_uri() == "some/model.detail.g4b");
	CHECK(model->get_item_name() == "StableModel");
}

TEST_CASE("[G4MFModel4D] Embedded models without a base path import without logging errors") {
	bool use_buffer_view = false;
	SUBCASE("Data URI") {}
	SUBCASE("Buffer view") {
		use_buffer_view = true;
	}
	const PackedByteArray payload = String("{\"asset\":{\"dimension\":4},\"nodes\":[{\"name\":\"InlineRoot\"}]}").to_utf8_buffer();
	Ref<G4MFState4D> state;
	state.instantiate();
	Ref<G4MFFileReference4D> file_ref;
	file_ref.instantiate();
	file_ref->set_mime_type("model/g4mf+json");
	if (use_buffer_view) {
		const int buffer_view_index = G4MFBufferView4D::write_new_buffer_view_into_state(state, payload);
		REQUIRE(buffer_view_index >= 0);
		file_ref->set_buffer_view_index(buffer_view_index);
	} else {
		file_ref->set_file_uri("data:model/g4mf+json;base64," + CoreBind::Marshalls::get_singleton()->raw_to_base64(payload));
	}
	TypedArray<G4MFFileReference4D> files;
	files.append(file_ref);
	state->set_g4mf_file_references(files);
	Ref<G4MFModel4D> model;
	model.instantiate();
	model->set_file_index(0);
	REQUIRE(state->get_g4mf_base_path().is_empty());
	ErrorDetector errors;
	REQUIRE(model->import_parse_file_data(state) == OK);
	CHECK_FALSE(errors.has_error);
	errors.clear();
	Node *instance = model->import_instantiate_model(state);
	REQUIRE(instance != nullptr);
	CHECK_FALSE(errors.has_error);
	CHECK(instance->get_name() == StringName("InlineRoot"));
	CHECK(instance->get_scene_file_path().is_empty());
	model->import_finalize_instance(instance);
	memdelete(instance);
	CHECK_FALSE(errors.has_error);
}

TEST_CASE("[G4MFModel4D] Imported model URIs seed a persistent stem before embedding") {
	String source_extension = ".g4tf";
	bool share_file_reference = false;
	SUBCASE("Single extension") {}
	SUBCASE("Compound text extension") {
		source_extension = ".g4tf.json";
	}
	SUBCASE("Alternative compound text extension") {
		source_extension = ".g4mf.json";
	}
	SUBCASE("Uppercase compound text extension") {
		source_extension = ".G4TF.JSON";
	}
	SUBCASE("Two models share the file reference") {
		share_file_reference = true;
	}
	const G4MFTestTempDirectory output("g4mf_imported_model_paths");
	REQUIRE(output.get_error() == OK);
	const String output_dir = output.get_path();
	const String source_uri = "some/subfolder/model.detail" + source_extension;
	const String source_file = output_dir.path_join("input").path_join(source_uri);
	REQUIRE(DirAccess::make_dir_recursive_absolute(source_file.get_base_dir()) == OK);
	const Ref<FileAccess> source = FileAccess::open(source_file, FileAccess::WRITE);
	REQUIRE(source.is_valid());
	source->store_string("{\"asset\":{\"dimension\":4},\"nodes\":[{\"name\":\"Root\"}]}");
	source->close();
	Ref<G4MFDocument4D> document;
	document.instantiate();
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path(output_dir.path_join("input/parent.g4tf"));
	Ref<G4MFFileReference4D> file_ref;
	file_ref.instantiate();
	file_ref->set_file_uri_no_sanitize(source_uri);
	file_ref->set_mime_type("model/g4mf+json");
	TypedArray<G4MFFileReference4D> files;
	files.append(file_ref);
	state->set_g4mf_file_references(files);
	Ref<G4MFModel4D> model;
	model.instantiate();
	model->set_file_index(0);
	model->set_model_g4mf_document(document);
	TypedArray<G4MFModel4D> models;
	models.append(model);
	state->set_g4mf_models(models);
	state->assign_unique_item_name(model, "ImportedModel");
	REQUIRE(model->import_parse_file_data(state) == OK);
	if (share_file_reference) {
		Ref<G4MFModel4D> second_model;
		second_model.instantiate();
		second_model->set_file_index(0);
		second_model->set_model_g4mf_document(document);
		models.append(second_model);
		state->set_g4mf_models(models);
		state->assign_unique_item_name(second_model, "SecondModel");
		REQUIRE(second_model->import_parse_file_data(state) == OK);
	}
	state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_EMBED_EVERYTHING);
	REQUIRE(document->export_write_to_file(state, output_dir.path_join("out/embedded.g4b")) == OK);
	CHECK(file_ref->get_file_uri().is_empty());
	state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES);
	for (const String &extension : Vector<String>{ ".g4tf", ".g4b", ".g4tf" }) {
		const String destination = output_dir.path_join("out/root" + extension);
		REQUIRE(document->export_write_to_file(state, destination) == OK);
		CHECK(file_ref->get_file_uri() == "some/subfolder/model.detail" + extension);
		CHECK(file_ref->get_buffer_view_index() == -1);
		CHECK(FileAccess::exists(destination.get_base_dir().path_join(file_ref->get_file_uri())));
		CHECK(model->get_item_name() == "ImportedModel");
	}
}
} // namespace TestG4MFModel4D
