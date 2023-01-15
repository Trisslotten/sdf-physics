#pragma once

#include <functional>

template <class Id>
class TypeSafeIdGenerator;

template <class T, class U>
class TypeSafeId {
public:
	friend class TypeSafeIdGenerator<TypeSafeId<T,U>>;
	TypeSafeId() : m_value{0} {}
	explicit TypeSafeId(U value) : m_value{value} {}

	U Value() const {
		return m_value;
	}

	bool IsValid() const {
		return m_value != 0;
	}
	bool operator==(TypeSafeId other) const {
		return m_value == other.m_value;
	}
	bool operator!=(TypeSafeId other) const {
		return m_value != other.m_value;
	}
private:
	U m_value;
};

template <class Id>
class TypeSafeIdGenerator {
public:
	Id Generate() {
		++m_id.m_value;
		return Id{ m_id.m_value };
	}
private:
	Id m_id;
};

namespace std {
	template <class T, class U>
	struct hash<TypeSafeId<T, U>> {
		size_t operator()(const TypeSafeId<T, U>& id) const {
			return hash<U>()(id.Value());
		}
	};
}
