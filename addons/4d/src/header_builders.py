# Copy of header_builders.py from https://github.com/godotengine/godot-cpp/pull/1789
import os.path


MAX_RAW_STRING_BYTES = 16000


## See https://github.com/godotengine/godot/blob/master/glsl_builders.py
class RAWHeaderStruct:
	def __init__(self):
		self.code = ""


def get_raw_header_dependencies(filename: str, dependencies=None):
	if dependencies is None:
		dependencies = []
	with open(filename, "r", encoding="utf-8") as source_file:
		for line in source_file:
			if line.find("#include ") == -1:
				continue
			include_line = line.replace("#include ", "").strip()[1:-1]
			included_file = os.path.relpath(os.path.dirname(filename) + "/" + include_line)
			if included_file not in dependencies:
				dependencies.append(included_file)
				get_raw_header_dependencies(included_file, dependencies)
	return dependencies


def validate_raw_string_size(source_filename: str, code: str) -> None:
	# MSVC limits narrow string literals to 16,380 bytes, and treats a literal containing
	# any non-ASCII characters as two bytes per character. Keep a safety margin and fail
	# on every platform so a generated header cannot work locally but fail on Windows.
	windows_bytes_per_character = 1 if code.isascii() else 2
	estimated_windows_byte_count = len(code) * windows_bytes_per_character
	if estimated_windows_byte_count > MAX_RAW_STRING_BYTES:
		raise ValueError(
			f'Expanded shader "{source_filename}" is too large for one Windows C++ string literal: '
			f"{estimated_windows_byte_count} estimated bytes exceeds the {MAX_RAW_STRING_BYTES}-byte safety limit. "
			"Split the shader into generated constants and concatenate them at runtime."
		)


def include_file_in_raw_header(filename: str, header_data: RAWHeaderStruct, depth: int) -> None:
	with open(filename, "r", encoding="utf-8") as source_file:
		line = source_file.readline()

		while line:
			while line.find("#include ") != -1:
				include_line = line.replace("#include ", "").strip()[1:-1]
				included_file = os.path.relpath(os.path.dirname(filename) + "/" + include_line)
				include_file_in_raw_header(included_file, header_data, depth + 1)
				line = source_file.readline()

			header_data.code += line
			line = source_file.readline()


def build_raw_header(source_filename: str, constant_name: str) -> None:
	include_file_in_raw_header(source_filename, header_data := RAWHeaderStruct(), 0)
	validate_raw_string_size(source_filename, header_data.code)
	constant_name = constant_name.replace(".", "_")
	# Build header content using a C raw string literal.
	header_content = (
		f'/* THIS FILE IS GENERATED. EDITS WILL BE LOST. */\n\n#pragma once\n\ninline constexpr const char *{constant_name} = R"<!>({header_data.code})<!>";\n'
	)
	# Write the header to the provided file name with a ".gen.h" suffix.
	header_filename = f"{source_filename}.gen.h"
	with open(header_filename, "w") as header_file:
		header_file.write(header_content)


def build_raw_headers_action(target, source, env):
	env.NoCache(target)
	for src in source:
		source_filename = str(src)
		# To match Godot, replace ".glsl" with "_shader_glsl". Does nothing for non-GLSL files.
		constant_name = os.path.basename(source_filename).replace(".glsl", "_shader_glsl")
		build_raw_header(source_filename, constant_name)


def escape_svg(filename: str) -> str:
	with open(filename, encoding="utf-8", newline="\n") as svg_file:
		svg_content = svg_file.read()
		return f'R"<!>({svg_content})<!>"'


## See https://github.com/godotengine/godot/blob/master/editor/icons/editor_icons_builders.py
## See https://github.com/godotengine/godot/blob/master/scene/theme/icons/default_theme_icons_builders.py
def make_svg_icons_action(target, source, env):
	destination = str(target[0])
	constant_prefix = os.path.basename(destination).replace(".gen.h", "")
	svg_icons = [str(x) for x in source]
	# Convert the SVG icons to escaped strings and convert their names to C strings.
	icon_names = [f'"{os.path.basename(fname)[:-4]}"' for fname in svg_icons]
	icon_sources = [escape_svg(fname) for fname in svg_icons]
	# Join them as indented comma-separated items for use in an array initializer.
	icon_names_str = ",\n\t".join(icon_names)
	icon_sources_str = ",\n\t".join(icon_sources)
	# Write the file to disk.
	with open(destination, "w", encoding="utf-8", newline="\n") as destination_file:
		destination_file.write(
			f"""\
/* THIS FILE IS GENERATED. EDITS WILL BE LOST. */

#pragma once

inline constexpr int {constant_prefix}_count = {len(icon_names)};
inline constexpr const char *{constant_prefix}_sources[] = {{
	{icon_sources_str}
}};

inline constexpr const char *{constant_prefix}_names[] = {{
	{icon_names_str}
}};
"""
		)
