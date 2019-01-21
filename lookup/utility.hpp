#pragma once

#include "lookup/detail.hpp"
#include "lookup/traits.hpp"

namespace lookup {

   namespace detail {

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
         resize(Grid& grid, int_pack<N> sizes) {
         grid.resize(sizes[0]);
      }

      template<class Grid, size_t N = dimension_v<Grid>>
      enable_if_nd_t<Grid>
         resize(Grid& grid, int_pack<N> sizes) {
         grid.resize(sizes[0]);
         for (auto& row : grid) {
            resize(row, peel(sizes));
         }
      }
   }

   template<class Grid, size_t N = dimension_v<Grid>>
   auto& at(Grid& grid, int_pack<N> indices) {
      return detail::at(grid, indices);
   }

   template<class Grid, size_t N = dimension_v<Grid>>
   void resize(Grid& grid, int_pack<N> sizes) {
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
}