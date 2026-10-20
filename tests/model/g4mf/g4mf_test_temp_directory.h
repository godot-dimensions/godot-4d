#pragma once

#include "core/io/dir_access.h"
#include "core/os/os.h"
#include "core/os/time.h"

// Keep G4MF fixtures together and remove only the unique directory owned by this scope.
class G4MFTestTempDirectory {
	String _path;
	Error _error = OK;

public:
	explicit G4MFTestTempDirectory(const String &p_prefix) {
		if (!p_prefix.is_valid_filename()) {
			_error = ERR_FILE_BAD_PATH;
			return;
		}
		const String base_dir = OS::get_singleton()->get_temp_path().path_join("godot/4d/g4mf");
		_error = DirAccess::make_dir_recursive_absolute(base_dir);
		if (_error != OK) {
			return;
		}
		const String name = p_prefix + "-" + Time::get_singleton()->get_datetime_string_from_system().remove_chars("-T:") + itos(Time::get_singleton()->get_ticks_usec());
		uint64_t discriminator = 0;
		do {
			const String candidate = base_dir.path_join(name + "-" + itos(discriminator++));
			_error = DirAccess::make_dir_absolute(candidate);
			if (_error == OK) {
				_path = candidate;
			}
		} while (_error == ERR_ALREADY_EXISTS);
	}

	G4MFTestTempDirectory(const G4MFTestTempDirectory &) = delete;
	G4MFTestTempDirectory &operator=(const G4MFTestTempDirectory &) = delete;

	~G4MFTestTempDirectory() {
		if (_path.is_empty()) {
			return;
		}
		{
			const Ref<DirAccess> directory = DirAccess::open(_path);
			if (directory.is_null() || directory->erase_contents_recursive() != OK) {
				return;
			}
		}
		DirAccess::remove_absolute(_path);
	}

	Error get_error() const { return _error; }
	String get_path() const { return _path; }
};
