#pragma once

#include "details/Util.h"

#include <type_traits>
#include <algorithm>

namespace Ubpa {
	template<typename Func>
	class Signal;

	struct Connection {
		void* instance;
		details::FuncPtr funcptr;
	};

	constexpr bool operator<(const Connection& lhs, const void* rhs);
	constexpr bool operator<(const void* lhs, const Connection& rhs);
	constexpr bool operator<(const Connection& lhs, const Connection& rhs);
	constexpr bool operator==(const Connection& lhs, const Connection& rhs);

	// You need to ensure that the life of the signal is longer than the life of the scpoed connection
	// The signal is movable, so you need to change the signal pointer of the scpoed connection when moving the signal
	template<typename Func>
	struct ScopedConnection : Connection {
		Signal<Func>* signal;

		ScopedConnection() noexcept;
		ScopedConnection(const Connection& conn, Signal<Func>* sig) noexcept;
		ScopedConnection(ScopedConnection&& other) noexcept;
		~ScopedConnection();

		ScopedConnection& operator=(ScopedConnection&& rhs) noexcept;

		void Swap(ScopedConnection& other) noexcept;

		void MoveInstance(void* instance);

		void Release();

		ScopedConnection(const ScopedConnection&) = delete;
		ScopedConnection& operator=(const ScopedConnection&) = delete;
	};
	template<typename Func>
	ScopedConnection(const Connection&, Signal<Func>*)->ScopedConnection<Func>;
}

#include "details/Connection.inl"
