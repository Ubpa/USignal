#pragma once

#include <cstddef>
#include <type_traits>
#include <algorithm>

namespace Ubpa::details {
	struct a { virtual ~a() = default; void f(); virtual void g(); };
	struct b { virtual ~b() = default; virtual void h(); };
	struct c : a, b { void g() override; };
	union FuncTypes {
		decltype(&c::g) m;
		decltype(&a::g) v;
		decltype(&a::f) d;
		void (*f)();
		void* o;
	};
	static_assert(sizeof(FuncTypes) % sizeof(std::size_t) == 0);

	template<typename T>
	union FuncPtrStorage {
		T ptr;
		std::size_t data[sizeof(T) / sizeof(std::size_t)];
	};

	union FuncPtr {
		FuncPtr() = default;

		template<typename T>
		constexpr FuncPtr(T t) noexcept {
			static_assert(std::is_function_v<std::remove_pointer_t<T>>
				|| std::is_member_function_pointer_v<T>
				|| std::is_function_v<T>);
			new(data)FuncPtrStorage<T>{ t };
			if constexpr (sizeof(FuncPtrStorage<T>) < sizeof(FuncTypes) / sizeof(std::size_t)) {
				for (std::size_t i = sizeof(FuncPtrStorage<T>); i < sizeof(FuncTypes) / sizeof(std::size_t); i++)
					data[i] = 0;
			}
		}

		friend constexpr bool operator==(const FuncPtr& lhs, const FuncPtr& rhs) noexcept {
			return std::equal(std::begin(lhs.data), std::end(lhs.data), std::begin(rhs.data), std::end(rhs.data));
		}
		friend constexpr bool operator<(const FuncPtr& lhs, const FuncPtr& rhs) noexcept {
			return std::lexicographical_compare(std::begin(lhs.data), std::end(lhs.data), std::begin(rhs.data), std::end(rhs.data));
		}

		FuncTypes _;
		std::size_t data[sizeof(FuncTypes) / sizeof(std::size_t)];
	};
}
