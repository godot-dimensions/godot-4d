#pragma once

#include "../../../model/g4mf/g4mf_state_4d.h"

#include "tests/test_macros.h"

namespace TestG4MFState4D {

TEST_CASE("[G4MFState4D] Path reservations retain sanitized filenames and extensions") {
	Ref<G4MFState4D> state;
	state.instantiate();

	SUBCASE("With only a base directory") {
		state->set_g4mf_base_path("res://out");
		CHECK(state->reserve_unique_path_uri_from_stem("SomeFolder/MyFile", ".PNG", false) == "some_folder/my_file.png");
		CHECK(state->reserve_unique_path_uri_from_stem("SomeFolder/MyFile", ".PNG", false) == "some_folder/my_file2.png");
		CHECK(state->reserve_unique_path_uri_from_stem("archive", "tar.gz", false) == "archive.tar.gz");
		CHECK(state->reserve_unique_path_uri_from_stem("archive", "tar.gz", false) == "archive2.tar.gz");
	}
	SUBCASE("With a destination") {
		state->set_g4mf_file_path("res://out/main.g4tf");
		Ref<G4MFFileReference4D> file;
		file.instantiate();
		state->assign_unique_path_uri_from_stem(file, "SomeFolder/MyFile", ".PNG", false);
		CHECK(file->resolve_file_uri(state->get_g4mf_base_path()) == "res://out/some_folder/my_file.png");
		state->assign_unique_path_uri_from_stem(file, "archive", "tar.gz", false);
		CHECK(file->resolve_file_uri(state->get_g4mf_base_path()) == "res://out/archive.tar.gz");
	}
}

TEST_CASE("[G4MFState4D] Relative reservations preserve the containing directory") {
	Ref<G4MFState4D> state;
	state.instantiate();

	SUBCASE("With a destination filename") {
		state->set_g4mf_file_path("res://G4MF Exports/Keep-This/main.g4tf");
	}
	SUBCASE("With only a base directory") {
		state->set_g4mf_base_path("res://G4MF Exports/Keep-This");
	}

	CHECK(state->reserve_unique_path_uri_from_stem("SomeFolder/MyFile", ".PNG", false) == "some_folder/my_file.png");
	CHECK(state->reserve_unique_path_uri_from_stem("SomeFolder/MyFile", ".PNG", false) == "some_folder/my_file2.png");
	state->mark_path_as_reserved("some_folder/my_file3.png");
	CHECK(state->reserve_unique_path_uri_from_stem("SomeFolder/MyFile", ".PNG", false) == "some_folder/my_file4.png");
	CHECK(state->reserve_unique_path_uri_from_stem("../shared/archive", "tar.gz", false) == "../shared/archive.tar.gz");
	CHECK(state->reserve_unique_path_uri_from_stem("../shared/archive", "tar.gz", false) == "../shared/archive2.tar.gz");

	Ref<G4MFFileReference4D> file;
	file.instantiate();
	state->assign_unique_path_uri_from_stem(file, "SomeFolder/MyFile", ".PNG", false);
	CHECK(file->get_file_uri() == "some_folder/my_file5.png");
	CHECK(file->resolve_file_uri(state->get_g4mf_base_path()) == "res://G4MF Exports/Keep-This/some_folder/my_file5.png");
}

TEST_CASE("[G4MFState4D] Relative paths require an explicit base and failed assignments preserve ownership") {
	Ref<G4MFState4D> state;
	state.instantiate();
	ERR_PRINT_OFF;
	const String unanchored_path = state->reserve_unique_path_uri_from_stem("image", ".png", false);
	state->mark_path_as_reserved("image.png");
	ERR_PRINT_ON;
	CHECK(unanchored_path.is_empty());
	CHECK(state->reserve_unique_path_uri_from_stem("user://g4mf_test/image", ".png", false) == "user://g4mf_test/image.png");
	state->set_g4mf_base_path("res://out");
	Ref<G4MFFileReference4D> file;
	file.instantiate();
	state->assign_unique_path_uri_from_stem(file, "image", ".png", false);
	REQUIRE(file->get_file_uri() == "image.png");
	state->set_g4mf_base_path(String());
	ERR_PRINT_OFF;
	state->assign_unique_path_uri_from_stem(file, "other", ".png", false);
	const String unresolved_uri = file->resolve_file_uri(String());
	ERR_PRINT_ON;
	CHECK(file->get_file_uri() == "image.png");
	CHECK(unresolved_uri.is_empty());
	state->set_g4mf_base_path("res://out");
	CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image2.png");
	state->set_g4mf_base_path(String());
	state->assign_unique_path_uri_from_stem(file, String(), String(), false);
	CHECK(file->get_file_uri().is_empty());
	state->set_g4mf_base_path("res://out");
	CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image.png");
}

TEST_CASE("[G4MFState4D] Rooted state paths cannot be replaced with relative paths") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("res://out/main.g4tf");
	state->set_source_path("res://source/main.tscn");
	ERR_PRINT_OFF;
	state->set_g4mf_file_path("out/other.g4tf");
	state->set_g4mf_base_path("other");
	state->set_source_path("source/other.tscn");
	ERR_PRINT_ON;
	CHECK(state->get_g4mf_file_path() == "res://out/main.g4tf");
	CHECK(state->get_source_path() == "res://source/main.tscn");
}

