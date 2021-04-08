#pragma once

namespace Ubpa::details {
	template<typename Func>
	struct SlotExpand;

	// to Ret(void*, Args...)
	template<typename Ret, typename... Args>
	struct SlotExpand<Ret(Args...)> {
		template<auto func>
		static auto get() noexcept {
			constexpr std::size_t N = Length_v<typename FuncTraits<decltype(func)>::ArgList>;
			return get<func>(std::make_index_sequence<N>{});
		}

		// Ret(Object::*)(...)
		// Ret([const] Object&, ...)
		// Ret([const] Object*, ...)
		template<auto memfunc>
		static auto mem_get() noexcept {
			using MemFunc = decltype(memfunc);
			if constexpr (std::is_member_function_pointer_v<std::remove_cvref_t<MemFunc>>) {
				constexpr std::size_t N = Length_v<typename FuncTraits<MemFunc>::ArgList>;
				return rmem_get<memfunc>(std::make_index_sequence<N>{});
			}
			else {
				constexpr std::size_t N = Length_v<typename FuncTraits<MemFunc>::ArgList>;
				return pmem_get<memfunc>(std::make_index_sequence<N - 1>{});
			}
		}

		template<typename Func>
		static auto get(Func&& func) noexcept {
			constexpr std::size_t N = Length_v<typename FuncTraits<Func>::ArgList>;
			return get(std::forward<Func>(func), std::make_index_sequence<N>{});
		}

		// Ret(Object::*)(...)
		// Ret([const] Object&, ...)
		// Ret([const] Object*, ...)
		template<typename MemFunc>
		static auto mem_get(MemFunc&& memfunc) noexcept {
			if constexpr (std::is_member_function_pointer_v<std::remove_cvref_t<MemFunc>>) {
				constexpr std::size_t N = Length_v<typename FuncTraits<MemFunc>::ArgList>;
				return rmem_get(std::forward<MemFunc>(memfunc), std::make_index_sequence<N>{});
			}
			else {
				constexpr std::size_t N = Length_v<typename FuncTraits<MemFunc>::ArgList>;
				return pmem_get(std::forward<MemFunc>(memfunc), std::make_index_sequence<N - 1>{});
			}
		}

	private:
		template<auto func, std::size_t... Ns>
		static auto get(std::index_sequence<Ns...>) {
			using Func = decltype(func);
			using FromArgList = typename FuncTraits<Func>::ArgList;
			using ToArgList = TypeList<Args...>;
			return [](void*, Args... args) mutable -> Ret {
				auto argsT = std::forward_as_tuple(std::forward<Args>(args)...);
				static_assert(details::CheckCompatibleArguments<ToArgList, FromArgList>::value,
					"from and to arguments are not compatible.");
				if constexpr (std::is_void_v<Ret>)
					func(std::get<Ns>(argsT)...);
				else
					return func(std::get<Ns>(argsT)...);
			};
		}

		template<typename Func, std::size_t... Ns>
		static auto get(Func&& func, std::index_sequence<Ns...>) {
			using FromArgList = typename FuncTraits<Func>::ArgList;
			using ToArgList = TypeList<Args...>;
			return [func = std::forward<Func>(func)](void*, Args... args) mutable -> Ret {
				auto argsT = std::forward_as_tuple(std::forward<Args>(args)...);
				static_assert(details::CheckCompatibleArguments<ToArgList, FromArgList>::value,
					"from and to arguments are not compatible.");
				if constexpr (std::is_void_v<Ret>)
					func(std::forward<At_t<FromArgList, Ns>>(std::get<Ns>(argsT))...);
				else
					return func(std::forward<At_t<FromArgList, Ns>>(std::get<Ns>(argsT))...);
			};
		}

		template<auto func, std::size_t... Ns>
		static auto rmem_get(std::index_sequence<Ns...>) {
			using Func = decltype(func);
			using FromArgList = typename FuncTraits<Func>::ArgList;
			using Object = typename FuncTraits<Func>::Object;
			using ToArgList = TypeList<Args...>;
			return [](void* obj, Args... args) mutable -> Ret {
				auto argsT = std::forward_as_tuple(std::forward<Args>(args)...);
				static_assert(details::CheckCompatibleArguments<ToArgList, FromArgList>::value,
					"from and to arguments are not compatible.");
				if constexpr (std::is_void_v<Ret>)
					(reinterpret_cast<Object*>(obj)->*func)(std::get<Ns>(argsT)...);
				else
					return (reinterpret_cast<Object*>(obj)->*func)(std::get<Ns>(argsT)...);
			};
		}

