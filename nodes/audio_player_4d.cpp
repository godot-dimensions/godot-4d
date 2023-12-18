#include "audio_player_4d.h"

#include "audio_listener_4d.h"
#include "scene/3d/audio_listener_3d.h"
#include "scene/main/viewport.h"

void AudioPlayer4D::_notification(const int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			AudioServer::get_singleton()->add_listener_changed_callback(_listener_changed_cb, this);
			if (_autoplay && !Engine::get_singleton()->is_editor_hint()) {
				play();
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			stop();
			AudioServer::get_singleton()->remove_listener_changed_callback(_listener_changed_cb, this);
		} break;
		case NOTIFICATION_PAUSED: {
			if (!can_process()) {
				// Node can't process so we start fading out to silence.
				set_stream_paused(true);
			}
		} break;
		case NOTIFICATION_UNPAUSED: {
			set_stream_paused(false);
		} break;
		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			// Update anything related to position first, if possible of course.
			//if (_setplay.get() > 0 || (_active.is_set() && _last_mix_count != AudioServer::get_singleton()->get_mix_count())) {
			_update_panning();
			//}

			if (_setplay.get() >= 0 && _stream.is_valid()) {
				_active.set();
				Ref<AudioStreamPlayback> new_playback = _stream->instantiate_playback();
				ERR_FAIL_COND_MSG(new_playback.is_null(), "Failed to instantiate playback.");
				AudioServer::get_singleton()->start_playback_stream(new_playback, _get_actual_bus(), _volume_vector, _setplay.get(), _pitch_scale);
				_stream_playbacks.push_back(new_playback);
				_setplay.set(-1);
			}

			if (!_stream_playbacks.is_empty() && _active.is_set()) {
				// Stop playing if no longer active.
				Vector<Ref<AudioStreamPlayback>> playbacks_to_remove;
				for (Ref<AudioStreamPlayback> &playback : _stream_playbacks) {
					if (playback.is_valid() && !AudioServer::get_singleton()->is_playback_active(playback) && !AudioServer::get_singleton()->is_playback_paused(playback)) {
						playbacks_to_remove.push_back(playback);
					}
				}
				// Now go through and remove playbacks that have finished. Removing elements from a Vector in a range based for is asking for trouble.
				for (Ref<AudioStreamPlayback> &playback : playbacks_to_remove) {
					_stream_playbacks.erase(playback);
				}
				if (!playbacks_to_remove.is_empty() && _stream_playbacks.is_empty()) {
					// This node is no longer actively playing audio.
					_active.clear();
					set_physics_process_internal(false);
				}
				if (!playbacks_to_remove.is_empty()) {
					emit_signal(SNAME("finished"));
				}
			}

			while (_stream_playbacks.size() > _max_polyphony) {
				AudioServer::get_singleton()->stop_playback_stream(_stream_playbacks[0]);
				_stream_playbacks.remove_at(0);
			}
		} break;
	}
}

StringName AudioPlayer4D::_get_actual_bus() const {
	return _audio_bus;
}

const Node4D *_get_audio_listener_4d(const Viewport *p_viewport) {
	const AudioListener3D *audio_listener_3d = p_viewport->get_audio_listener_3d();
	if (audio_listener_3d) {
		return Object::cast_to<const Node4D>(audio_listener_3d->get_parent());
	}
	return nullptr;
}

//float AudioStreamPlayer3D::_get_attenuation_db(float p_distance) const {
//	float att = 0;
//	switch (_attenuation_model) {
//		case ATTENUATION_INVERSE_CUBIC_DISTANCE: {
//			real_t d = (p_distance / unit_size);
//			d *= d * d;
//			att = Math::linear_to_db(1.0 / (d + CMP_EPSILON));
//		} break;
//		case ATTENUATION_INVERSE_SQUARE_DISTANCE: {
//			float d = (p_distance / unit_size);
//			d *= d;
//			att = Math::linear_to_db(1.0 / (d + CMP_EPSILON));
//		} break;
//		case ATTENUATION_INVERSE_DISTANCE: {
//			att = Math::linear_to_db(1.0 / ((p_distance / unit_size) + CMP_EPSILON));
//		} break;
//		case ATTENUATION_DISABLED:
//			break;
//	}
//
//	att += volume_db;
//	if (att > max_db) {
//		att = max_db;
//	}
//
//	return att;
//}

