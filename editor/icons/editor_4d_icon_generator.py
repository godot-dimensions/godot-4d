from io import StringIO
import os


# See also `editor/icons/editor_icons_builders.py`.
# See also `scene/theme/icons/default_theme_icons_builders.py`.
def make_editor_4d_icons_action(target, source, env):
	dst = str(target[0])
	svg_icons = [str(x) for x in source]

	with StringIO() as s:
		s.write("/* THIS FILE IS GENERATED DO NOT EDIT */\n\n")
		s.write("#pragma once\n")
		# Write the SVG icon sources as an escaped array of C strings.
		s.write("\nstatic const char *editor_4d_icon_sources[] = {\n")
		for f in svg_icons:
			fname = str(f)
			s.write('\t"')
			with open(fname, "rb") as svgf:
				b = svgf.read(1)
				while len(b) == 1:
					if ord(b) != 10:
						s.write("\\" + str(hex(ord(b)))[1:])
					b = svgf.read(1)
			s.write('"')
			if fname != svg_icons[-1]:
				s.write(",")
			s.write("\n")
		s.write("};\n")
		# Write the SVG icon names as an array of C strings.
		s.write("\nstatic const char *editor_4d_icon_names[] = {\n")
		index = 0
		for f in svg_icons:
			fname = str(f)
			# Trim the `.svg` extension from the string.
			icon_name = os.path.basename(fname)[:-4]
			s.write('\t"{0}"'.format(icon_name))
			if fname != svg_icons[-1]:
				s.write(",")
			s.write("\n")
			index += 1
		s.write("};\n")
		# Write the number of icons.
		s.write("\nstatic const int editor_4d_icon_count = {0};\n".format(index))
		# Write the file to disk.
		with open(dst, "w", encoding="utf-8", newline="\n") as f:
			f.write(s.getvalue())
