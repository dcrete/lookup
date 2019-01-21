#pragma once

#include "lookup/detail.hpp"

namespace lookup {

   namespace detail {

      template<class T, size_t N>
      struct grid : type_constant<axis_t<typename grid<T, N - 1>::type>> {};

      template<class T>
      struct grid<T, 0> : type_constant<T> {};


      template<class T>
      struct dimension : size_constant<0> {};

      template<class T, size_t N>
      struct dimension<array<T, N>> : size_constant<N> {};

      template<class T, size_t N>
      struct dimension<table<N, T>> : size_constant<N> {};

      template<class T>
      struct dimension<axis_t<T>> : size_constant<1U + dimension_v<T>> {};


      template<class T>
      struct root : type_constant<T> {};

      template<class T, size_t N>
      struct root<array<T, N>> : root<T> {};

      template<class T, size_t N>
      struct root<table<N, T>> : root<T> {};

      template<class T>
      struct root<axis_t<T>> : root<T> {};


      template<class Grid, class R = void>
      using enable_if_1d_t = std::enable_if_t<(dimension_v<Grid> == 1), R>;

      template<class Grid, class R = void>
      using enable_if_nd_t = std::enable_if_t<(dimension_v<Grid> > 1), R>;
   }

}