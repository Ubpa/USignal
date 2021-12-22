#pragma once

namespace Ubpa {
	constexpr bool operator<(const Connection& lhs, const void* rhs) {
		return lhs.instance < rhs;
	}

	constexpr bool operator<(const void* lhs, const Connection& rhs) {
		return lhs <= rhs.instance;
	}

	constexpr bool operator<(const Connection& lhs, const Connection& rhs) {
		return lhs.instance < rhs.instance
			|| (lhs.instance == rhs.instance && lhs.funcptr < rhs.funcptr);
	}

	constexpr bool operator==(const Connection& lhs, const Connection& rhs) {
		return lhs.instance == rhs.instance && lhs.funcptr == rhs.funcptr;
	}

	template<typename Func>
	ScopedConnection<Func>::ScopedConnection() noexcept :
		signal{ nullptr } {}

	template<typename Func>
	ScopedConnection<Func>::ScopedConnection(const Connection& conn, Signal<Func>* sig) noexcept :
		Connection{ conn }, signal{ sig }{}

	template<typename Func>
	ScopedConnection<Func>::ScopedConnection(ScopedConnection&& other) noexcept :
		Connection{ std::move(other) }, signal{ other.signal } { other.signal = nullptr; }

	template<typename Func>
	ScopedConnection<Func>::~ScopedConnection() {
		if (signal)
			signal->Disconnect(*this);
	}

	template<typename Func>
	ScopedConnection<Func>& ScopedConnection<Func>::operator=(ScopedConnection&& rhs) noexcept {
		ScopedConnection{ std::move(rhs) }.Swap(*this);
		return *this;
	}

	template<typename Func>
	void ScopedConnection<Func>::Swap(ScopedConnection& other) noexcept {
		std::swap(signal, other.signal);
		std::swap(static_cast<Connection&>(*this), static_cast<Connection&>(other));
	}

	template<typename Func>
	void ScopedConnection<Func>::MoveInstance(void* instance) {
		signal->MoveInstance(instance, this->instance);
		this->instance = instance;
	}

	template<typename Func>
	void ScopedConnection<Func>::Release() {
		if (signal) {
			signal->Disconnect(*this);
			signal = nullptr;
		}
	}


	template<typename Func>
	void ScopedConnection<Func>::Reset() noexcept { signal = nullptr; }
}