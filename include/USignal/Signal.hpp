#pragma once

#include "Connection.hpp"

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
		template<auto memslot, typename T>
		Connection Connect(const T* obj);

		template<typename MemSlot, typename T>
		Connection Connect(MemSlot&& memslot, T* obj);
		template<typename MemSlot, typename T>
		Connection Connect(MemSlot&& memslot, const T* obj);

		void Emit(Args... args) const;

		template<typename Acc> requires std::negation_v<std::is_void<Ret>>
		void Emit(Acc&& acc, Args... args) const;

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

		template<typename T>
		void Move(const T* dst, const T* src);

		void Clear() noexcept;

	private:
		size_t inner_id{ 0 };
		template<typename Slot>
		void Connect(const Connection& connection, Slot&& slot);

		small_flat_map<Connection, std::function<FuncSig>> slots;
	};
}

#include "details/Signal.inl"
