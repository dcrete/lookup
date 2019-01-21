#pragma once

#include <map>
#include "lookup/detail.hpp"
#include "lookup/traits.hpp"
#include "lookup/utility.hpp"
#include "lookup/interpolate.hpp"

namespace lookup {

   namespace detail {

      template<class Map, class Key>
      auto contains(const Map& map, const Key& key) -> bool {
         return (map.find(key) != map.end());
      }

      struct table_base {
         virtual ~table_base() = default;
      };

      template<class Table>
      auto table_cast(const table_base& base) -> const Table& {
         return static_cast<const Table&>(base);
      }
   }

   template<size_t N, class T>
   struct table : detail::table_base {
      virtual ~table() = default;

      using int_pack = int_pack<N>;
      using data_t = grid_t<T, N>;
      using axis_t = axis_t<T>;
      using axes_t = axes_t<T, N>;
      using bounds_t = bounds<T>;
      using axes_bounds_t = axes_bounds_t<T, N>;
      using targets_t = array<T, N>;
      using axes_policies_t = axes_policies_t<N>;

      axes_t axes{};
      data_t data{};
      axes_policies_t policies{};

      template<class... Values>
      std::enable_if_t<(N == size_v<Values...>), T>
         lookup(Values&&... values) const {
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
      const Table& get_table(const std::string& name) const {
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
      void emplace(const std::string& name, Table&& table) {
         if (!contains(N)) {
            maps.emplace(N, dim_map_t{});
         }
         maps.at(N)[name] = table_ptr_t {
            std::make_unique<Table>(std::forward<Table>(table))
         };
      }

      template<class... Values>
      auto lookup(const std::string& name, Values&&... values) const {
         constexpr size_t N = size_v<Values...>;
         const auto& table = get_table<N>(name);
         return table.lookup(std::forward<Values>(values)...);
      }
   };
}