void AudioPlayer4D::_update_panning() {
	if (!_active.is_set() || _stream.is_null()) {
		return;
	}

	Viewport *viewport = get_viewport();

	if (viewport->is_audio_listener_3d()) {
		const Node4D *audio_listener_4d = _get_audio_listener_4d(viewport);
		if (audio_listener_4d) {
			_update_volume_frame(audio_listener_4d);
		}
	}

//	if (viewport->is_audio_listener_2d()) {
//		real_t balance_distance = _balance_distance;
//		if (balance_distance < 0) {
//			balance_distance *= -viewport->get_visible_rect().size.x;
//		}
//		if (balance_distance < CMP_EPSILON) {
//			balance_distance = CMP_EPSILON;
//		}
//
//		real_t audio_position = get_global_position();
//		bool is_vertical = false;
//
//		const Node4D *listener_node_4d = _get_audio_listener_4d(viewport);
//		if (listener_node_1d) {
//			audio_position -= listener_node_1d->get_global_position();
//			is_vertical = _is_audio_listener_vertical(listener_node_1d);
//		}
//
//		real_t audio_distance = Math::abs(audio_position);
//		if (audio_distance < _max_distance) {
//			real_t pan = 0.5;
//			if (!is_vertical) {
//				real_t pseudo_sigmoid = audio_position / (balance_distance + audio_distance);
//				pan = pseudo_sigmoid * 0.5f + 0.5f;
//			}
//			real_t multiplier = Math::pow(1.0f - audio_distance / _max_distance, _attenuation);
//			multiplier *= Math::db_to_linear(_volume_db);
//			_volume_vector.write[0] = AudioFrame(1.0f - pan, pan) * multiplier;
//		}
//	}

	for (const Ref<AudioStreamPlayback> &playback : _stream_playbacks) {
		AudioServer::get_singleton()->set_playback_bus_exclusive(playback, _get_actual_bus(), _volume_vector);
	}

	for (Ref<AudioStreamPlayback> &playback : _stream_playbacks) {
		AudioServer::get_singleton()->set_playback_pitch_scale(playback, _pitch_scale);
	}

	_last_mix_count = AudioServer::get_singleton()->get_mix_count();
}

void AudioPlayer4D::_update_volume_frame(const Node4D *p_listener) {
	const Transform4D listener_ortho = p_listener->get_global_transform().orthonormalized();
	const Transform4D rel_to_listener = listener_ortho.inverse_transposed() * get_global_transform();

	const Vector4 rel_pos = rel_to_listener.get_origin();
	const real_t pan_angle = Vector4D::angle_to(rel_pos, Vector4(1, 0, 0, 0));
}

void AudioPlayer4D::set_stream(Ref<AudioStream> p_stream) {
	stop();
	_stream = p_stream;
}

Ref<AudioStream> AudioPlayer4D::get_stream() const {
	return _stream;
}

void AudioPlayer4D::set_volume_db(const real_t p_volume) {
	_volume_db = p_volume;
}

real_t AudioPlayer4D::get_volume_db() const {
	return _volume_db;
}

void AudioPlayer4D::set_pitch_scale(const real_t p_pitch_scale) {
	ERR_FAIL_COND(p_pitch_scale <= 0.0);
	_pitch_scale = p_pitch_scale;
	for (Ref<AudioStreamPlayback> &playback : _stream_playbacks) {
		AudioServer::get_singleton()->set_playback_pitch_scale(playback, p_pitch_scale);
	}
}

real_t AudioPlayer4D::get_pitch_scale() const {
	return _pitch_scale;
}

void AudioPlayer4D::play(const float p_from_pos) {
	if (_stream.is_null()) {
		return;
	}
	ERR_FAIL_COND_MSG(!is_inside_tree(), "Playback can only happen when a node is inside the scene tree");
	if (_stream->is_monophonic() && is_playing()) {
		stop();
	}

	_setplay.set(p_from_pos);
	_active.set();
	set_physics_process_internal(true);
}

void AudioPlayer4D::seek(const float p_seconds) {
	if (is_playing()) {
		stop();
		play(p_seconds);
	}
}

void AudioPlayer4D::stop() {
	_setplay.set(-1);
	for (Ref<AudioStreamPlayback> &playback : _stream_playbacks) {
		AudioServer::get_singleton()->stop_playback_stream(playback);
	}
	_stream_playbacks.clear();
	_active.clear();
	set_physics_process_internal(false);
}

bool AudioPlayer4D::is_playing() const {
	for (const Ref<AudioStreamPlayback> &playback : _stream_playbacks) {
		if (AudioServer::get_singleton()->is_playback_active(playback)) {
			return true;
		}
	}
	return false;
}

float AudioPlayer4D::get_playback_position() const {
	// Return the playback position of the most recently started playback stream.
	if (!_stream_playbacks.is_empty()) {
		return AudioServer::get_singleton()->get_playback_position(_stream_playbacks[_stream_playbacks.size() - 1]);
	}
	return 0;
}

void AudioPlayer4D::set_bus(const StringName &p_bus) {
	_audio_bus = p_bus; // This will be pushed to the audio server during the next physics timestep, which is fast enough.
}

