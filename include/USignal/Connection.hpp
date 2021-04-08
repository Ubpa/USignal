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

		friend bool operator<(const Connection& lhs, const void* rhs) {
			return lhs.instance < rhs;
		}
		friend bool operator<(const void* lhs, const Connection& rhs) {
			return lhs <= rhs.instance;
		}
		friend bool operator<(const Connection& lhs, const Connection& rhs) {
			return lhs.instance < rhs.instance
				|| (lhs.instance == rhs.instance && lhs.funcptr < rhs.funcptr);
		}
		friend bool operator==(const Connection& lhs, const Connection& rhs) {
			return lhs.instance == rhs.instance && lhs.funcptr == rhs.funcptr;
		}
	};

	// You need to ensure that the life of the signal is longer than the life of the scpoed connection
	// The signal is movable, so you need to change the signal pointer of the scpoed connection when moving the signal
	template<typename Func>
	struct ScopedConnection : Connection {
		Signal<Func>* signal;

		ScopedConnection() : signal{ nullptr } {}
		ScopedConnection(const Connection& conn, Signal<Func>* sig) : Connection{ conn }, signal{ sig }{}
		ScopedConnection(ScopedConnection&& other) noexcept :
			Connection{ std::move(other) }, signal{ other.signal } { other.signal = nullptr; }
		~ScopedConnection() {
			if (signal)
				signal->Disconnect(*this);
		}

		ScopedConnection& operator=(ScopedConnection&& rhs) noexcept {
			ScopedConnection{ std::move(rhs) }.Swap(*this);
			return *this;
		}

		void Swap(ScopedConnection& other) noexcept {
			std::swap(signal, other.signal);
			std::swap(static_cast<Connection&>(*this), static_cast<Connection&>(other));
		}

		void MoveInstance(void* instance) {
			signal->MoveInstance(instance, this->instance);
			this->instance = instance;
		}

		void Release() {
			if (signal) {
				signal->Disconnect(*this);
				signal = nullptr;
			}
		}

		ScopedConnection(const ScopedConnection&) = delete;
		ScopedConnection& operator=(const ScopedConnection&) = delete;
	};
	template<typename Func>
	ScopedConnection(const Connection&, Signal<Func>*)->ScopedConnection<Func>;
}
