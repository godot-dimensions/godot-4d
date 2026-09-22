#include "editor_import_plugin_base_4d.h"

const Transform4D EditorImportPluginBase4D::CONVERT_Z_UP_TO_Y_UP_TRANSFORM = Transform4D(Basis4D(Vector4(1, 0, 0, 0), Vector4(0, 0, -1, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 0, 1)), Vector4(0, 0, 0, 0));
