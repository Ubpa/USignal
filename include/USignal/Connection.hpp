#pragma once

#include <type_traits>

namespace Ubpa {
	template<typename... Args>
	class Signal;

	class Connection {
	public:
		struct ID {
			std::size_t first;
			std::size_t second;
		};

		constexpr Connection(ID id) noexcept : id(id) {}

		constexpr bool Valid() const noexcept { return id.first || id.second; }

		friend bool operator<(const Connection& lhs, const Connection& rhs) {
			return lhs.id.first < rhs.id.first
				|| (lhs.id.first == rhs.id.first && lhs.id.second < rhs.id.second);
		}
		friend bool operator==(const Connection& lhs, const Connection& rhs) {
			return lhs.id.first == rhs.id.first && lhs.id.second == rhs.id.second;
		}
		const ID& GetID() const noexcept { return id; }
	private:
		ID id;

		template<typename... Args>
		friend class Signal;
	};
}

