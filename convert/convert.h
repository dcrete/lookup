#pragma once

#include <iostream>
#include <fstream>
#include <cassert>
#include "convert/csv.h"
#include "lookup/json.h"

namespace convert {

   template<class T, size_t N>
   struct Axes {
      using indices_t = lookup::int_pack<N>;
      using values_t = lookup::array<T, N>;
      using axis_t = lookup::axis_t<T>;
      using axes_t = lookup::axes_t<T, N>;

      axes_t& axes{};
      lookup::array<std::string, N> headers{};

      Axes(axes_t& axes, const csv::CSV& csv) : axes(axes) {
         const auto& keys = csv.headers;
         for (auto i = 0U; i < N; ++i) {
            const auto& header = keys.at(i);
            headers[i] = header;
            auto& axis = axes.at(i);
            axis = csv.values<T>(header);
            std::sort(std::begin(axis), std::end(axis));
            auto last = std::unique(std::begin(axis), std::end(axis));
            axis.erase(last, std::end(axis));
         }
      }

      indices_t indices(const values_t& values) const {
         auto index = [](const auto& axis, const auto& value) {
            auto equals = [&](const auto& val) -> bool {
               return (val == value);
            };
            auto begin = std::begin(axis);
            auto it = std::find_if(begin, std::end(axis), equals);
            return std::distance(begin, it);
         };
         indices_t vals{};
         for (auto i = 0U; i < N; ++i) {
            vals[i] = index(axes[i], values[i]);
         }
         return vals;
      }

      template<class Row>
      values_t values(const Row& row) const {
         values_t values{};
         for (auto i = 0U; i < N; ++i) {
            values[i] = csv::get<T>(row.at(headers.at(i)));
         }
         return values;
      }
   };

   template<class Table>
   void load(const csv::CSV& csv, Table& table) {
      constexpr size_t N = dimension_v<Table>;
      using value_t = std::decay_t<root_t<Table>>;
      Axes<value_t, N> axes(table.axes, csv);
      resize(table.data, sizes(table.axes));
      const auto& header = csv.headers.back();
      for (const auto& row : csv.rows) {
         const auto ivs = axes.values(row);
         const auto indices = axes.indices(ivs);
         auto& v = detail::at(table.data, indices);
         v = csv::get<value_t>(row.at(header));
      }
   }
}