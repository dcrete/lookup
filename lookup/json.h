#pragma once

#include <iosfwd>
#include "lookup/lookup.hpp"
#include "nlohmann/json.hpp"

namespace lookup {

   using json_t = nlohmann::json;

   void to_json(json_t& json, const ExtrapolationPolicy& policy);
   void from_json(const json_t& json, ExtrapolationPolicy& policy);

   json_t load_file(const std::string& path);
   void save_file(const std::string& path, const json_t& json);

   namespace keys {
      namespace policy {
         static constexpr auto LOWER = "lower";
         static constexpr auto UPPER = "upper";
      }
      namespace table {
         static constexpr auto AXES = "axes";
         static constexpr auto DATA = "data";
         static constexpr auto POLICIES = "policies";
      }
      namespace map {
         static constexpr auto TABLE = "table";
         static constexpr auto NAME = "name";
         static constexpr auto DIMS = "dims";
      }
   }

   namespace detail {

      template<class Grid, size_t N = dimension_v<Grid>>
      enable_if_1d_t<Grid>
         fill(const json_t& json, Grid& grid) {
         grid = json.get<std::decay_t<Grid>>();
      }

      template<class Grid, size_t N = dimension_v<Grid>>
      enable_if_nd_t<Grid>
         fill(const json_t& json, Grid& grid) {
         for (auto i = 0U; i < std::size(grid); ++i) {
            fill(json[i], grid[i]);
         }
      }

      template<class T>
      struct is_table : std::false_type {};

      template<class T, size_t N>
      struct is_table<table<N, T>> : std::true_type {};

      template<class T>
      struct is_table_map : std::false_type {};

      template<>
      struct is_table_map<table_map> : std::true_type {};
   }

   template<class T, class R = void>
   using enable_if_table_t = std::enable_if_t<value_v<detail::is_table, T>, R>;

   template<class T, class R = void>
   using enable_if_table_map_t = std::enable_if_t<value_v<detail::is_table_map, T>, R>;

   template<class Table>
   enable_if_table_t<Table>
      from_json(const json_t& json, Table& table) {
      using namespace keys::table;
      constexpr size_t N = dimension_v<Table>;
      json.at(POLICIES).get_to(table.policies);
      json.at(AXES).get_to(table.axes);
      resize(table.data, sizes(table.axes));
      detail::fill(json[DATA], table.data);
   }

   template<class Table>
   enable_if_table_t<Table>
      to_json(json_t& json, const Table& table) {
      using namespace keys::table;
      json = json_t{
         { POLICIES, table.policies },
         { AXES, table.axes },
         { DATA, table.data },
      };
   }

   template<class Map>
   enable_if_table_map_t<Map>
      from_json(const json_t& root, Map& map) {
      using namespace keys::map;
      for (auto& json : root) {
         const auto name = json[NAME].get<std::string>();
         const auto dims = json[DIMS].get<size_t>();
         switch (dims) {
         case 1:
            map.emplace(name, json[TABLE].get<table<1>>());
            break;
         case 2:
            map.emplace(name, json[TABLE].get<table<2>>());
            break;
         case 3:
            map.emplace(name, json[TABLE].get<table<3>>());
            break;
         case 4:
            map.emplace(name, json[TABLE].get<table<4>>());
            break;
         case 5:
            map.emplace(name, json[TABLE].get<table<5>>());
            break;
         }
      }
   }

   template<class Map>
   enable_if_table_map_t<Map>
      to_json(json_t& json, const Map& map) {
      using namespace keys::map;
      for (const auto& dim_map : map.data()) {
         const auto dims = dim_map.first;
         for (const auto& pair : dim_map.second) {
            json.emplace_back(
               json_t{
               { NAME, pair.first },
               { DIMS, dims }
               });
            auto write_table = [&](auto table) {
               using table_t = std::decay_t<decltype(table)>;
               json.back()[TABLE] = detail::table_cast<table_t>(*pair.second);
            };
            switch (dims) {
            case 1:
               write_table(table<1>{});
               break;
            case 2:
               write_table(table<2>{});
               break;
            case 3:
               write_table(table<3>{});
               break;
            case 4:
               write_table(table<4>{});
               break;
            case 5:
               write_table(table<5>{});
               break;
            }
         }
      }
   }
}