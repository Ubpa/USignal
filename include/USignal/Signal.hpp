#pragma once

#include "Connection.hpp"

#include <USmallFlat/small_flat_map.hpp>

#include <functional>

namespace Ubpa {
	template<typename... Args>
	class Signal {
	public:
		template<typename Slot>
		requires std::negation_v<std::is_pointer<std::remove_cvref_t<Slot>>>
		Connection Connect(Slot&& slot);

		template<typename CallableObject>
		Connection Connect(CallableObject* ptr);

		template<auto FuncPtr>
		requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
		Connection Connect();

		template<auto MemFuncPtr, typename T>
		requires std::is_member_function_pointer_v<decltype(MemFuncPtr)>
		Connection Connect(T* obj);

		template<typename Func, typename T>
		Connection Connect(T* obj, Func(T::*func));

		void Emit(Args... args) const;

		void Disconnect(const Connection& connection);

		template<typename T>
		void Disconnect(T* ptr);

		template<auto FuncPtr>
		requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
		void Disconnect();

		template<auto MemFuncPtr, typename T>
		requires std::is_member_function_pointer_v<decltype(MemFuncPtr)>
		void Disconnect(T* obj);

		template<typename Func, typename T>
		void Disconnect(T* obj, Func(T::* func));

		void Clear() noexcept;

	private:
		size_t id_second{ 1 };

		struct ConnectionLess {
			using is_transparent = int;
			constexpr bool operator()(const Connection& lhs, const std::size_t& rhs) const noexcept {
				return lhs.id.first == 0 && lhs.id.second < rhs;
			}
			constexpr bool operator()(const std::size_t& lhs, const Connection& rhs) const noexcept {
				return rhs.id.first > 0 || (rhs.id.first == 0 && lhs < rhs.id.second);
			}
			constexpr bool operator()(const Connection& lhs, const Connection& rhs) const noexcept {
				return lhs.id.first < rhs.id.first
					|| (lhs.id.first == rhs.id.first && lhs.id.second < rhs.id.second);
			}
		};

		template<typename Slot>
		Connection Connect(Connection::ID id, Slot&& slot);

		small_flat_map<Connection, std::function<void(Args...)>, 16, ConnectionLess> slots;
	};
}

#include "details/Signal.inl"