TEST_CASE("[G4MFState4D] Reservations in shared contexts use their resolved destinations") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_base_path("/g4mf/export");
	CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image.png");
	CHECK(state->reserve_unique_path_uri_from_stem("/g4mf/export/image", ".png", false) == "/g4mf/export/image2.png");
	Ref<G4MFState4D> child;
	child.instantiate();
	child->use_path_context_of(state);
	child->set_g4mf_file_path("/g4mf/export/branch/model.g4tf");
	CHECK(child->reserve_unique_path_uri_from_stem("../image", ".png", false) == "../image3.png");
	CHECK(child->reserve_unique_path_uri_from_stem("image", ".png", false) == "image.png");
}

TEST_CASE("[G4MFState4D] Destination usages follow the containing file across saves") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("res://Case-Container/first/scene.g4tf");
	CHECK(state->reserve_destination_usage_path("buffer:0", "scene_buffer0", ".bin", false) == "scene_buffer0.bin");
	state->set_g4mf_file_path("res://Case-Container/second/scene.g4tf");
	CHECK(state->reserve_destination_usage_path("buffer:0", "scene_buffer0", ".bin", false) == "scene_buffer0.bin");
	state->set_g4mf_file_path("res://Case-Container/first/./scene.g4tf");
	CHECK(state->reserve_destination_usage_path("buffer:0", "scene_buffer0", ".bin", false) == "scene_buffer0.bin");
	// Buffer indices and containing-file extensions have independent allocations even with the same preferred name.
	CHECK(state->reserve_destination_usage_path("buffer:1", "scene_buffer0", ".bin", false) == "scene_buffer02.bin");
	state->set_g4mf_file_path("res://Case-Container/first/scene.g4b");
	CHECK(state->reserve_destination_usage_path("buffer:0", "scene_buffer0", ".bin", false) == "scene_buffer03.bin");
	CHECK(state->reserve_destination_usage_path("buffer:0", "scene_buffer0", ".bin", false) == "scene_buffer03.bin");
	state->set_g4mf_file_path("res://Case-Container/first/SCENE.g4tf");
	CHECK(state->reserve_destination_usage_path("buffer:0", "scene_buffer0", ".bin", false) == "scene_buffer0.bin");
}

