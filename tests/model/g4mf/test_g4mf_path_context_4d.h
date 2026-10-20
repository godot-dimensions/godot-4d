#pragma once

#include "../../../model/g4mf/structures/internal/g4mf_path_context_4d.h"

#include "core/config/project_settings.h"
#include "tests/test_macros.h"

namespace TestG4MFPathContext4D {

TEST_CASE("[G4MFPathContext4D] Anonymous destinations keep their requested paths") {
	Ref<G4MFPathContext4D> context;
	context.instantiate();

	SUBCASE("Absolute destinations") {
		CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "/g4mf/export/first.g4tf", false) == "/g4mf/export/first.g4tf");
		CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "/g4mf/export/second.g4tf", false) == "/g4mf/export/second.g4tf");
	}
	SUBCASE("Relative destinations") {
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "/g4mf/export/main.g4tf", "first.g4tf", false) == "first.g4tf");
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "/g4mf/export/main.g4tf", "second.g4tf", false) == "second.g4tf");
	}
}

TEST_CASE("[G4MFPathContext4D] Relative sources do not acquire an anonymous identity") {
	Ref<G4MFPathContext4D> context;
	context.instantiate();

	SUBCASE("Absolute destinations") {
		CHECK(context->reserve_destination_abs_path_for_source(String(), "usage", "/g4mf/export/anonymous.g4tf", false) == "/g4mf/export/anonymous.g4tf");
		ERR_PRINT_OFF;
		const String relative_source = context->reserve_destination_abs_path_for_source(".", "usage", "/g4mf/export/sourced.g4tf", false);
		ERR_PRINT_ON;
		CHECK(relative_source.is_empty());
		// A usage context can still identify a reusable output without a source file.
		CHECK(context->reserve_destination_abs_path_for_source(String(), "usage", String(), false) == "/g4mf/export/anonymous.g4tf");
		CHECK(context->reserve_destination_abs_path_for_source("/g4mf/source/model.tscn", "usage", "/g4mf/export/sourced.g4tf", false) == "/g4mf/export/sourced.g4tf");
	}
	SUBCASE("Relative destinations") {
		CHECK(context->reserve_destination_path_for_source(String(), String(), "usage", "/g4mf/export/main.g4tf", "anonymous.g4tf", false) == "anonymous.g4tf");
		ERR_PRINT_OFF;
		const String relative_source = context->reserve_destination_path_for_source(".", String(), "usage", "/g4mf/export/main.g4tf", "sourced.g4tf", false);
		ERR_PRINT_ON;
		CHECK(relative_source.is_empty());
		CHECK(context->reserve_destination_path_for_source(String(), String(), "usage", "/g4mf/export/main.g4tf", String(), false) == "anonymous.g4tf");
		CHECK(context->reserve_destination_path_for_source("/g4mf/source/model.tscn", String(), "usage", "/g4mf/export/main.g4tf", "sourced.g4tf", false) == "sourced.g4tf");
	}
}

TEST_CASE("[G4MFPathContext4D] Every anonymous destination remains reserved") {
	Ref<G4MFPathContext4D> context;
	context.instantiate();

	SUBCASE("Absolute destinations") {
		CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "/g4mf/export/first.g4tf", false) == "/g4mf/export/first.g4tf");
		CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "/g4mf/export/second.g4tf", false) == "/g4mf/export/second.g4tf");
		CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "/g4mf/export/first.g4tf", false) == "/g4mf/export/first2.g4tf");
		CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "/g4mf/export/second.g4tf", false) == "/g4mf/export/second2.g4tf");
		CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "/g4mf/export/first.g4tf", false) == "/g4mf/export/first3.g4tf");
	}
	SUBCASE("Relative destinations") {
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "/g4mf/export/main.g4tf", "first.g4tf", false) == "first.g4tf");
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "/g4mf/export/main.g4tf", "second.g4tf", false) == "second.g4tf");
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "/g4mf/export/main.g4tf", "first.g4tf", false) == "first2.g4tf");
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "/g4mf/export/main.g4tf", "second.g4tf", false) == "second2.g4tf");
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "/g4mf/export/main.g4tf", "first.g4tf", false) == "first3.g4tf");
	}
	SUBCASE("Absolute and relative destinations share reservations") {
		CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "/g4mf/export/model.g4tf", false) == "/g4mf/export/model.g4tf");
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "/g4mf/export/main.g4tf", "model.g4tf", false) == "model2.g4tf");
		CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "/g4mf/export/model.g4tf", false) == "/g4mf/export/model3.g4tf");
	}
}

