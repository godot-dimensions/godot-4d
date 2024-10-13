#include "quad_split_container.h"

#if GDEXTENSION
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#endif

void QuadSplitContainer::GDEXTMOD_GUI_INPUT(const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND(p_event.is_null());
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->get_button_index() == MOUSE_BUTTON_LEFT) {
		if (mb->is_pressed()) {
			const Vector2 size = get_size();
			const int h_sep = get_theme_constant(StringName("separation"), StringName("HSplitContainer"));
			const int v_sep = get_theme_constant(StringName("separation"), StringName("VSplitContainer"));
			const int mid_w = size.width * _ratio_h;
			const int mid_h = size.height * _ratio_v;
			_drag_begin_pos = mb->get_position();
			_drag_begin_ratio.x = _ratio_h;
			_drag_begin_ratio.y = _ratio_v;
			if (_visible_child_count > 2 || _dominant_side == SIDE_TOP || _dominant_side == SIDE_BOTTOM) {
				_dragging_v = mb->get_position().y > (mid_h - v_sep / 2) && mb->get_position().y < (mid_h + v_sep / 2);
			} else {
				_dragging_v = false;
			}
			if (_visible_child_count > 2 || _dominant_side == SIDE_LEFT || _dominant_side == SIDE_RIGHT) {
				_dragging_h = mb->get_position().x > (mid_w - h_sep / 2) && mb->get_position().x < (mid_w + h_sep / 2);
			} else {
				_dragging_h = false;
			}
		} else {
			_dragging_h = false;
			_dragging_v = false;
		}
	}
	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		if (_visible_child_count > 2) {
			const Vector2 size = get_size();
			const int h_sep = get_theme_constant(StringName("separation"), StringName("HSplitContainer"));
			const int v_sep = get_theme_constant(StringName("separation"), StringName("VSplitContainer"));
			const int mid_w = size.width * _ratio_h;
			const int mid_h = size.height * _ratio_v;
			const bool was_hovering_h = _hovering_h;
			const bool was_hovering_v = _hovering_v;
			_hovering_h = mm->get_position().x > (mid_w - h_sep / 2) && mm->get_position().x < (mid_w + h_sep / 2);
			_hovering_v = mm->get_position().y > (mid_h - v_sep / 2) && mm->get_position().y < (mid_h + v_sep / 2);
			if (was_hovering_h != _hovering_h || was_hovering_v != _hovering_v) {
				queue_redraw();
			}
		}
		if (_dragging_h) {
			real_t new_ratio = _drag_begin_ratio.x + (mm->get_position().x - _drag_begin_pos.x) / get_size().width;
			new_ratio = CLAMP(new_ratio, 40 / get_size().width, (get_size().width - 40) / get_size().width);
			_ratio_h = new_ratio;
			queue_sort();
			queue_redraw();
		}
		if (_dragging_v) {
			real_t new_ratio = _drag_begin_ratio.y + (mm->get_position().y - _drag_begin_pos.y) / get_size().height;
			new_ratio = CLAMP(new_ratio, 40 / get_size().height, (get_size().height - 40) / get_size().height);
			_ratio_v = new_ratio;
			queue_sort();
			queue_redraw();
		}
	}
}