TEST_CASE("[G4MFState4D] Destination usages retain collision suffixes and dotted stems") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("res://Case-Container/scene.v2.g4tf");
	state->mark_path_as_reserved("scene.v2_buffer0.bin");
	CHECK(state->reserve_destination_usage_path("buffer:0", "scene.v2_buffer0", ".bin", false) == "scene.v2_buffer02.bin");
	CHECK(state->reserve_destination_usage_path("buffer:0", "scene.v2_buffer0", ".bin", false) == "scene.v2_buffer02.bin");
	CHECK(state->reserve_unique_path_uri_from_stem("scene.v2_buffer0", ".bin", false) == "scene.v2_buffer03.bin");
}

TEST_CASE("[G4MFState4D] Resetting a path context detaches only this state") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("res://out/main.g4tf");
	Ref<G4MFFileReference4D> file;
	file.instantiate();
	state->assign_unique_path_uri_from_stem(file, "image", ".png", false);
	state->assign_unique_item_name(file, "image", false);
	CHECK(state->reserve_destination_path_for_source("res://source/model.tscn", "usage", "shared.g4tf", false) == "shared.g4tf");
	CHECK(state->reserve_destination_usage_path("buffer:0", "main_buffer0", ".bin", false) == "main_buffer0.bin");
	Ref<G4MFState4D> child;
	child.instantiate();
	child->set_g4mf_file_path(state->get_g4mf_file_path());
	child->use_path_context_of(state);

	state->reset_path_context();
	state->reset_path_context(); // Resetting an absent context is harmless.
	CHECK(file->get_file_uri() == "image.png");
	CHECK(state->get_item_by_name("image") == file);
	CHECK(state->get_g4mf_file_path() == "res://out/main.g4tf");
	CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image.png");
	CHECK(child->reserve_unique_path_uri_from_stem("image", ".png", false) == "image2.png");
	CHECK(state->reserve_destination_path_for_source("res://source/model.tscn", "usage", "fresh.g4tf", false) == "fresh.g4tf");
	CHECK(child->reserve_destination_path_for_source("res://source/model.tscn", "usage", "fresh.g4tf", false) == "shared.g4tf");
	CHECK(state->reserve_destination_usage_path("buffer:0", "fresh_buffer0", ".bin", false) == "fresh_buffer0.bin");
	CHECK(child->reserve_destination_usage_path("buffer:0", "fresh_buffer0", ".bin", false) == "main_buffer0.bin");
	child->use_path_context_of(state);
	CHECK(child->reserve_destination_usage_path("buffer:0", "fresh_buffer0", ".bin", false) == "fresh_buffer0.bin");
}

TEST_CASE("[G4MFState4D] Sharing a path context also inherits the external data mode") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("/g4mf/export/main.g4tf");
	state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_EMBED_EVERYTHING);
	CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image.png");
	Ref<G4MFState4D> other;
	other.instantiate();
	other->set_g4mf_file_path("/g4mf/export/other.g4tf");
	other->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_ALL_FILES);
	other->use_path_context_of(state);
	CHECK(other->get_external_data_mode() == G4MFState4D::EXTERNAL_DATA_MODE_EMBED_EVERYTHING);
	CHECK(other->get_g4mf_file_path() == "/g4mf/export/other.g4tf");
	CHECK(other->reserve_unique_path_uri_from_stem("image", ".png", false) == "image2.png");
	ERR_PRINT_OFF;
	other->use_path_context_of(Ref<G4MFState4D>());
	ERR_PRINT_ON;
	CHECK(other->reserve_unique_path_uri_from_stem("image", ".png", false) == "image3.png");
}