TEST_CASE("[G4MFPathContext4D] Explicit reservations recognize path aliases") {
	Ref<G4MFPathContext4D> context;
	context.instantiate();

	SUBCASE("Case, separators, and dot segments") {
		context->mark_path_as_reserved("/g4mf/export/sub/../ARCHIVE.tar.gz");
		context->mark_path_as_reserved("\\g4mf\\export\\archive2.tar.gz");
		CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "/g4mf/export/archive.tar.gz", false) == "/g4mf/export/archive3.tar.gz");
	}
	SUBCASE("Project and filesystem paths") {
		String absolute_path = ProjectSettings::get_singleton()->globalize_path("res://g4mf_paths/reserved.bin").replace("\\", "/");
		if (!absolute_path.is_absolute_path()) {
			// Native tests may run without a project filesystem mapping.
			absolute_path = "res://g4mf_paths/reserved.bin";
		}
		context->mark_path_as_reserved(absolute_path);
		CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "res://g4mf_paths/reserved.bin", false) == "res://g4mf_paths/reserved2.bin");
	}
}

TEST_CASE("[G4MFPathContext4D] Reusable and anonymous destinations share reservations") {
	Ref<G4MFPathContext4D> context;
	context.instantiate();

	CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "/g4mf/export/model.g4tf", false) == "/g4mf/export/model.g4tf");
	CHECK(context->reserve_destination_abs_path_for_source("/g4mf/source/model.tscn", "usage", "/g4mf/export/model.g4tf", false) == "/g4mf/export/model2.g4tf");
	CHECK(context->reserve_destination_abs_path_for_source("/g4mf/source/model.tscn", "usage", String(), false) == "/g4mf/export/model2.g4tf");
	CHECK(context->reserve_destination_path_for_source("/g4mf/source/model.tscn", String(), "other_usage", "/g4mf/export/main.g4tf", "model.g4tf", false) == "model3.g4tf");
	CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "/g4mf/export/model.g4tf", false) == "/g4mf/export/model4.g4tf");
}

TEST_CASE("[G4MFPathContext4D] Relative destinations can refer to parent directories") {
	Ref<G4MFPathContext4D> context;
	context.instantiate();

	CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "/g4mf/export/branch/main.g4tf", "../shared/tree.g4tf", false) == "../shared/tree.g4tf");
	CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "/g4mf/export/other/child.g4tf", String(), false) == "../shared/tree.g4tf");
	CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "/g4mf/export/branch/main.g4tf", "../shared/tree.g4tf", false) == "../shared/tree2.g4tf");
	// A matching filename is still part of the relative path, rather than a common directory.
	CHECK(context->reserve_destination_path_for_source("/source/self.tscn", String(), "usage", "/g4mf/export/main.g4tf", "main.g4tf", false) == "main.g4tf");
	CHECK(context->reserve_destination_path_for_source("/source/self.tscn", String(), "usage", "/g4mf/export/main.g4tf", String(), false) == "main.g4tf");
}

TEST_CASE("[G4MFPathContext4D] Project and filesystem destinations produce equivalent relative URIs") {
	Ref<G4MFPathContext4D> context;
	context.instantiate();

	SUBCASE("Relative allocation and reuse") {
		CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "res://out/main.g4tf", "models/tree.g4tf", false) == "models/tree.g4tf");
		CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "res://out/branch/child.g4tf", String(), false) == "../models/tree.g4tf");
	}
	SUBCASE("An absolute reservation can be used by a relative lookup") {
		CHECK(context->reserve_destination_abs_path_for_source("/source/tree.tscn", "usage", "res://out/models/tree.g4tf", false) == "res://out/models/tree.g4tf");
		CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "res://out/branch/child.g4tf", String(), false) == "../models/tree.g4tf");
		CHECK(context->reserve_destination_abs_path_for_source("/source/tree.tscn", "usage", String(), false) == "res://out/models/tree.g4tf");
	}
	SUBCASE("An absolute requested destination can use a protocol") {
		CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "res://out/main.g4tf", "res://out/models/tree.g4tf", false) == "models/tree.g4tf");
	}
	SUBCASE("Absolute and relative allocations retain the same extension sanitation") {
		CHECK(context->reserve_destination_abs_path_for_source(String(), String(), "res://out/models/packed.tar_gz", false) == "res://out/models/packed.targz");
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "res://out/main.g4tf", "models/packed.tar_gz", false) == "models/packed2.targz");
	}
}

TEST_CASE("[G4MFPathContext4D] Relative allocations preserve the containing directory spelling") {
	Ref<G4MFPathContext4D> context;
	context.instantiate();

	CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "/G4MF Exports/Keep-This/main.g4tf", "MyModels/MyTree.g4tf", false) == "my_models/my_tree.g4tf");
	CHECK(context->reserve_destination_abs_path_for_source("/source/tree.tscn", "usage", String(), false) == "/G4MF Exports/Keep-This/my_models/my_tree.g4tf");
	CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "/G4MF Exports/Keep-This/branch/child.g4tf", String(), false) == "../my_models/my_tree.g4tf");
}

TEST_CASE("[G4MFPathContext4D] Source-derived paths normalize protocols and dot segments") {
	Ref<G4MFPathContext4D> context;
	context.instantiate();
	String referencing_source = ProjectSettings::get_singleton()->globalize_path("res://sources/main.tscn");
	if (!referencing_source.is_absolute_path()) {
		referencing_source = "res://sources/main.tscn";
	}

	CHECK(context->reserve_destination_path_for_source("res://sources/models/./tree.tscn", referencing_source, "usage", "/g4mf/export/main.g4tf", String(), false) == "models/tree.tscn");
}

