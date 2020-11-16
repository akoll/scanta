#pragma once

#include <boost/callable_traits.hpp>
#include <boost/hana.hpp>

#include "to_hana_tuple_t.hpp"

namespace ct = boost::callable_traits;
namespace hana = boost::hana;

// Get the argument types of some callable as a hana::tuple_t.
template<typename T>
static constexpr auto argtypes_of = to_hana_tuple_t<ct::args_t<T>>;

// Get the type of operator() on some template parameters.
template<typename... Ps>
constexpr auto get_template_callee_type = []<typename T>(T) consteval
	-> decltype(hana::type_c<decltype(&T::type::template operator()<Ps...>)>)
{
	return hana::type_c<decltype(&T::type::template operator()<Ps...>)>;
};

// Get the type of operator() without template parameters.
constexpr auto get_callee_type = []<typename T>(T) consteval
	-> decltype(hana::type_c<decltype(&T::type::operator())>)
{
	return hana::type_c<decltype(&T::type::operator())>;
};

// Determine if operator() is a template.
template<typename T, typename... Ps>
constexpr auto is_template_callable = hana::sfinae(get_template_callee_type<Ps...>)(hana::type_c<T>) != hana::nothing;

// Get the type of operator() and apply some template parameters only if possible.
template<typename T, typename... Ps>
consteval auto get_conditional_callee_type() {
	constexpr auto template_callee = hana::sfinae(get_template_callee_type<Ps...>)(hana::type_c<T>);
	if constexpr (template_callee == hana::nothing)
		return hana::sfinae(get_template_callee_type<Ps...>)(hana::type_c<T>).value();
	else return get_callee_type(hana::type_c<T>).value();
}

// Wrapper for get_conditional_callee_type.
template<typename T, typename... Ps>
using ConditionalCalleeType = typename decltype(get_conditional_callee_type<T, Ps...>())::type;