TEST_CASE("[G4MFState4D] Source and destination usages have independent identities") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("/g4mf/export/main.g4tf");
	CHECK(state->reserve_destination_path_for_source(state->get_g4mf_file_path(), "usage", "shared.bin", false) == "shared.bin");
	CHECK(state->reserve_destination_usage_path("usage", "local", ".bin", false) == "local.bin");
	CHECK(state->reserve_destination_path_for_source(state->get_g4mf_file_path(), "usage", "other.bin", false) == "shared.bin");
	CHECK(state->reserve_destination_usage_path("usage", "other", ".bin", false) == "local.bin");
	// Separate usage identities still participate in the same path collision checks.
	CHECK(state->reserve_unique_path_uri_from_stem("shared", ".bin", false) == "shared2.bin");
	CHECK(state->reserve_unique_path_uri_from_stem("local", ".bin", false) == "local2.bin");
}

TEST_CASE("[G4MFState4D] Relative source usages require a containing filename") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_base_path("res://out");
	ERR_PRINT_OFF;
	const String invalid_path = state->reserve_destination_path_for_source("res://source/model.tscn", "usage", "model.g4tf", false);
	ERR_PRINT_ON;
	CHECK(invalid_path.is_empty());
	CHECK(state->reserve_destination_path_for_source("res://source/model.tscn", "usage", "res://out/models/model.g4tf", false) == "res://out/models/model.g4tf");
	state->set_g4mf_file_path("res://out/main.g4tf");
	CHECK(state->reserve_destination_path_for_source("res://source/model.tscn", "usage", "model.g4tf", false) == "models/model.g4tf");
}

TEST_CASE("[G4MFState4D] Combined filename assignments preserve ownership and multipart extensions") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("res://out/main.g4tf");
	Ref<G4MFFileReference4D> file;
	file.instantiate();
	file->set_unique_file_path_uri(state, "Models/archive.tar.gz", false);
	CHECK(file->get_file_uri() == "models/archive.tar.gz");
	CHECK(state->reserve_unique_path_uri_from_stem("models/archive", ".tar.gz", false) == "models/archive2.tar.gz");
	file->set_unique_file_path_uri(state, "Models/archive.tar.gz", false);
	CHECK(file->get_file_uri() == "models/archive.tar.gz");
	ERR_PRINT_OFF;
	file->set_unique_file_path_uri(state, "https://example.com/model.g4tf", false);
	ERR_PRINT_ON;
	CHECK(file->get_file_uri() == "models/archive.tar.gz");
	file->set_unique_file_path_uri(state, String(), false);
	CHECK(file->get_file_uri().is_empty());
	CHECK(state->reserve_unique_path_uri_from_stem("models/archive", ".tar.gz", false) == "models/archive.tar.gz");
}

TEST_CASE("[G4MFState4D] Empty path requests clear file URIs with a destination set") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("res://out/main.g4tf");
	CHECK(state->reserve_unique_path_uri_from_stem(String(), String(), false).is_empty());
	// An empty stem with an extension still requests a file.
	CHECK(state->reserve_unique_path_uri_from_stem(String(), ".PNG", false).get_file() == ".png");

	Ref<G4MFFileReference4D> file;
	file.instantiate();
	state->assign_unique_path_uri_from_stem(file, "image", ".bin", false);
	const String assigned_uri = file->get_file_uri();
	REQUIRE(!assigned_uri.is_empty());
	ERR_PRINT_OFF;
	state->assign_unique_path_uri_from_stem(file, "https://example.com/image", ".png", false);
	ERR_PRINT_ON;
	CHECK(file->get_file_uri() == assigned_uri);
	state->assign_unique_path_uri_from_stem(file, String(), String(), false);
	CHECK(file->get_file_uri().is_empty());
}

