#pragma once

#include "../../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/resource.hpp>
#elif GODOT_MODULE
#include "core/io/resource.h"
#endif

struct GeneralShapeCurveTaperPoint4D {
	Vector4 position = Vector4();
	Vector4 radii = Vector4();
	double exponent = 2.0;

	bool is_equal_exact(const GeneralShapeCurveTaperPoint4D &p_other) const;
};

class GeneralShapeCurve4D : public Resource {
	GDCLASS(GeneralShapeCurve4D, Resource);

	Vector4 _radii;
	Vector<GeneralShapeCurveTaperPoint4D> _taper;
	double _exponent = 2.0;

protected:
	static void _bind_methods();

public:
	Vector4 get_radii() const { return _radii; }
	void set_radii(const Vector4 &p_radii);

	double get_exponent() const { return _exponent; }
	void set_exponent(double p_exponent);

	Vector<GeneralShapeCurveTaperPoint4D> get_taper() const { return _taper; }
	void set_taper(const Vector<GeneralShapeCurveTaperPoint4D> &p_taper) { _taper = p_taper; }

	int get_radii_dimension() const;
	real_t get_radii_sum() const;
	int get_taper_count() const { return _taper.size(); }
	PackedInt32Array get_used_axes() const;
	PackedInt32Array get_zero_axes() const;
	bool is_equal_exact(const Ref<GeneralShapeCurve4D> &p_other) const;
};
