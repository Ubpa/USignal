#pragma once

#include "details/Util.h"

#include <type_traits>
#include <algorithm>

namespace Ubpa {
	struct Connection {
		void* instance;
		details::FuncPtr funcptr;

		friend bool operator<(const Connection& lhs, const Connection& rhs) {
			return lhs.instance < rhs.instance
				|| (lhs.instance == rhs.instance && lhs.funcptr < rhs.funcptr);
		}
		friend bool operator==(const Connection& lhs, const Connection& rhs) {
			return lhs.instance == rhs.instance && lhs.funcptr == rhs.funcptr;
		}
	};
}

