#pragma once

#include <string>
#include <ctype.h>

namespace CBuild {

	struct String_Helper {

		static void trim(std::string& _str);
		static void lower(std::string& _str);

	};

}