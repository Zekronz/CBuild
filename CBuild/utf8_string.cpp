#include "pch.h"
#include "utf8_string.h"

namespace CBuild {

	UTF8_String::UTF8_String() {}

	UTF8_String::UTF8_String(const char* _value) {
		append(_value);
	}

	UTF8_String::UTF8_String(std::string _value) : UTF8_String(_value.c_str()) {}

	UTF8_String::UTF8_String(UTF8_String& _value) {
		char_list = _value.char_list;
	}

	bool UTF8_String::operator == (const UTF8_String& _value) {
		return (char_list == _value.char_list);
	}

	bool UTF8_String::operator == (const char* _value) {

		if (empty()) return (_value[0] == '\0');

		u64 i = 0;
		while (_value[i] != '\0') {

			if (i >= size()) return false;

			if ((u32)_value[i] != at(i)) return false;
			++i;

		}

		return (i == size());

	}

	bool UTF8_String::operator != (const char* _value) {
		return !((*this) == _value);
	}

	bool UTF8_String::operator != (const UTF8_String& _value) {
		return (char_list != _value.char_list);
	}

	void UTF8_String::operator += (const char* _value) {
		append(_value);
	}

	void UTF8_String::operator += (char _value) {
		char_list.push_back((u32)_value);
	}

	void UTF8_String::operator += (const std::string& _value) {
		append(_value.c_str());
	}

	void UTF8_String::operator += (const UTF8_String& _value) {

		char_list.reserve(char_list.size() + _value.char_list.size());

		for (const u32& val : _value.char_list) {
			char_list.push_back(val);
		}

	}

	u32& UTF8_String::operator [] (u64 _charPos) const {

		u32 val = at(_charPos);
		return val;

	}

	std::vector<u32>::const_iterator UTF8_String::begin() const {
		return char_list.begin();
	}

	std::vector<u32>::const_iterator UTF8_String::end() const {
		return char_list.end();
	}

	u64 UTF8_String::size() const {
		return char_list.size();
	}

	u64 UTF8_String::length() const {
		return size();
	}

	u32 UTF8_String::at(u64 _charPos) const {

		if (_charPos >= char_list.size()) {

			throw std::out_of_range("_char_pos out of range.");
			return 0;

		}

		return char_list[_charPos];

	}

	bool UTF8_String::empty() const {
		return char_list.empty();
	}

	void UTF8_String::reserve(u64 _size) {
		char_list.reserve(_size);
	}

	void UTF8_String::append(const char* _value) {

		u32 endian = 255;
		bool is_little_endian = ((endian & 0x000000FF)) == 255;

		u64 ind = 0;
		u8 val;

		while ((val = _value[ind]) != '\0') {

			if (val <= 127) {

				char_list.push_back((u32)val);
				++ind;

				continue;

			}

			u32 byte_count = 0;
			bool error = false;

			for (u8 j = 0; j < 7; ++j) {

				if (((val & (0b10000000 >> j)) >> (7 - j)) == 1) {

					if (j == 6) {

						error = true;
						break; //Last bit can't be 1.

					}

					++byte_count;
					continue;

				}

				break;

			}

			if (byte_count <= 1 || error) {

				++ind;
				continue;

			}

			if (byte_count <= 4) { //32 bits is the max we'll handle for now.

				for (u64 j = ind + 1; j < ind + (u64)byte_count; ++j) {
					if (_value[j] == '\0') return;
				}

				u32 combinedVal = 0;

				for (u32 j = 0; j < byte_count; ++j) {

					u8 c = (u8)_value[ind + (u64)j];
					u8 mask = (j == 0) ? 0xFF >> (byte_count + 1) : 0b00111111;

					if (is_little_endian) combinedVal |= ((u32)(c & mask) << (((byte_count - 1) - j) * 6));
					else combinedVal |= ((u32)(c & mask) << (j * 6));

				}

				char_list.push_back(combinedVal);

			}

			ind += (u64)byte_count;

		}

	}

	std::vector<u32>::iterator UTF8_String::erase(std::vector<u32>::const_iterator _position) {
		return char_list.erase(_position);
	}

	std::vector<u32>::iterator UTF8_String::erase(u64 _position, u64 _length) {

		if (_position >= char_list.size()) {

			throw std::out_of_range("_position out of range.");
			return char_list.end();

		}

		if (_position + _length > char_list.size()) {
			_length -= (_position + _length) - char_list.size();
		}

		return char_list.erase(char_list.begin() + _position, char_list.begin() + _position + _length - 1);

	}

	void UTF8_String::trim() {
		
		while (length() > 0 && std::isspace(char_list[0])) erase(0, 1);
		while (length() > 0 && std::isspace(char_list[length() - 1])) erase(length() - 1, 1);

	}

}