TEST_CASE("[G4MFState4D] Assignments release only the file reference's owned path") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("res://owned/main.g4tf");
	Ref<G4MFFileReference4D> file;
	file.instantiate();
	state->assign_unique_path_uri_from_stem(file, "image", ".png", false);
	REQUIRE(file->get_file_uri() == "image.png");

	SUBCASE("Reassigning the current name") {
		state->assign_unique_path_uri_from_stem(file, "IMAGE", ".PNG", false);
		CHECK(file->get_file_uri() == "image.png");
		CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image2.png");
	}
	SUBCASE("Renaming") {
		state->assign_unique_path_uri_from_stem(file, "other", ".png", false);
		CHECK(file->get_file_uri() == "other.png");
		CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image.png");
	}
	SUBCASE("Clearing after a direct URI change") {
		file->set_file_uri_no_sanitize("untracked.png");
		state->assign_unique_path_uri_from_stem(file, String(), String(), false);
		CHECK(file->get_file_uri().is_empty());
		CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image.png");
	}
	SUBCASE("Invalid input leaves ownership intact") {
		ERR_PRINT_OFF;
		state->assign_unique_path_uri_from_stem(file, "https://example.com/image", ".png", false);
		ERR_PRINT_ON;
		CHECK(file->get_file_uri() == "image.png");
		CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image2.png");
	}
	SUBCASE("An explicit reservation outlives ownership") {
		state->mark_path_as_reserved("image.png");
		state->assign_unique_path_uri_from_stem(file, String(), String(), false);
		CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image2.png");
	}
	SUBCASE("Another file keeps its reservation") {
		Ref<G4MFFileReference4D> other;
		other.instantiate();
		state->assign_unique_path_uri_from_stem(other, "image", ".png", false);
		REQUIRE(other->get_file_uri() == "image2.png");
		state->assign_unique_path_uri_from_stem(other, "image", ".png", false);
		CHECK(other->get_file_uri() == "image2.png");
		state->assign_unique_path_uri_from_stem(file, String(), String(), false);
		CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image.png");
		CHECK(state->reserve_unique_path_uri_from_stem("image2", ".png", false) == "image22.png");
	}
	SUBCASE("A base directory change does not change the released key") {
		state->set_g4mf_base_path("res://other");
		state->assign_unique_path_uri_from_stem(file, String(), String(), false);
		Ref<G4MFState4D> original_directory;
		original_directory.instantiate();
		original_directory->use_path_context_of(state);
		original_directory->set_g4mf_base_path("res://owned");
		CHECK(original_directory->reserve_unique_path_uri_from_stem("image", ".png", false) == "image.png");
	}
}

TEST_CASE("[G4MFState4D] Clearing an assigned path preserves anonymous and shared reservations") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("res://owned/main.g4tf");
	SUBCASE("Anonymous reservation") {
		CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image.png");
	}
	SUBCASE("Shared source reservation") {
		CHECK(state->reserve_destination_path_for_source("res://source/image.png", "usage", "image.png", false) == "image.png");
	}
	Ref<G4MFFileReference4D> file;
	file.instantiate();
	state->assign_unique_path_uri_from_stem(file, "image", ".png", false);
	REQUIRE(file->get_file_uri() == "image2.png");
	state->assign_unique_path_uri_from_stem(file, String(), String(), false);
	CHECK(file->get_file_uri().is_empty());
	CHECK(state->reserve_unique_path_uri_from_stem("image", ".png", false) == "image2.png");
}

TEST_CASE("[G4MFState4D] Embedding file bytes clears the URI with a destination set") {
	Ref<G4MFState4D> state;
	state.instantiate();
	state->set_g4mf_file_path("res://out/main.g4b");
	Ref<G4MFFileReference4D> file;
	file.instantiate();
	state->assign_unique_path_uri_from_stem(file, "image", ".bin", false);
	REQUIRE(!file->get_file_uri().is_empty());
	PackedByteArray bytes;
	bytes.push_back(1);
	bytes.push_back(2);
	bytes.push_back(3);
	bytes.push_back(4);
	file->export_store_file_bytes(bytes);
	REQUIRE(file->export_write_to_buffer(state) == OK);
	CHECK(file->get_file_uri().is_empty());
	CHECK(file->get_buffer_view_index() >= 0);
	CHECK(file->import_read_file_data(state, false) == bytes);
}

} // namespace TestG4MFState4D