void QuadSplitContainer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_MOUSE_ENTER:
		case NOTIFICATION_MOUSE_EXIT: {
			_mouseover = (p_what == NOTIFICATION_MOUSE_ENTER);
			queue_redraw();
		} break;
		case NOTIFICATION_DRAW: {
			if (_mouseover) {
				const Ref<Texture2D> h_grabber = get_theme_icon(StringName("grabber"), StringName("HSplitContainer"));
				const Ref<Texture2D> v_grabber = get_theme_icon(StringName("grabber"), StringName("VSplitContainer"));
				const Vector2 size = get_size();
				const int h_sep = get_theme_constant(StringName("separation"), StringName("HSplitContainer"));
				const int v_sep = get_theme_constant(StringName("separation"), StringName("VSplitContainer"));
				const int mid_w = size.width * _ratio_h;
				const int mid_h = size.height * _ratio_v;
				switch (_visible_child_count) {
					// We do not show any grabbers for 1 child because it takes up the whole space. Also none for 0 children.
					case 2: {
						if (_dominant_side == SIDE_TOP || _dominant_side == SIDE_BOTTOM) {
							draw_texture(v_grabber, Vector2((size.width - v_grabber->get_width()) / 2, mid_h - v_grabber->get_height() / 2));
							set_default_cursor_shape(CURSOR_VSPLIT);
						} else {
							draw_texture(h_grabber, Vector2(mid_w - h_grabber->get_width() / 2, (size.height - h_grabber->get_height()) / 2));
							set_default_cursor_shape(CURSOR_HSPLIT);
						}
					} break;
					case 3: {
						// TODO: Get rid of the dependency on the editor-only icons.
						const Ref<Texture2D> hdiag_grabber = get_theme_icon(StringName("GuiViewportHdiagsplitter"), StringName("EditorIcons"));
						const Ref<Texture2D> vdiag_grabber = get_theme_icon(StringName("GuiViewportVdiagsplitter"), StringName("EditorIcons"));
						const int size_left = mid_w - h_sep / 2;
						const int size_bottom = size.height - mid_h - v_sep / 2;
						switch (_dominant_side) {
							case SIDE_LEFT: {
								if ((_hovering_v && _hovering_h && !_dragging_v && !_dragging_h) || (_dragging_v && _dragging_h)) {
									// TODO: Texture for left-dominant 3-split grabber.
									set_default_cursor_shape(CURSOR_DRAG);
								} else if ((_hovering_v && !_dragging_h) || _dragging_v) {
									draw_texture(v_grabber, Vector2((size_left - v_grabber->get_width()) / 2, mid_h - v_grabber->get_height() / 2));
									set_default_cursor_shape(CURSOR_VSPLIT);
								} else if (_hovering_h || _dragging_h) {
									draw_texture(h_grabber, Vector2(mid_w - h_grabber->get_width() / 2, (size.height - h_grabber->get_height()) / 2));
									set_default_cursor_shape(CURSOR_HSPLIT);
								}
							} break;
							case SIDE_TOP: {
								if ((_hovering_v && _hovering_h && !_dragging_v && !_dragging_h) || (_dragging_v && _dragging_h)) {
									draw_texture(hdiag_grabber, Vector2(mid_w - hdiag_grabber->get_width() / 2, mid_h - v_grabber->get_height() / 4));
									set_default_cursor_shape(CURSOR_DRAG);
								} else if ((_hovering_v && !_dragging_h) || _dragging_v) {
									draw_texture(v_grabber, Vector2((size.width - v_grabber->get_width()) / 2, mid_h - v_grabber->get_height() / 2));
									set_default_cursor_shape(CURSOR_VSPLIT);
								} else if (_hovering_h || _dragging_h) {
									draw_texture(h_grabber, Vector2(mid_w - h_grabber->get_width() / 2, mid_h + v_grabber->get_height() / 2 + (size_bottom - h_grabber->get_height()) / 2));
									set_default_cursor_shape(CURSOR_HSPLIT);
								}
							} break;
							case SIDE_RIGHT: {
								if ((_hovering_v && _hovering_h && !_dragging_v && !_dragging_h) || (_dragging_v && _dragging_h)) {
									draw_texture(vdiag_grabber, Vector2(mid_w - vdiag_grabber->get_width() + v_grabber->get_height() / 4, mid_h - vdiag_grabber->get_height() / 2));
									set_default_cursor_shape(CURSOR_DRAG);
								} else if ((_hovering_v && !_dragging_h) || _dragging_v) {
									draw_texture(v_grabber, Vector2((size_left - v_grabber->get_width()) / 2, mid_h - v_grabber->get_height() / 2));
									set_default_cursor_shape(CURSOR_VSPLIT);
								} else if (_hovering_h || _dragging_h) {
									draw_texture(h_grabber, Vector2(mid_w - h_grabber->get_width() / 2, (size.height - h_grabber->get_height()) / 2));
									set_default_cursor_shape(CURSOR_HSPLIT);
								}
							}
							case SIDE_BOTTOM: {
								if ((_hovering_v && _hovering_h && !_dragging_v && !_dragging_h) || (_dragging_v && _dragging_h)) {
									// TODO: Texture for bottom-dominant 3-split grabber.
									set_default_cursor_shape(CURSOR_DRAG);
								} else if ((_hovering_v && !_dragging_h) || _dragging_v) {
									draw_texture(v_grabber, Vector2((size.width - v_grabber->get_width()) / 2, mid_h - v_grabber->get_height() / 2));
									set_default_cursor_shape(CURSOR_VSPLIT);
								} else if (_hovering_h || _dragging_h) {
									draw_texture(h_grabber, Vector2(mid_w - h_grabber->get_width() / 2, mid_h + v_grabber->get_height() / 2 + (size_bottom - h_grabber->get_height()) / 2));
									set_default_cursor_shape(CURSOR_HSPLIT);
								}
							} break;
						}
					} break;
					case 4: {
						// TODO: Get rid of the dependency on the editor-only icons.
						const Ref<Texture2D> vh_grabber = get_theme_icon(StringName("GuiViewportVhsplitter"), StringName("EditorIcons"));
						Vector2 half(mid_w, mid_h);
						if ((_hovering_v && _hovering_h && !_dragging_v && !_dragging_h) || (_dragging_v && _dragging_h)) {
							draw_texture(vh_grabber, half - vh_grabber->get_size() / 2.0f);
							set_default_cursor_shape(CURSOR_DRAG);
						} else if ((_hovering_v && !_dragging_h) || _dragging_v) {
							draw_texture(v_grabber, half - v_grabber->get_size() / 2.0f);
							set_default_cursor_shape(CURSOR_VSPLIT);
						} else if (_hovering_h || _dragging_h) {
							draw_texture(h_grabber, half - h_grabber->get_size() / 2.0f);
							set_default_cursor_shape(CURSOR_HSPLIT);
						}
					} break;
				}
			}
		} break;
		case NOTIFICATION_SORT_CHILDREN: {
			Control *control_children[4];
			int control_child_count = 0;
			for (int i = 0; i < get_child_count(); i++) {
				control_children[control_child_count] = Object::cast_to<Control>(get_child(i));
				if (control_children[control_child_count]) {
					control_child_count++;
					if (unlikely(control_child_count >= 4)) {
						break;
					}
				}
			}
			const Size2 size = get_size();
			if (size.x < 10 || size.y < 10 || _visible_child_count < 1) {
				for (int i = 0; i < control_child_count; i++) {
					control_children[i]->hide();
				}
				return;
			}
			const int h_sep = get_theme_constant(StringName("separation"), StringName("HSplitContainer"));
			const int v_sep = get_theme_constant(StringName("separation"), StringName("VSplitContainer"));
			const int mid_w = size.width * _ratio_h;
			const int mid_h = size.height * _ratio_v;
			const int size_left = mid_w - h_sep / 2;
			const int size_right = size.width - mid_w - h_sep / 2;
			const int size_top = mid_h - v_sep / 2;
			const int size_bottom = size.height - mid_h - v_sep / 2;
			switch (MIN(control_child_count, _visible_child_count)) {
				case 1: {
					control_children[0]->show();
					for (int i = 1; i < control_child_count; i++) {
						control_children[i]->hide();
					}
					fit_child_in_rect(control_children[0], Rect2(Vector2(), size));
				} break;
				case 2: {
					for (int i = 0; i < control_child_count; i++) {
						if (i > 1) {
							control_children[i]->hide();
						} else {
							control_children[i]->show();
						}
					}
					if (_dominant_side == SIDE_TOP || _dominant_side == SIDE_BOTTOM) {
						fit_child_in_rect(control_children[0], Rect2(Vector2(), Vector2(size.width, size_top)));
						fit_child_in_rect(control_children[1], Rect2(Vector2(0, mid_h + v_sep / 2), Vector2(size.width, size_bottom)));
					} else {
						fit_child_in_rect(control_children[0], Rect2(Vector2(), Vector2(size_left, size.height)));
						fit_child_in_rect(control_children[1], Rect2(Vector2(mid_w + h_sep / 2, 0), Vector2(size_right, size.height)));
					}
				} break;
				case 3: {
					for (int i = 0; i < control_child_count; i++) {
						if (i == 3) {
							control_children[i]->hide();
						} else {
							control_children[i]->show();
						}
					}
					switch (_dominant_side) {
						case SIDE_LEFT: {
							fit_child_in_rect(control_children[0], Rect2(Vector2(), Vector2(size_left, size.height)));
							fit_child_in_rect(control_children[1], Rect2(Vector2(mid_w + h_sep / 2, 0), Vector2(size_right, size_top)));
							fit_child_in_rect(control_children[2], Rect2(Vector2(mid_w + h_sep / 2, mid_h + v_sep / 2), Vector2(size_right, size_bottom)));
						} break;
						case SIDE_TOP: {
							fit_child_in_rect(control_children[0], Rect2(Vector2(), Vector2(size.width, size_top)));
							fit_child_in_rect(control_children[1], Rect2(Vector2(0, mid_h + v_sep / 2), Vector2(size_left, size_bottom)));
							fit_child_in_rect(control_children[2], Rect2(Vector2(mid_w + h_sep / 2, mid_h + v_sep / 2), Vector2(size_right, size_bottom)));
						} break;
						case SIDE_RIGHT: {
							fit_child_in_rect(control_children[0], Rect2(Vector2(mid_w + h_sep / 2, 0), Vector2(size_right, size.height)));
							fit_child_in_rect(control_children[1], Rect2(Vector2(), Vector2(size_left, size_top)));
							fit_child_in_rect(control_children[2], Rect2(Vector2(0, mid_h + v_sep / 2), Vector2(size_left, size_bottom)));
						} break;
						case SIDE_BOTTOM: {
							fit_child_in_rect(control_children[0], Rect2(Vector2(0, mid_h + v_sep / 2), Vector2(size.width, size_bottom)));
							fit_child_in_rect(control_children[1], Rect2(Vector2(), Vector2(size_left, size_top)));
							fit_child_in_rect(control_children[2], Rect2(Vector2(mid_w + h_sep / 2, 0), Vector2(size_right, size_top)));
						} break;
					}
				} break;
				case 4: {
					for (int i = 0; i < control_child_count; i++) {
						control_children[i]->show();
					}
					fit_child_in_rect(control_children[0], Rect2(Vector2(), Vector2(size_left, size_top)));
					fit_child_in_rect(control_children[1], Rect2(Vector2(mid_w + h_sep / 2, 0), Vector2(size_right, size_top)));
					fit_child_in_rect(control_children[2], Rect2(Vector2(0, mid_h + v_sep / 2), Vector2(size_left, size_bottom)));
					fit_child_in_rect(control_children[3], Rect2(Vector2(mid_w + h_sep / 2, mid_h + v_sep / 2), Vector2(size_right, size_bottom)));
				} break;
			}
		} break;
	}
}

void QuadSplitContainer::set_layout(const int8_t p_visible_child_count, const Side p_dominant_side) {
	ERR_FAIL_COND_MSG(p_visible_child_count < 0 || p_visible_child_count > 4, "Visible child count must be between 1 and 4.");
	_visible_child_count = p_visible_child_count;
	_dominant_side = p_dominant_side;
	queue_sort();
}

void QuadSplitContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_layout", "visible_child_count", "dominant_side"), &QuadSplitContainer::set_layout, DEFVAL(SIDE_TOP));
}

QuadSplitContainer::QuadSplitContainer() {
	set_clip_contents(true);
}