		template<auto func, std::size_t... Ns>
		static auto pmem_get(std::index_sequence<Ns...>) {
			using Func = decltype(func);
			using FromArgList = typename FuncTraits<Func>::ArgList;
			using Object = Front_t<FromArgList>;
			using FromTailArgList = PopFront_t<typename FuncTraits<Func>::ArgList>;
			using ToArgList = TypeList<Args...>;
			return [](void* obj, Args... args) mutable -> Ret {
				auto argsT = std::forward_as_tuple(std::forward<Args>(args)...);
				static_assert(details::CheckCompatibleArguments<ToArgList, FromTailArgList>::value,
					"from and to arguments are not compatible.");
				if constexpr (std::is_void_v<Ret>) {
					if constexpr (std::is_reference_v<Object>)
						func(*reinterpret_cast<std::add_pointer_t<Object>>(obj), std::get<Ns>(argsT)...);
					else if constexpr (std::is_pointer_v<Object>)
						func(reinterpret_cast<Object>(obj), std::get<Ns>(argsT)...);
					else
						static_assert(always_false<Func>);
				}
				else {
					if constexpr (std::is_reference_v<Object>)
						return func(*reinterpret_cast<std::add_pointer_t<Object>>(obj), std::get<Ns>(argsT)...);
					else if constexpr (std::is_pointer_v<Object>)
						return func(reinterpret_cast<Object>(obj), std::get<Ns>(argsT)...);
					else {
						static_assert(always_false<Func>);
						return Ret();
					}
				}
			};
		}

		template<typename Func, std::size_t... Ns>
		static auto rmem_get(Func&& func, std::index_sequence<Ns...>) {
			using FromArgList = typename FuncTraits<Func>::ArgList;
			using Object = typename FuncTraits<Func>::Object;
			using ToArgList = TypeList<Args...>;
			return [func = std::forward<Func>(func)](void* obj, Args... args) mutable -> Ret {
				auto argsT = std::forward_as_tuple(std::forward<Args>(args)...);
				static_assert(details::CheckCompatibleArguments<ToArgList, FromArgList>::value,
					"from and to arguments are not compatible.");
				if constexpr (std::is_void_v<Ret>)
					(reinterpret_cast<Object*>(obj)->*func)(std::get<Ns>(argsT)...);
				else
					return (reinterpret_cast<Object*>(obj)->*func)(std::get<Ns>(argsT)...);
			};
		}

		template<typename Func, std::size_t... Ns>
		static auto pmem_get(Func&& func, std::index_sequence<Ns...>) {
			using FromArgList = typename FuncTraits<Func>::ArgList;
			using Object = Front_t<FromArgList>;
			using FromTailArgList = PopFront_t<typename FuncTraits<Func>::ArgList>;
			using ToArgList = TypeList<Args...>;
			return [func = std::forward<Func>(func)](void* obj, Args... args) mutable -> Ret {
				auto argsT = std::forward_as_tuple(std::forward<Args>(args)...);
				static_assert(details::CheckCompatibleArguments<ToArgList, FromTailArgList>::value,
					"from and to arguments are not compatible.");

				if constexpr (std::is_void_v<Ret>) {
					if constexpr (std::is_reference_v<Object>)
						func(*reinterpret_cast<std::add_pointer_t<Object>>(obj), std::get<Ns>(argsT)...);
					else if constexpr (std::is_pointer_v<Object>)
						func(reinterpret_cast<Object>(obj), std::get<Ns>(argsT)...);
					else
						static_assert(always_false<Func>);
				}
				else {
					if constexpr (std::is_reference_v<Object>)
						return func(*reinterpret_cast<std::add_pointer_t<Object>>(obj), std::get<Ns>(argsT)...);
					else if constexpr (std::is_pointer_v<Object>)
						return func(reinterpret_cast<Object>(obj), std::get<Ns>(argsT)...);
					else {
						static_assert(always_false<Func>);
						return Ret();
					}
				}
			};
		}
	};
}

