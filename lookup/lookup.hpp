#pragma once

#include <map>
#include <memory>
#include <vector>
#include <array>
#include <tuple>
#include <numeric>
#include <algorithm>
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

   // 
   template<class T>
   using type_constant = std::common_type<T>;

   template<size_t N>
   using size_constant = std::integral_constant<size_t, N>;

   template<template<class...> class Trait, class... Args>
   using type_t = typename Trait<Args...>::type;

   template<template<class...> class Trait, class... Args>
   constexpr auto value_v = Trait<Args...>::value;

   // alias because sizeof...(XYZ) is clucky?
   template<class... Args>
   constexpr size_t size_v = sizeof...(Args);

   // 
   template<class T, template<class> class Alloc = std::allocator>
   using vector = std::vector<T, Alloc<T>>;

   template<class T, size_t N>
   using array = std::array<T, N>;

   template<size_t N>
   using int_pack = array<int_t, N>;

   // see traits.hpp
   namespace detail {

      // NOTE: this actually corresponds to the "inner" type of grid (vector)
      template<class T, size_t N, template<class> class Alloc = std::allocator>
      struct grid;

      template<class T, size_t N, template<class> class Alloc = std::allocator>
      using grid_t = typename grid<T, N, Alloc>::type;

      template<class T>
      struct dimension;

      template<class T>
      struct root;
   }

   // alias for the type returned by an index operator
   template<class T, class Index = int_t>
   using inner_t = decltype(std::declval<T>()[std::declval<Index>()]);

   // constexpr variable for the dimensionality (# of dims) of a given type
   template<class T>
   constexpr size_t dimension_v = value_v<detail::dimension, std::decay_t<T>>;

   template<class T>
   using root_t = type_t<detail::root, std::decay_t<T>>;

   template<class T, size_t N, template<class> class Alloc = std::allocator>
   using grid_t = detail::grid_t<T, N, Alloc>;

   template<class T, size_t N, template<class> class Alloc = std::allocator>
   using axes_t = array<vector<T, Alloc>, N>;

   template<size_t N, class T = std::double_t, template<class> class Alloc = std::allocator>
   struct table;

   namespace detail {

      // 'peels' (omits) the first element and returns the remaining array
      template<size_t N>
      constexpr std::enable_if_t<(N > 1), int_pack<N - 1>>
         peel(int_pack<N> vals) {
         int_pack<N - 1> result{};
         for (auto i = 1U; i < N; ++i) {
            result[i - 1] = vals[i];
         }
         return result;
      }

      template<class T, size_t N, template<class> class Alloc>
      struct grid : type_constant<vector<grid_t<T, N - 1, Alloc>>> {};

      template<class T, template<class> class Alloc>
      struct grid<T, 0, Alloc> : type_constant<T> {};


      template<class T>
      struct dimension : size_constant<0> {};

      template<class T, size_t N>
      struct dimension<array<T, N>> : size_constant<N> {};

      template<class T, size_t N, template<class> class Alloc>
      struct dimension<table<N, T, Alloc>> : size_constant<N> {};

      template<class T, template<class> class Alloc>
      struct dimension<vector<T, Alloc>> : size_constant<1U + dimension_v<T>> {};


      template<class T>
      struct root : type_constant<T> {};

      template<class T, size_t N>
      struct root<array<T, N>> : root<T> {};

      template<class T, size_t N, template<class> class Alloc>
      struct root<table<N, T, Alloc>> : root<T> {};

      template<class T, template<class> class Alloc>
      struct root<vector<T, Alloc>> : root<T> {};


      template<class Grid, class R = void>
      using enable_if_1d_t = std::enable_if_t<(dimension_v<Grid> == 1), R>;

      template<class Grid, class R = void>
      using enable_if_nd_t = std::enable_if_t<(dimension_v<Grid> > 1), R>;


      template<class T>
      using at_t = std::add_lvalue_reference_t<root_t<T>>;

      template<class Grid, size_t N = dimension_v<Grid>>
      enable_if_1d_t<Grid, at_t<Grid>>
         at(Grid& grid, int_pack<N> indices) {
         return grid[indices[0]];
      }

      template<class Grid, size_t N = dimension_v<Grid>>
      enable_if_nd_t<Grid, at_t<Grid>>
         at(Grid& grid, int_pack<N> indices) {
         return at(grid[indices[0]], peel(indices));
      }

      template<class Grid, size_t N = dimension_v<Grid>>
      enable_if_1d_t<Grid>
         resize(Grid & grid, int_pack<N> sizes) {
         grid.resize(sizes[0]);
      }

      template<class Grid, size_t N = dimension_v<Grid>>
      enable_if_nd_t<Grid>
         resize(Grid & grid, int_pack<N> sizes) {
         grid.resize(sizes[0]);
         for (auto& row : grid) {
            resize(row, peel(sizes));
         }
      }
   }

   template<class Grid, size_t N = dimension_v<Grid>>
   auto& at(Grid & grid, int_pack<N> indices) {
      return detail::at(grid, indices);
   }

   template<class Grid, size_t N = dimension_v<Grid>>
   void resize(Grid & grid, int_pack<N> sizes) {
      detail::resize(grid, sizes);
   }

   template<class T, size_t N>
   constexpr int_pack<N> sizes(const axes_t<T, N>& axes) {
      int_pack<N> values{};
      for (auto i = 0U; i < N; ++i) {
         values[i] = static_cast<int_t>(std::size(axes[i]));
      }
      return values;
   }

   enum class ExtrapolationMode : int {
      Constant,
      Linear
   };

   struct ExtrapolationPolicy {
      ExtrapolationMode lower = ExtrapolationMode::Constant;
      ExtrapolationMode upper = ExtrapolationMode::Constant;
   };

   template<class T>
   struct bounds {
      int_t lower = 0;
      int_t upper = 0;
      T slope = 0;
   };

   template<class T, template<class> class Alloc>
   auto search_axis(bounds<T>& bounds,
      const ExtrapolationPolicy& policy,
      const vector<T, Alloc>& axis,
      const T& value) {

      bounds.lower = 0;
      bounds.upper = 0;
      bounds.slope = 0;

      if (std::empty(axis)) return;

      if (value >= axis.back()) {
         bounds.upper = static_cast<int_t>(axis.size() - 1);
         bounds.lower = bounds.upper;
         if (policy.upper == ExtrapolationMode::Linear) {
            --bounds.lower;
         }
      }
      else if (value < axis.front()) {
         bounds.lower = 0;
         bounds.upper = bounds.lower;
         if (policy.lower == ExtrapolationMode::Linear) {
            ++bounds.upper;
         }
      }
      else if (value > axis.front()) {
         auto it = std::upper_bound(std::begin(axis), std::end(axis), value);
         bounds.upper = static_cast<int_t>(std::distance(std::begin(axis), it));
         bounds.lower = bounds.upper - 1;
      }

      if (bounds.lower != bounds.upper) {
         const auto& lower_value = axis[bounds.lower];
         const auto& upper_value = axis[bounds.upper];
         bounds.slope = (value - lower_value) / (upper_value - lower_value);
      }
      bounds.lower = std::max(bounds.lower, int_t{ 0 });
      bounds.upper = std::max(bounds.upper, int_t{ 0 });
   }

   template<class T, size_t N>
   using axes_bounds_t = array<bounds<T>, N>;

   template<size_t N>
   using axes_policies_t = array<ExtrapolationPolicy, N>;

   namespace detail {
      template<class T>
      auto linear(const T& y0, const T& y2, const T& slope) {
         return (y0 + (slope * (y2 - y0)));
      }

      template<class Grid, class It>
      enable_if_1d_t<Grid, root_t<Grid>>
         interpolate(const Grid& grid, It it) {
         return linear(grid[it->lower],
            grid[it->upper],
            it->slope);
      }

      template<class Grid, class It>
      enable_if_nd_t<Grid, root_t<Grid>>
         interpolate(const Grid& grid, It it) {
         return linear(interpolate(grid[it->lower], it + 1),
            interpolate(grid[it->upper], it + 1),
            it->slope);
      }
   }

   template<class T, size_t N>
   auto interpolate(const grid_t<T, N>& grid,
      const axes_bounds_t<T, N>& bounds) {
      return detail::interpolate(grid, std::begin(bounds));
   }

   namespace detail {

      template<class Map, class Key>
      auto contains(const Map& map, const Key& key) -> bool {
         return (map.find(key) != map.end());
      }

      struct table_base {
         virtual ~table_base() = default;
      };

      template<class Table>
      auto table_cast(const table_base& base) -> const Table & {
         return static_cast<const Table&>(base);
      }
   }

   template<size_t N, class T, template<class> class Alloc>
   struct table : detail::table_base {
      virtual ~table() = default;

      using int_pack = int_pack<N>;
      using data_t = grid_t<T, N, Alloc>;
      using axes_t = axes_t<T, N, Alloc>;
      using bounds_t = bounds<T>;
      using axes_bounds_t = axes_bounds_t<T, N>;
      using targets_t = array<T, N>;
      using axes_policies_t = axes_policies_t<N>;

      axes_t axes{};
      data_t data{};
      axes_policies_t policies{};

      template<class... Values>
      std::enable_if_t<(N == size_v<Values...>), T>
         lookup(Values&& ... values) const {
         const targets_t targets{ static_cast<T&&>(values)... };
         for (auto i = 0U; i < N; ++i) {
            search_axis(bounds[i], policies[i], axes[i], targets[i]);
         }
         return interpolate(data, bounds);
      }

   private:
      mutable axes_bounds_t bounds{};
   };

   class table_map {
      using table_ptr_t = std::unique_ptr<detail::table_base>;
      using dim_map_t = std::map<std::string, table_ptr_t>;
      using multi_map_t = std::map<size_t, dim_map_t>;
      multi_map_t maps{};

      bool contains(size_t N) const {
         return detail::contains(maps, N);
      }

      bool contains(size_t N, const std::string& name) const {
         if (!contains(N)) return false;
         return detail::contains(maps.at(N), name);
      }

      template<size_t N, class Table = table<N>>
      const Table & get_table(const std::string & name) const {
#ifdef _DEBUG
         if (!contains(N)) {
            std::string msg = "No ";
            msg += std::to_string(N);
            msg += "-D tables found.";
            throw std::runtime_error(msg);
         }
         if (!contains(N, name)) {
            std::string msg = "No ";
            msg += std::to_string(N);
            msg += "-D tables found containing ";
            msg += name;
            msg += ".";
            throw std::runtime_error(msg);
         }
#endif
         const auto& base = maps.at(N).at(name);
         return static_cast<const Table&>(*base);
      }

   public:
      table_map() = default;
      table_map(table_map&&) = default;
      table_map& operator=(table_map&&) = default;

      table_map(const table_map&) = delete;
      table_map& operator=(const table_map&) = delete;

      const auto& data() const {
         return this->maps;
      }

      template<class Table, size_t N = dimension_v<Table>>
      void emplace(const std::string & name, Table && table) {
         if (!contains(N)) {
            maps.emplace(N, dim_map_t{});
         }
         maps.at(N)[name] = table_ptr_t{
            std::make_unique<Table>(std::forward<Table>(table))
         };
      }

      template<class... Values>
      auto lookup(const std::string& name, Values&& ... values) const {
         constexpr size_t N = size_v<Values...>;
         const auto& table = get_table<N>(name);
         return table.lookup(std::forward<Values>(values)...);
      }
   };
}