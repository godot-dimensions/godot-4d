
#define APPEND_COMPONENT_TO_STRING(m_string, m_component)                                \
	if (!Math::is_zero_approx(m_component)) {                                            \
		if (m_string.length() > 0) {                                                     \
			if (m_component > 0) {                                                       \
				m_string += " + ";                                                       \
			} else {                                                                     \
				m_string += " - ";                                                       \
			}                                                                            \
			real_t abs_component = Math::abs(m_component);                               \
			String number_string = abs_component == 1 ? "" : String::num(abs_component); \
			m_string += number_string + #m_component;                                    \
		} else {                                                                         \
			String number_string = m_component == 1 ? "" : String::num(m_component);     \
			m_string += number_string + #m_component;                                    \
		}                                                                                \
	}
