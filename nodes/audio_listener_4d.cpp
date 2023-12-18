#include "audio_listener_4d.h"

#include "scene/3d/audio_listener_3d.h"

void AudioListener4D::set_current(const bool p_current) {
	if (p_current) {
		_real_listener->make_current();
	} else {
		_real_listener->clear_current();
	}
}

bool AudioListener4D::is_current() const {
	return _real_listener->is_current();
}

void AudioListener4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_current", "current"), &AudioListener4D::set_current);
	ClassDB::bind_method(D_METHOD("is_current"), &AudioListener4D::is_current);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "current"), "set_current", "is_current");
}

AudioListener4D::AudioListener4D() {
	_real_listener = memnew(AudioListener3D);
	add_child(_real_listener);
	_real_listener->make_current();
}
