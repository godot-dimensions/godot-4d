#pragma once

#include "node_4d.h"
#include "servers/audio/audio_stream.h"
#include "servers/audio_server.h"

class AudioPlayer4D : public Node4D {
	GDCLASS(AudioPlayer4D, Node4D);

private:
	Vector<Ref<AudioStreamPlayback>> _stream_playbacks;
	Ref<AudioStream> _stream;

//	SafeNumeric<float> _setplay{ -1.0 };

//	Vector<AudioFrame> _volume_vector;
	StringName _audio_bus = SNAME("Master");
	AudioFrame _volume_frame = AudioFrame(0.0, 0.0);

//	uint64_t _last_mix_count = -1;

	real_t _volume_db = 0.0;
	real_t _pitch_scale = 1.0;
	real_t _unit_distance = 10.0;
	real_t _max_distance = 2000.0;
	real_t _attenuation = 3.0;

	real_t _cone_inner_angle = Math_TAU / 8.0;
	real_t _cone_outer_angle = Math_TAU / 4.0;
	bool _cone_enabled = false;

	bool _autoplay = false;
	SafeFlag _active{ false };

//	int _max_polyphony = 1;

	void _set_playing(const bool p_enable);
	bool _is_active() const;

	StringName _get_actual_bus() const;
	void _bus_layout_changed();
	void _update_panning();
	void _update_volume_frame(const Node4D *p_listener);

	static void _listener_changed_cb(void *self) { reinterpret_cast<AudioPlayer4D *>(self)->_update_panning(); }

protected:
	void _validate_property(PropertyInfo &property) const;
	void _notification(const int p_what);
	static void _bind_methods();

public:
	void set_stream(Ref<AudioStream> p_stream);
	Ref<AudioStream> get_stream() const;

	void set_volume_db(const real_t p_volume);
	real_t get_volume_db() const;

	void set_pitch_scale(const real_t p_pitch_scale);
	real_t get_pitch_scale() const;

	void play(const float p_from_pos = 0.0);
	void seek(const float p_seconds);
	void stop();
	bool is_playing() const;
	float get_playback_position() const;

	void set_bus(const StringName &p_bus);
	StringName get_bus() const;

	void set_autoplay(const bool p_enable);
	bool is_autoplay_enabled() const;

	void set_balance_distance(const real_t p_pixels);
	real_t get_balance_distance() const;

	void set_max_distance(const real_t p_pixels);
	real_t get_max_distance() const;

	void set_attenuation(const real_t p_curve);
	real_t get_attenuation() const;

	void set_stream_paused(const bool p_pause);
	bool get_stream_paused() const;

//	void set_max_polyphony(const int p_max_polyphony);
//	int get_max_polyphony() const;

	Ref<AudioStreamPlayback> get_stream_playback() const;

	AudioPlayer4D();
	~AudioPlayer4D();
};