namespace Ubpa {
	template<typename Ret, typename... Args>
	template<typename Slot>
	void Signal<Ret(Args...)>::ConnectImpl(const Connection& connection, Slot&& slot) {
		if constexpr (std::is_constructible_v<unique_function<FuncSig>, Slot>)
			slots.emplace(connection, std::forward<Slot>(slot));
		else
			ConnectImpl(connection, details::SlotExpand<Ret(Args...)>::template get(std::forward<Slot>(slot)));
	}

	template<typename Ret, typename... Args>
	template<typename Slot>
	Connection Signal<Ret(Args...)>::Connect(Slot&& slot) {
		static_assert(!std::is_pointer_v<Slot>);
		Connection connection{ nullptr, reinterpret_cast<FuncSig*>(inner_id++)};
		ConnectImpl(connection, std::forward<Slot>(slot));
		return connection;
	}

	template<typename Ret, typename... Args>
	template<typename CallableObject>
	Connection Signal<Ret(Args...)>::Connect(CallableObject* ptr) {
		assert(ptr);
		if constexpr (std::is_function_v<CallableObject>)
			return Connect(Connection{ nullptr, reinterpret_cast<std::size_t>(ptr) },
				details::SlotExpand<Ret(Args...)>::template get(ptr));
		else
			return Connect<&CallableObject::operator()>(ptr);
	}

	template<typename Ret, typename... Args>
	template<auto funcptr>
	Connection Signal<Ret(Args...)>::Connect() {
		static_assert(std::is_function_v<std::remove_pointer_t<decltype(funcptr)>>);
		Connection connection{ nullptr, reinterpret_cast<std::size_t>(funcptr) };
		ConnectImpl(connection, details::SlotExpand<Args>::template get<funcptr>());
		return connection;
	}

	template<typename Ret, typename... Args>
	template<auto memslot, typename T>
	Connection Signal<Ret(Args...)>::Connect(T* obj) {
		using MemSlot = decltype(memslot);

		assert(obj);
		
		void* instance;
		if constexpr (std::is_member_function_pointer_v<MemSlot>) {
			static_assert(!std::is_const_v<T> || FuncTraits_is_const<MemSlot>);
			instance = static_cast<member_pointer_traits_object<MemSlot>*>(const_cast<std::remove_const_t<T>*>(obj));
		}
		else {
			static_assert(is_function_pointer_v<decltype(memslot)>);
			using ArgList = FuncTraits_ArgList<MemSlot>;
			using Object = Front_t<ArgList>;
			using UnrefObject = std::remove_reference_t<std::remove_pointer_t<Object>>;
			static_assert(!std::is_const_v<T> || std::is_const_v<UnrefObject>);
			instance = static_cast<std::remove_const_t<UnrefObject>*>(const_cast<std::remove_const_t<T>*>(obj));
		}

		Connection connection{ instance, memslot };
		ConnectImpl(connection, details::SlotExpand<Ret(Args...)>::template mem_get<memslot>());
		return connection;
	}

	template<typename Ret, typename... Args>
	template<typename MemSlot, typename T>
	Connection Signal<Ret(Args...)>::Connect(MemSlot&& memslot, T* obj) {
		assert(obj);

		void* instance;
		details::FuncPtr funcptr;
		if constexpr (std::is_member_function_pointer_v<MemSlot>) {
			static_assert(!std::is_const_v<T> || FuncTraits_is_const<MemSlot>);
			assert(memslot);
			instance = static_cast<member_pointer_traits_object<MemSlot>*>(const_cast<std::remove_const_t<T>*>(obj));
			funcptr = memslot;
		}
		else {
			using ArgList = FuncTraits_ArgList<MemSlot>;
			using Object = Front_t<ArgList>;
			using UnrefObject = std::remove_reference_t<std::remove_pointer_t<Object>>;
			static_assert(!std::is_const_v<T> || std::is_const_v<UnrefObject>);
			instance = static_cast<std::remove_const_t<UnrefObject>*>(const_cast<std::remove_const_t<T>*>(obj));
			if constexpr (is_function_pointer_v<MemSlot>)
				funcptr = memslot;
			else
				funcptr = reinterpret_cast<FuncSig*>(inner_id++);
		}

		Connection connection{ instance, funcptr };
		ConnectImpl(connection, details::SlotExpand<Ret(Args...)>::template mem_get(std::forward<MemSlot>(memslot)));
		return connection;
	}

