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
		
		// you can only use the result to disconnect with this
		template<typename Slot>
		requires std::negation_v<std::is_pointer<std::remove_cvref_t<Slot>>>
		Connection Connect(Slot&& slot);

		template<typename CallableObject>
		Connection Connect(CallableObject* ptr);

		template<auto funcptr>
		requires std::is_function_v<std::remove_pointer_t<decltype(funcptr)>>
		Connection Connect();

		template<auto memslot, typename T>
		Connection Connect(T* obj);

		template<typename MemSlot, typename T>
		Connection Connect(MemSlot&& memslot, T* obj);

		//
		// Scope Connect
		//////////////////

		// you can only use the result to disconnect with this
		template<typename Slot>
		requires std::negation_v<std::is_pointer<std::remove_cvref_t<Slot>>>
		ScopedConnection<Ret(Args...)> ScopeConnect(Slot&& slot);

		template<typename CallableObject>
		ScopedConnection<Ret(Args...)> ScopeConnect(CallableObject* ptr);

		template<auto funcptr>
		requires std::is_function_v<std::remove_pointer_t<decltype(funcptr)>>
		ScopedConnection<Ret(Args...)> ScopeConnect();

		template<auto memslot, typename T>
		ScopedConnection<Ret(Args...)> ScopeConnect(T* obj);

		template<typename MemSlot, typename T>
		ScopedConnection<Ret(Args...)> ScopeConnect(MemSlot&& memslot, T* obj);

		//
		// Disconnect
		///////////////

		void Disconnect(const Connection& connection);

		template<typename T>
		void Disconnect(const T* ptr);

		template<auto funcptr>
		requires std::is_function_v<std::remove_pointer_t<decltype(funcptr)>>
		void Disconnect();

		template<auto memfuncptr>
		void Disconnect(const member_pointer_traits_object<decltype(memfuncptr)>* obj);

		template<typename MemFuncPtr>
		void Disconnect(MemFuncPtr memfuncptr, const member_pointer_traits_object<MemFuncPtr>* obj);

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
			std::swap(inner_id, other.inner_id);
			std::swap(slots, other.slots);
		}

	private:
		size_t inner_id{ 0 };
		template<typename Slot>
		void ConnectImpl(const Connection& connection, Slot&& slot);

		small_flat_map<Connection, unique_function<FuncSig>, 16, std::less<>> slots;
	};
}

#include "details/Signal.inl"
