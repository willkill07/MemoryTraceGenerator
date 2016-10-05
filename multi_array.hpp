#include <array>

namespace detail
{
	template <class T, size_t Dim, size_t... Dims>
	struct
	multi_array_impl
	{
		using type = std::array <typename multi_array_impl <T, Dims...>::type, Dim>;
	};

	template <class T, size_t Dim>
	struct
	multi_array_impl <T, Dim>
	{
		using type = std::array <T, Dim>;
	};

	template <class T, size_t... Dims>
	using multi_array = typename multi_array_impl <T, Dims...>::type;
}

using detail::multi_array;
