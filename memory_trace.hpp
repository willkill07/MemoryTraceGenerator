#include <type_traits>
#include <string>
#include <tuple>
#include <vector>
#include <functional>
#include <string>
#include <sstream>
#include <iostream>

namespace tt
{

	template<typename>
	struct Void
	{
		typedef void type;
	};

	template <typename T, typename U, typename Sfinae = void>
	struct
	has_operator_brackets
		: std::false_type {};

	template <typename T, typename U>
	struct
	has_operator_brackets
	<T, U, typename Void <decltype (std::declval<T&>()[U{}])>::type>
		: std::true_type {};
}

namespace mem
{

	struct
	trace_storage
	{
		enum Type : char
		{
			READ = 'r' ,
			READI = 'R',
			WRITE = 'w',
			STATUS = 's'
		};

		enum Op : char
		{
			PLUS = '+',
			MINUS = '-',
			MULTIPLIES = '*',
			DIVIDES = '/',
			ASSIGN = '=',
			UNKNOWN = ' '
		};

		using Info = std::string;
		using MemoryAction = std::tuple <Type, Op, Info>;

		static std::vector <MemoryAction> list;
		static void add (Type t, Op o, Info info)
		{
			list.emplace_back (t, o, info);
		}

		template <typename O, typename T>
		struct
		Operator
		{
			static Op get() {
				return UNKNOWN;
			}
		};

		template <typename T>
		struct
		Operator<std::plus<T>, T>
		{
			static Op get() {
				return PLUS;
			}
		};

		template <typename T>
		struct
		Operator<std::minus<T>, T>
		{
			static Op get() {
				return MINUS;
			}
		};

		template <typename T>
		struct
		Operator<std::multiplies<T>, T>
		{
			static Op get() {
				return MULTIPLIES;
			}
		};

		template <typename T>
		struct
		Operator<std::divides<T>, T>
		{
			static Op get() {
				return DIVIDES;
			}
		};

		template <typename T>
		static
		Info
		getInfo (T && val) {
			std::ostringstream os;
			os << val;
			return os.str();
		}

		template <typename T>
		static
		Info
		getInfo (T* && val) {
			std::ostringstream os;
			os << std::hex << (void*)val;
			return os.str();
		}

	};

	using AccessType = std::vector <trace_storage::MemoryAction>;

	std::ostream&
	operator<< (std::ostream& os, const trace_storage::MemoryAction &t) {
		os << (char)std::get<0>(t) << ' '  << (char)std::get<1>(t) << ' ' << std::hex << std::get<2>(t);
		return os;
	}

	template <typename T>
	struct trace;

	template <typename T>
	trace <T>
	make_trace (T && val)
	{
		return trace <T> (val);
	}

	template <typename T>
	struct trace
	{
		T& value;
		void* addr;

		template <typename T1 = T>
		trace (T& v, void* a = nullptr) : value (v), addr(a) {
			if (a == nullptr)
				addr = &v;
    }

		template <typename T1 = T>
		trace (typename std::enable_if <!std::is_array <T1>::value, T&&>::type v, void* a = nullptr)
			: value (v), addr(a) {
			if (a == nullptr) addr = &v;
    }

		template <typename U>
		auto
		operator[] (U index) -> trace<typename std::remove_reference <decltype(value[index])>::type>
		{
			using RetType = typename std::remove_reference <decltype(value[index])>::type;
			return trace<RetType> (value[index], (RetType*)addr + index);
		}

		template <typename U>
		trace <T>&
		operator= (const trace<U> &other) {
			value = other.value;
			trace_storage::add (trace_storage::READ, trace_storage::ASSIGN, trace_storage::getInfo(other.addr));
			trace_storage::add (trace_storage::WRITE, trace_storage::ASSIGN, trace_storage::getInfo(addr));
			return *this;
		}

		trace <T>&
		operator= (trace<T> other) {
			value = other.value;
			trace_storage::add (trace_storage::READ, trace_storage::ASSIGN, trace_storage::getInfo(other.addr));
			trace_storage::add (trace_storage::WRITE, trace_storage::ASSIGN, trace_storage::getInfo(addr));
			return *this;
		}

