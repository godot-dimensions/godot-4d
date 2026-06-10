# Contributing to Godot Dimensions

## C++ code style guide

For anything not listed here, follow Godot's C++ guidelines: https://contributing.godotengine.org/en/latest/engine/guidelines/cpp_usage_guidelines.html

### Include order

Always include things in this order:

1. Depending on the type of file:
   * For cpp files, the corresponding header file.
   * For header files that inherit another class in the module, the parent class header file.
2. Other headers in the Godot 4D module.
3. If neither 1 or 2 apply, `godot_4d_defines.h`.
4. Headers from Godot, guarded by `#if GDEXTENSION` and/or `#if GODOT_MODULE`.
5. External or standard library headers like `#include <algorithm>`.
6. Forward declarations go after all includes.

Critically, there must be an include path leading to `godot_4d_defines.h` right at the start of the file, so the first include MUST lead to `godot_4d_defines.h`, or be `godot_4d_defines.h` directly if no other in-module includes are needed. The file `godot_4d_defines.h` contains critical definitions needed across most or all of the Godot 4D module.

Separate the includes for each number with a blank line, except do not fight clang-format if it wants to remove them. If the list of includes gets quite long, group them by category and mark the groups with comments. See `register_types.cpp` for an example.
