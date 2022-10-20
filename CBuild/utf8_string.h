#pragma once

#include "types.h"

#include <string>
#include <vector>

namespace CBuild {

	class UTF8_String {
	private:
		std::vector<u32> char_list;

	public:
		UTF8_String();
		UTF8_String(const char* _value);
		UTF8_String(std::string _value);
		UTF8_String(UTF8_String& _value);

		bool operator == (const UTF8_String& _value);
		bool operator == (const char* _value);
		bool operator != (const UTF8_String& _value);
		bool operator != (const char* _value);
		void operator += (const char* _value);
		void operator += (char _value);
		void operator += (const std::string& _value);
		void operator += (const UTF8_String& _value);
		
		u32& operator [] (u64 _char_pos) const;

		std::vector<u32>::const_iterator begin() const;
		std::vector<u32>::const_iterator end() const;

		u64 size() const;
		u64 length() const;

		u32 at(u64 _char_pos) const;

		bool empty() const;

		void reserve(u64 _size);
		void append(const char* _value);
		std::vector<u32>::iterator erase(std::vector<u32>::const_iterator _position);
		std::vector<u32>::iterator erase(u64 _position, u64 _length);

		void trim();

	};

}