		template <typename U>
		trace <T>&
		operator= (U other) {
			value = other;
			trace_storage::add (trace_storage::READI, trace_storage::ASSIGN, trace_storage::getInfo(other));
			trace_storage::add (trace_storage::WRITE, trace_storage::ASSIGN, trace_storage::getInfo(addr));
			return *this;
		}

		template <typename Pred, typename U>
		trace<T>
		math_op (const trace<U> & other) {
			trace_storage::add (trace_storage::READ, trace_storage::Operator<Pred, T>::get(), trace_storage::getInfo(other.addr));
			return trace<T> (Pred()(value, other.value), addr);
		}

		template <typename Pred, typename U>
		trace<T>
		math_op (const U &other) {
			trace_storage::add (trace_storage::READI, trace_storage::Operator<Pred, T>::get(), trace_storage::getInfo(other));
			return trace<T> (Pred()(value, other), addr);
		}

		template <typename Pred, typename U>
		trace<T>
		math_op (U other) {
			trace_storage::add (trace_storage::READI, trace_storage::Operator<Pred, T>::get(), trace_storage::getInfo(other));
			return trace<T> (Pred()(value, other), addr);
		}

		template <typename U>
		trace<T>
		operator+ (const trace<U> &other) {
			return math_op <std::plus<T>, U> (other);
		}

		template <typename U>
		trace<T>
		operator+ (const U &other) {
			return math_op <std::plus<T>, U> (other);
		}

		template <typename U>
		trace<T>
		operator+ (U other) {
			return math_op <std::plus<T>, U> (other);
		}

		template <typename U>
		trace<T>
		operator- (const trace<U> other) {
			return math_op <std::minus<T>, U> (other);
		}

		template <typename U>
		trace<T>&
		operator- (const U &other) {
			return math_op <std::minus<T>, U> (other);
		}

		template <typename U>
		trace<T>&
		operator- (U other) {
			return math_op <std::minus<T>, U> (other);
		}

		template <typename U>
		trace<T>
		operator* (const trace<U> other) {
			return math_op <std::multiplies<T>, U> (other);
		}

		template <typename U>
		trace<T>
		operator* (const U &other) {
			return math_op <std::multiplies<T>, U> (other);
		}

		template <typename U>
		trace<T>
		operator* (U other) {
			return math_op <std::multiplies<T>, U> (other);
		}

		template <typename U>
		trace<T>
		operator/ (const trace<U> &other) {
			return math_op <std::divides<T>, U> (other);
		}

		template <typename U>
		trace<T>
		operator/ (const U &other) {
			return math_op <std::divides<T>, U> (other);
		}

		template <typename U>
		trace<T>
		operator/ (U other) {
			return math_op <std::divides<T>, U> (other);
		}

		template <typename U>
		trace<T>&
		operator+= (const trace<U> & other) {
			return (*this = *this + other);
		}

		template <typename U>
		trace<T>&
		operator+= (const U &other) {
			return (*this = *this + other);
		}

		template <typename U>
		trace<T>&
		operator+= (U other) {
			return (*this = *this + other);
		}

		template <typename U>
		trace<T>&
		operator-= (const trace<U> &other) {
			return (*this = *this - other);
		}

		template <typename U>
		trace<T>&
		operator-= (const U &other) {
			return (*this = *this - other);
		}

		template <typename U>
		trace<T>&
		operator-= (U other) {
			return (*this = *this - other);
		}

		template <typename U>
		trace<T>&
		operator*= (const trace<U> &other) {
			return (*this = *this * other);
		}

		template <typename U>
		trace<T>&
		operator*= (const U &other) {
			return (*this = *this * other);
		}

		template <typename U>
		trace<T>&
		operator*= (U other) {
			return (*this = *this * other);
		}

		template <typename U>
		trace<T>&
		operator/= (const trace<U> &other) {
			return (*this = *this / other);
		}

		template <typename U>
		trace<T>&
		operator/= (const U &other) {
			return (*this = *this / other);
		}

		template <typename U>
		trace<T>&
		operator/= (U other) {
			return (*this = *this / other);
		}
	};
}