TEST_CASE("[G4MFPathContext4D] Preferred source paths use normalized directory relationships") {
	String referencing_source = ProjectSettings::get_singleton()->globalize_path("res://sources/main.tscn");
	if (!referencing_source.is_absolute_path()) {
		referencing_source = "res://sources/main.tscn";
	}
	CHECK(G4MFPathContext4D::get_source_placement_path("res://sources/models/./tree.tscn", referencing_source) == "models/tree.tscn");
	CHECK(G4MFPathContext4D::get_source_placement_path("res://sources/models/tree.tscn", String()) == "tree.tscn");
	CHECK(G4MFPathContext4D::get_source_placement_path(String(), referencing_source).is_empty());
	CHECK(G4MFPathContext4D::get_source_placement_path("D:/models/tree.tscn", "C:/sources/main.tscn") == "tree.tscn");
	CHECK(G4MFPathContext4D::get_source_placement_path("//server/other/models/tree.tscn", "//server/share/main.tscn") == "tree.tscn");
}

TEST_CASE("[G4MFPathContext4D] Relative destinations respect drive roots") {
	Ref<G4MFPathContext4D> context;
	context.instantiate();

	SUBCASE("Explicit destinations on another drive stay absolute and unique") {
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "C:/export/main.g4tf", "D:/Export/MyTree.g4tf", false) == "D:/export/my_tree.g4tf");
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "C:/export/main.g4tf", "D:/Export/MyTree.g4tf", false) == "D:/export/my_tree2.g4tf");
	}
	SUBCASE("Cached destinations stay absolute only for users on another root") {
		CHECK(context->reserve_destination_abs_path_for_source("/source/tree.tscn", "usage", "D:/export/tree.g4tf", false) == "D:/export/tree.g4tf");
		CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "C:/export/main.g4tf", String(), false) == "D:/export/tree.g4tf");
		CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "d:/export/branch/main.g4tf", String(), false) == "../tree.g4tf");
		CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "/export/main.g4tf", String(), false) == "D:/export/tree.g4tf");
	}
	SUBCASE("Sources on unrelated roots use only the filename") {
		CHECK(context->reserve_destination_path_for_source("D:/source/models/MyTree.tscn", "C:/source/main.tscn", "usage", "C:/export/main.g4tf", String(), false) == "my_tree.tscn");
		CHECK(context->reserve_destination_path_for_source("E:/source/models/MyTree.tscn", "C:/source/main.tscn", "usage", "C:/export/main.g4tf", String(), false) == "my_tree2.tscn");
		CHECK(context->reserve_destination_path_for_source("c:/source/models/MyTree.tscn", "C:/source/main.tscn", "usage", "C:/export/main.g4tf", String(), false) == "models/my_tree.tscn");
	}
}

TEST_CASE("[G4MFPathContext4D] Relative destinations respect network share roots") {
	Ref<G4MFPathContext4D> context;
	context.instantiate();

	SUBCASE("Different shares and servers require absolute paths") {
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "//server/first/export/main.g4tf", "//server/second/export/tree.g4tf", false) == "//server/second/export/tree.g4tf");
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "//server/first/export/main.g4tf", "//other/first/export/tree.g4tf", false) == "//other/first/export/tree.g4tf");
		CHECK(context->reserve_destination_path_for_source(String(), String(), String(), "//server/first/export/main.g4tf", "/export/tree.g4tf", false) == "/export/tree.g4tf");
	}
	SUBCASE("Cached destinations retain their server and share") {
		CHECK(context->reserve_destination_abs_path_for_source("/source/tree.tscn", "usage", "//server/second/export/tree.g4tf", false) == "//server/second/export/tree.g4tf");
		CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "//server/first/export/main.g4tf", String(), false) == "//server/second/export/tree.g4tf");
		CHECK(context->reserve_destination_path_for_source("/source/tree.tscn", String(), "usage", "//SERVER/SECOND/export/branch/main.g4tf", String(), false) == "../tree.g4tf");
	}
	SUBCASE("Sources on unrelated shares use only the filename") {
		CHECK(context->reserve_destination_path_for_source("//server/second/models/MyTree.tscn", "//server/first/main.tscn", "usage", "/export/main.g4tf", String(), false) == "my_tree.tscn");
		CHECK(context->reserve_destination_path_for_source("//other/first/models/MyTree.tscn", "//server/first/main.tscn", "usage", "/export/main.g4tf", String(), false) == "my_tree2.tscn");
		CHECK(context->reserve_destination_path_for_source("//SERVER/FIRST/models/MyTree.tscn", "//server/first/main.tscn", "usage", "/export/main.g4tf", String(), false) == "models/my_tree.tscn");
	}
}

} // namespace TestG4MFPathContext4D
