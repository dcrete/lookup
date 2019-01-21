#pragma once

#include <numeric>
#include <algorithm>
#include "lookup/traits.hpp"

namespace lookup {

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

   template<class T>
   auto search_axis(bounds<T>& bounds,
                    const ExtrapolationPolicy& policy,
                    const axis_t<T>& axis,
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
}