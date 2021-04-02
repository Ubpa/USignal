#pragma once

#include <UTemplate/Func.hpp>

namespace Ubpa::details {
	template<typename ArgList>
	struct WrapSlot;
	template<typename... Args>
	struct WrapSlot<TypeList<Args...>> {
		template<auto FuncPtr>
		static constexpr auto get() {
			return [](Args... args) {
				return FuncPtr(std::forward<Args>(args)...);
			};
		};

		template<auto MemFuncPtr, typename T>
		static constexpr auto get(T* obj) {
			return [obj](Args... args) {
				return (obj->*MemFuncPtr)(std::forward<Args>(args)...);
			};
		};

		template<typename MemFuncPtr, typename T>
		static constexpr auto get(MemFuncPtr func, T* obj) {
			return [func, obj](Args... args) {
				return (obj->*func)(std::forward<Args>(args)...);
			};
		};
	};

	template<typename MemFuncPtr>
	struct MemFuncPtrToSizeType {
		using type = MemFuncPtr;
		static_assert(sizeof(type) == sizeof(std::size_t));
		union Storage {
			type ptr;
			std::size_t buffer;
		};
		static constexpr std::size_t get(MemFuncPtr ptr) noexcept {
			Storage storage{ ptr };
			return storage.buffer;
		}
	};
}

namespace Ubpa {
	template<typename... Args>
	template<typename Slot>
	Connection Signal<Args...>::Connect(Connection::ID id, Slot&& slot) {
		if constexpr (std::is_same_v<std::decay_t<Slot>, std::function<void(Args...)>>)
			return slots.insert_or_assign(Connection{ id }, std::forward<Slot>(slot)).first->first;
		else
			return Connect(id, std::function<void(Args...)>{FuncExpand<void(Args...)>::template get(std::forward<Slot>(slot))});
	}

	template<typename... Args>
	template<typename Slot>
	requires std::negation_v<std::is_pointer<std::remove_cvref_t<Slot>>>
	Connection Signal<Args...>::Connect(Slot&& slot) {
		while (slots.contains(id_second))
			id_second++;

		assert(!slots.contains(id_second));
		return Connect(Connection::ID{ 0, id_second }, std::forward<Slot>(slot));
	}

	template<typename... Args>
	template<typename CallableObject>
	Connection Signal<Args...>::Connect(CallableObject* ptr) {
		assert(ptr);
		if constexpr (std::is_function_v<std::remove_cvref_t<CallableObject>>)
			return Connect(Connection::ID{ reinterpret_cast<std::size_t>(ptr), 0 }, ptr);
		else
			return Connect<&CallableObject::operator()>(ptr);
	}

	template<typename... Args>
	template<auto FuncPtr> requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
	Connection Signal<Args...>::Connect() {
		return Connect(Connection::ID{ reinterpret_cast<std::size_t>(FuncPtr), 0},
			details::WrapSlot<FuncTraits_ArgList<decltype(FuncPtr)>>::template get<FuncPtr>());
	}

	template<typename... Args>
	template<auto MemFuncPtr, typename T> requires std::is_member_function_pointer_v<decltype(MemFuncPtr)>
	Connection Signal<Args...>::Connect(T* obj) {
		assert(obj);
		return Connect(
			Connection::ID{
				reinterpret_cast<std::size_t>(obj),
				details::MemFuncPtrToSizeType<decltype(MemFuncPtr)>::get(MemFuncPtr)
			},
			details::WrapSlot<FuncTraits_ArgList<decltype(MemFuncPtr)>>::template get<MemFuncPtr>(obj)
		);
	}

	template<typename... Args>
	template<typename Func, typename T>
	Connection Signal<Args...>::Connect(T* obj, Func(T::* func)) {
		assert(obj);
		return Connect(
			Connection::ID{
				reinterpret_cast<std::size_t>(obj),
				details::MemFuncPtrToSizeType<Func(T::*)>::get(func)
			},
			details::WrapSlot<FuncTraits_ArgList<Func>>::template get(func, obj)
		);
	}

	template<typename... Args>
	void Signal<Args...>::Emit(Args... args) const {
		for (const auto& [id, slot] : slots)
			slot(std::forward<Args>(args)...);
	}

	template<typename... Args>
	void Signal<Args...>::Disconnect(const Connection& connection) {
		slots.erase(connection);
	}

	template<typename... Args>
	template<typename T>
	void Signal<Args...>::Disconnect(T* ptr) {
		auto cursor = slots.begin();
		while (cursor != slots.end()) {
			if (cursor->first.GetID().first == reinterpret_cast<std::size_t>(ptr))
				cursor = slots.erase(cursor);
			else
				++cursor;
		}
	}

	template<typename... Args>
	template<auto FuncPtr>
	requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
	void Signal<Args...>::Disconnect() {
		Disconnect(Connection::ID{ reinterpret_cast<std::size_t>(FuncPtr), 0 });
	}

	template<typename... Args>
	template<auto MemFuncPtr, typename T>
	requires std::is_member_function_pointer_v<decltype(MemFuncPtr)>
	void Signal<Args...>::Disconnect(T* obj) {
		Disconnect(Connection::ID{
			reinterpret_cast<std::size_t>(obj),
			details::MemFuncPtrToSizeType<decltype(MemFuncPtr)>::get(MemFuncPtr)
		});
	}

	template<typename... Args>
	template<typename Func, typename T>
	void Signal<Args...>::Disconnect(T* obj, Func(T::* func)) {
		Disconnect(Connection::ID{
			reinterpret_cast<std::size_t>(obj),
			details::MemFuncPtrToSizeType<Func(T::*)>::get(func)
		});
	}

	template<typename... Args>
	void Signal<Args...>::Clear() noexcept {
		slots.clear();
	}
}