StringName AudioPlayer4D::get_bus() const {
	for (int i = 0; i < AudioServer::get_singleton()->get_bus_count(); i++) {
		if (AudioServer::get_singleton()->get_bus_name(i) == _audio_bus) {
			return _audio_bus;
		}
	}
	return SNAME("Master");
}

void AudioPlayer4D::set_autoplay(const bool p_enable) {
	_autoplay = p_enable;
}

bool AudioPlayer4D::is_autoplay_enabled() const {
	return _autoplay;
}

void AudioPlayer4D::_set_playing(const bool p_enable) {
	if (p_enable) {
		play();
	} else {
		stop();
	}
}

bool AudioPlayer4D::_is_active() const {
	return _active.is_set();
}

void AudioPlayer4D::_validate_property(PropertyInfo &property) const {
	if (property.name == "bus") {
		String options;
		for (int i = 0; i < AudioServer::get_singleton()->get_bus_count(); i++) {
			if (i > 0) {
				options += ",";
			}
			String name = AudioServer::get_singleton()->get_bus_name(i);
			options += name;
		}

		property.hint_string = options;
	}
}

void AudioPlayer4D::_bus_layout_changed() {
	notify_property_list_changed();
}

void AudioPlayer4D::set_balance_distance(const real_t p_pixels) {
	_balance_distance = p_pixels;
}

real_t AudioPlayer4D::get_balance_distance() const {
	return _balance_distance;
}

void AudioPlayer4D::set_max_distance(const real_t p_pixels) {
	ERR_FAIL_COND(p_pixels <= 0.0);
	_max_distance = p_pixels;
}

real_t AudioPlayer4D::get_max_distance() const {
	return _max_distance;
}

void AudioPlayer4D::set_attenuation(const real_t p_curve) {
	_attenuation = p_curve;
}

real_t AudioPlayer4D::get_attenuation() const {
	return _attenuation;
}

void AudioPlayer4D::set_stream_paused(const bool p_pause) {
	// TODO this does not have perfect recall, fix that maybe? If there are zero playbacks registered with the AudioServer, this bool isn't persisted.
	for (Ref<AudioStreamPlayback> &playback : _stream_playbacks) {
		AudioServer::get_singleton()->set_playback_paused(playback, p_pause);
	}
}

bool AudioPlayer4D::get_stream_paused() const {
	// There's currently no way to pause some playback streams but not others. Check the first and don't bother looking at the rest.
	if (!_stream_playbacks.is_empty()) {
		return AudioServer::get_singleton()->is_playback_paused(_stream_playbacks[0]);
	}
	return false;
}

Ref<AudioStreamPlayback> AudioPlayer4D::get_stream_playback() const {
	if (!_stream_playbacks.is_empty()) {
		return _stream_playbacks[_stream_playbacks.size() - 1];
	}
	return nullptr;
}

void AudioPlayer4D::set_max_polyphony(const int p_max_polyphony) {
	if (p_max_polyphony > 0) {
		_max_polyphony = p_max_polyphony;
	}
}

int AudioPlayer4D::get_max_polyphony() const {
	return _max_polyphony;
}