	template<typename Ret, typename... Args>
	template<typename Slot>
	ScopedConnection<Ret(Args...)> Signal<Ret(Args...)>::ScopeConnect(Slot&& slot)
	{ return { Connect(std::forward<Slot>(slot)), this }; }

	template<typename Ret, typename... Args>
	template<typename CallableObject>
	ScopedConnection<Ret(Args...)> Signal<Ret(Args...)>::ScopeConnect(CallableObject* ptr)
	{ return { Connect(ptr), this }; }

	template<typename Ret, typename... Args>
	template<auto funcptr>
	ScopedConnection<Ret(Args...)> Signal<Ret(Args...)>::ScopeConnect()
	{ return { Connect<funcptr>(), this }; }

	template<typename Ret, typename... Args>
	template<auto memslot, typename T>
	ScopedConnection<Ret(Args...)> Signal<Ret(Args...)>::ScopeConnect(T* obj)
	{ return { Connect<memslot>(obj), this }; }

	template<typename Ret, typename... Args>
	template<typename MemSlot, typename T>
	ScopedConnection<Ret(Args...)> Signal<Ret(Args...)>::ScopeConnect(MemSlot&& memslot, T* obj)
	{ return { Connect(std::forward<MemSlot>(memslot), obj), this }; }

	template<typename Ret, typename... Args>
	void Signal<Ret(Args...)>::Emit(Args... args) {
		for (auto& [c, slot] : slots)
			slot(reinterpret_cast<void*>(c.instance), std::forward<Args>(args)...);
	}

	template<typename Ret, typename... Args>
	template<typename Acc> requires std::negation_v<std::is_void<Ret>>
	void Signal<Ret(Args...)>::Emit(Acc&& acc, Args... args) {
		for (auto& [c, slot] : slots)
			acc(slot(reinterpret_cast<void*>(c.instance), std::forward<Args>(args)...));
	}

	template<typename Ret, typename... Args>
	void Signal<Ret(Args...)>::Disconnect(const Connection& connection) {
		slots.erase(connection);
	}

	template<typename Ret, typename... Args>
	template<typename T>
	void Signal<Ret(Args...)>::Disconnect(const T* ptr) {
		if constexpr (std::is_function_v<T>)
			Disconnect(Connection{ nullptr, ptr });
		else{
			const auto iter_begin = slots.lower_bound(ptr);
			const auto iter_end = slots.end();
			auto cursor = iter_begin;
			while (cursor != iter_end && cursor->first.instance == ptr)
				++cursor;
			slots.erase(iter_begin, cursor);
		}
	}

	template<typename Ret, typename... Args>
	template<auto funcptr>
	void Signal<Ret(Args...)>::Disconnect() {
		static_assert(is_function_pointer_v<decltype(funcptr)>);
		Disconnect(Connection{ nullptr, funcptr });
	}
	
	template<typename Ret, typename... Args>
	template<auto memslot>
	void Signal<Ret(Args...)>::Disconnect(const details::ObjectTypeOfGeneralMemFunc_t<decltype(memslot)>* obj) {
		Disconnect(Connection{const_cast<details::ObjectTypeOfGeneralMemFunc_t<decltype(memslot)>*>(obj), memslot });
	}

	template<typename Ret, typename... Args>
	template<typename MemSlot>
	void Signal<Ret(Args...)>::Disconnect(MemSlot memslot, const details::ObjectTypeOfGeneralMemFunc_t<MemSlot>* obj) {
		Disconnect(Connection{ const_cast<details::ObjectTypeOfGeneralMemFunc_t<MemSlot>*>(obj), memslot });
	}

	template<typename Ret, typename... Args>
	void Signal<Ret(Args...)>::Clear() noexcept {
		slots.clear();
	}

	template<typename Ret, typename... Args>
	template<typename T>
	void Signal<Ret(Args...)>::MoveInstance(T* dst, const T* src) {
		const auto iter_begin = slots.lower_bound(src);
		const auto iter_end = slots.end();
		auto cursor = iter_begin;
		small_vector<std::pair<Connection, unique_function<FuncSig>>> buffer;
		while (cursor != iter_end && cursor->first.instance == src) {
			buffer.emplace_back(Connection{ dst, cursor->first.funcptr }, std::move(cursor->second));
			++cursor;
		}
		slots.erase(iter_begin, cursor);
		slots.insert(std::make_move_iterator(buffer.begin()), std::make_move_iterator(buffer.end()));
	}
}
