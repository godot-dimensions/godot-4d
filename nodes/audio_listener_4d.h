#pragma once

#include "node_4d.h"
class AudioListener3D;

class AudioListener4D : public Node4D {
	GDCLASS(AudioListener4D, Node4D);

	// Viewport is hard-coded to work with AudioListener3D (and
	// AudioListener2D). We'll just use AudioListener3D as a
	// proxy so that we don't have to modify Viewport, but our
	// AudioPlayer4D will still use the 4D listener.
	AudioListener3D *_real_listener;

protected:
	static void _bind_methods();

public:
	void set_current(const bool p_current);
	bool is_current() const;

	AudioListener4D();
};
