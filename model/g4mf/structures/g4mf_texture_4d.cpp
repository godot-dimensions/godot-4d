#include "g4mf_texture_4d.h"

Ref<G4MFTexture4D> G4MFTexture4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFTexture4D> texture;
	texture.instantiate();
	texture->read_item_entries_from_dictionary(p_dict);
	return texture;
}

Dictionary G4MFTexture4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	return dict;
}

void G4MFTexture4D::_bind_methods() {
	ClassDB::bind_static_method("G4MFTexture4D", D_METHOD("from_dictionary", "dict"), &G4MFTexture4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFTexture4D::to_dictionary);
}
