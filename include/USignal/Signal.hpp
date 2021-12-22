#pragma once

#include "Connection.hpp"

#include <UFunction.hpp>

#include <USmallFlat/small_flat_map.hpp>

#include <UTemplate/Func.hpp>

#include <functional>
#include <variant>

namespace Ubpa {
	template<typename Func>
	class Signal;

	// you can register function R(Ts...)
	// require
	// - if Ret is void, R can be any type, else R should be implicit convertible to Ret
	// - Ts... must be compatible with Args...
	// for example
	// float(const int&) is compatible with void(int)
	template<typename Ret, typename... Args>
	class Signal<Ret(Args...)> {
		using FuncSig = Ret(void*, Args...);

	public:
		Signal() = default;
		Signal(const Signal&) = delete;
		Signal& operator=(const Signal&) = delete;
		Signal(Signal&&) noexcept = default;
		Signal& operator=(Signal&&) noexcept = default;

		//
		// Connect
		////////////

		template<typename CallableObject>
		Connection Connect(CallableObject* ptr);
		
		// you can only use the result to disconnect with this
		template<typename Slot>
		Connection Connect(Slot&& slot);

		template<auto funcptr>
		Connection Connect();

		// memslot
		// - member function pointer
		// - function pointer, the first argument is treated as the object, it can be a pointer or reference
		// if T is const, the object type in memslot must also be const
		template<auto memslot, typename T>
		Connection Connect(T* obj);

		// memslot
		// 1. member function pointer
		// 2. function pointer, the first argument is treated as the object, it can be a pointer or reference
		// 3. callable object
		// in case 1 and 2, we use memslot as the funcptr of the result connection
		// if T is const, the object type in memslot must also be const
		template<typename MemSlot, typename T>
		Connection Connect(MemSlot&& memslot, T* obj);

		//
		// Scope Connect
		//////////////////
		// You need to ensure that the life of the signal is longer than the life of the scpoed connection
		// The signal is movable, so you need to change the signal pointer of the scpoed connection when moving the signal

		template<typename CallableObject>
		ScopedConnection<Ret(Args...)> ScopeConnect(CallableObject* ptr);

		// you can only use the result to disconnect with this
		template<typename Slot>
		ScopedConnection<Ret(Args...)> ScopeConnect(Slot&& slot);

		template<auto funcptr>
		ScopedConnection<Ret(Args...)> ScopeConnect();

		// memslot
		// - member function pointer
		// - function pointer, the first argument is treated as the object, it can be a pointer or reference
		// if T is const, the object type in memslot must also be const
		template<auto memslot, typename T>
		ScopedConnection<Ret(Args...)> ScopeConnect(T* obj);

		// memslot
		// 1. member function pointer
		// 2. function pointer, the first argument is treated as the object, it can be a pointer or reference
		// 3. other callable object
		// in case 1 and 2, we use memslot as the funcptr of the result connection
		// if T is const, the object type in memslot must also be const
		template<typename MemSlot, typename T>
		ScopedConnection<Ret(Args...)> ScopeConnect(MemSlot&& memslot, T* obj);

		//
		// Disconnect
		///////////////

		void Disconnect(const Connection& connection);

		template<typename T>
		void Disconnect(const T* ptr);

		template<auto funcptr>
		void Disconnect();

		// memslot
		// - member function pointer
		// - function pointer, the first argument is treated as the object, it can be a pointer or reference
		template<auto memslot>
		void Disconnect(const details::ObjectTypeOfGeneralMemFunc_t<decltype(memslot)>* obj);

		// memslot
		// - member function pointer
		// - function pointer, the first argument is treated as the object, it can be a pointer or reference
		// unlike similar Connect API, the memslot can't be other callable object
		template<typename MemSlot>
		void Disconnect(MemSlot memslot, const details::ObjectTypeOfGeneralMemFunc_t<MemSlot>* obj);

		//
		// Emit
		/////////

		void Emit(Args... args);

		template<typename Acc> requires std::negation_v<std::is_void<Ret>>
		void Emit(Acc&& acc, Args... args);

		//
		// Modify
		///////////

		template<typename T>
		void MoveInstance(T* dst, const T* src);

		void Clear() noexcept;

		void Swap(Signal& other) noexcept {
			std::swap(innerID, other.innerID);
			std::swap(slots, other.slots);
		}

	private:
		size_t innerID{ 0 };
		template<typename Slot>
		void ConnectImpl(const Connection& connection, Slot&& slot);
		bool isEmitting{ false };
		small_flat_map<Connection, unique_function<FuncSig>, 16, std::less<>> slots;
	};
}

#include "details/Signal.inl"
