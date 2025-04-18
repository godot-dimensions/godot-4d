#pragma once

#include "g4mf_item_4d.h"

class G4MFState4D;

class G4MFTexture4D : public G4MFItem4D {
	GDCLASS(G4MFTexture4D, G4MFItem4D);

protected:
	static void _bind_methods();

public:
	static Ref<G4MFTexture4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