void AudioPlayer4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_stream", "stream"), &AudioPlayer4D::set_stream);
	ClassDB::bind_method(D_METHOD("get_stream"), &AudioPlayer4D::get_stream);

	ClassDB::bind_method(D_METHOD("set_volume_db", "volume_db"), &AudioPlayer4D::set_volume_db);
	ClassDB::bind_method(D_METHOD("get_volume_db"), &AudioPlayer4D::get_volume_db);

	ClassDB::bind_method(D_METHOD("set_pitch_scale", "pitch_scale"), &AudioPlayer4D::set_pitch_scale);
	ClassDB::bind_method(D_METHOD("get_pitch_scale"), &AudioPlayer4D::get_pitch_scale);

	ClassDB::bind_method(D_METHOD("play", "from_position"), &AudioPlayer4D::play, DEFVAL(0.0));
	ClassDB::bind_method(D_METHOD("seek", "to_position"), &AudioPlayer4D::seek);
	ClassDB::bind_method(D_METHOD("stop"), &AudioPlayer4D::stop);

	ClassDB::bind_method(D_METHOD("is_playing"), &AudioPlayer4D::is_playing);
	ClassDB::bind_method(D_METHOD("get_playback_position"), &AudioPlayer4D::get_playback_position);

	ClassDB::bind_method(D_METHOD("set_bus", "bus"), &AudioPlayer4D::set_bus);
	ClassDB::bind_method(D_METHOD("get_bus"), &AudioPlayer4D::get_bus);

	ClassDB::bind_method(D_METHOD("set_autoplay", "enable"), &AudioPlayer4D::set_autoplay);
	ClassDB::bind_method(D_METHOD("is_autoplay_enabled"), &AudioPlayer4D::is_autoplay_enabled);

	ClassDB::bind_method(D_METHOD("_set_playing", "enable"), &AudioPlayer4D::_set_playing);
	ClassDB::bind_method(D_METHOD("_is_active"), &AudioPlayer4D::_is_active);

	ClassDB::bind_method(D_METHOD("set_balance_distance", "pixels"), &AudioPlayer4D::set_balance_distance);
	ClassDB::bind_method(D_METHOD("get_balance_distance"), &AudioPlayer4D::get_balance_distance);

	ClassDB::bind_method(D_METHOD("set_max_distance", "pixels"), &AudioPlayer4D::set_max_distance);
	ClassDB::bind_method(D_METHOD("get_max_distance"), &AudioPlayer4D::get_max_distance);

	ClassDB::bind_method(D_METHOD("set_attenuation", "curve"), &AudioPlayer4D::set_attenuation);
	ClassDB::bind_method(D_METHOD("get_attenuation"), &AudioPlayer4D::get_attenuation);

	ClassDB::bind_method(D_METHOD("set_stream_paused", "pause"), &AudioPlayer4D::set_stream_paused);
	ClassDB::bind_method(D_METHOD("get_stream_paused"), &AudioPlayer4D::get_stream_paused);

	ClassDB::bind_method(D_METHOD("set_max_polyphony", "max_polyphony"), &AudioPlayer4D::set_max_polyphony);
	ClassDB::bind_method(D_METHOD("get_max_polyphony"), &AudioPlayer4D::get_max_polyphony);

	ClassDB::bind_method(D_METHOD("get_stream_playback"), &AudioPlayer4D::get_stream_playback);

	// Cone
	ClassDB::bind_method(D_METHOD("set_cone_inner_angle", "radians"), &AudioPlayer4D::set_cone_inner_angle);
	ClassDB::bind_method(D_METHOD("get_cone_inner_angle"), &AudioPlayer4D::get_cone_inner_angle);
	ClassDB::bind_method(D_METHOD("set_cone_outer_angle", "radians"), &AudioPlayer4D::set_cone_outer_angle);
	ClassDB::bind_method(D_METHOD("get_cone_outer_angle"), &AudioPlayer4D::get_cone_outer_angle);
	ClassDB::bind_method(D_METHOD("set_cone_enabled", "enable"), &AudioPlayer4D::set_cone_enabled);
	ClassDB::bind_method(D_METHOD("is_cone_enabled"), &AudioPlayer4D::is_cone_enabled);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "stream", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream"), "set_stream", "get_stream");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "volume_db", PROPERTY_HINT_RANGE, "-80,24"), "set_volume_db", "get_volume_db");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "pitch_scale", PROPERTY_HINT_RANGE, "0.01,4,0.01,or_greater"), "set_pitch_scale", "get_pitch_scale");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "playing", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "_set_playing", "is_playing");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autoplay"), "set_autoplay", "is_autoplay_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "stream_paused", PROPERTY_HINT_NONE, ""), "set_stream_paused", "get_stream_paused");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "balance_distance", PROPERTY_HINT_RANGE, "-8,4096,0.001,or_greater,exp"), "set_balance_distance", "get_balance_distance");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_distance", PROPERTY_HINT_RANGE, "1,4096,1,or_greater,exp"), "set_max_distance", "get_max_distance");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "attenuation", PROPERTY_HINT_EXP_EASING, "attenuation"), "set_attenuation", "get_attenuation");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_polyphony", PROPERTY_HINT_NONE, ""), "set_max_polyphony", "get_max_polyphony");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "bus", PROPERTY_HINT_ENUM, ""), "set_bus", "get_bus");

	ADD_GROUP("Cone", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cone_inner_angle", PROPERTY_HINT_RANGE, "0,360,0.1,radians_as_degrees"), "set_cone_inner_angle", "get_cone_inner_angle");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cone_outer_angle", PROPERTY_HINT_RANGE, "0,360,0.1,radians_as_degrees"), "set_cone_outer_angle", "get_cone_outer_angle");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cone_enabled"), "set_cone_enabled", "is_cone_enabled");

	ADD_SIGNAL(MethodInfo("finished"));
}

AudioPlayer4D::AudioPlayer4D() {
	AudioServer::get_singleton()->connect("bus_layout_changed", callable_mp(this, &AudioPlayer4D::_bus_layout_changed));
//	_volume_vector.resize(4);
//	_volume_vector.write[0] = AudioFrame(0, 0);
//	_volume_vector.write[1] = AudioFrame(0, 0);
//	_volume_vector.write[2] = AudioFrame(0, 0);
//	_volume_vector.write[3] = AudioFrame(0, 0);
}

AudioPlayer4D::~AudioPlayer4D() {
}
