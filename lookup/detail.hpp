#pragma once

#include <memory>
#include <vector>
#include <array>
#include <tuple>
#include <functional>
#include <type_traits>

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

namespace lookup {
   using size_t = std::size_t;

#ifdef ENVIRONMENT64
   using int_t = std::int64_t;
#elif ENVIRONMENT32
   using int_t = std::int32_t;
#endif

   template<class T>
   struct type_constant : std::common_type<T> {};

   template<size_t N>
   struct size_constant : std::integral_constant<size_t, N> {};

   template<template<class...> class Trait, class... Args>
   using type_t = typename Trait<Args...>::type;

   template<template<class...> class Trait, class... Args>
   constexpr auto value_v = Trait<Args...>::value;

   template<class... Args>
   constexpr size_t size_v = sizeof...(Args);

   template<class T, class Alloc = std::allocator<T>>
   using vector = std::vector<T, Alloc>;

   template<class T, size_t N>
   using array = std::array<T, N>;

   template<size_t N>
   using int_pack = array<int_t, N>;

   namespace detail {

      template<class T, size_t N>
      struct grid;

      template<class T, size_t N>
      using grid_t = typename grid<T, N>::type;

      template<class T>
      struct dimension;

      template<class T>
      struct root;
   }

   template<class T, class Index = int_t>
   using inner_t = decltype(std::declval<T>()[std::declval<Index>()]);

   template<class T>
   constexpr size_t dimension_v = value_v<detail::dimension, std::decay_t<T>>;

   template<class T>
   using root_t = type_t<detail::root, std::decay_t<T>>;

   template<class T, size_t N>
   using grid_t = detail::grid_t<T, N>;

   template<class T>
   using axis_t = vector<T>;

   template<class T, size_t N>
   using axes_t = array<axis_t<T>, N>;

   template<size_t N, class T = std::double_t>
   struct table;

   namespace detail {
      template<size_t N>
      constexpr std::enable_if_t<(N > 1), int_pack<N - 1>>
         peel(int_pack<N> vals) {
         int_pack<N - 1> result{};
         for (auto i = 1U; i < N; ++i) {
            result[i - 1] = vals[i];
         }
         return result;
      }
   }
}