#pragma once

#include "Signal.hpp"

namespace Ubpa {
	// use the signal as a public member object of type T
	// other users can't emit/clear/move/swap the signal, but T can do all of them
	template<typename T, typename Func>
	class MSignal;

	template<typename T, typename Ret, typename... Args>
	class MSignal<T, Ret(Args...)> : protected Signal<Ret(Args...)> {
	public:
		using Signal<Ret(Args...)>::Connect;
		using Signal<Ret(Args...)>::ScopeConnect;
		using Signal<Ret(Args...)>::Disconnect;
		using Signal<Ret(Args...)>::MoveInstance;
	protected:
		friend T;
		MSignal() = default;
		MSignal(MSignal&&) noexcept = default;
		MSignal& operator=(MSignal&&) noexcept = default;
	};
}
