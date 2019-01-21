#pragma once

#include <map>
#include <vector>
#include <string>
#include <iosfwd>

namespace csv {
   namespace detail {
      template<class T>
      struct get {
         auto operator()(const std::string& str) const -> T;
      };

      template<>
      struct get<std::string> {
         auto operator()(const std::string& str) const -> std::string {
            return str;
         }
      };

      template<>
      struct get<int> {
         auto operator()(const std::string& str) const -> int {
            return std::stoi(str);
         }
      };

      template<>
      struct get<double> {
         auto operator()(const std::string& str) const -> double {
            return std::stod(str);
         }
      };
   }

   template<class T>
   std::enable_if_t<!std::is_void_v<T>, T>
      get(const std::string& str) {
      detail::get<T> helper{};
      return helper(str);
   }

   struct CSV {
      using row_t = std::map<std::string, std::string>;
      std::vector<std::string> headers{};
      std::vector<row_t> rows{};

      std::string newline = "\n";
      std::string delimiter = ",";

      void load(std::istream& stream);
      void save(std::ostream& stream) const;

      template<class T>
      auto values(const std::string& key) const {
         std::vector<T> vals{};
         for (const auto& row : rows) {
            vals.emplace_back(get<T>(row.at(key)));
         }
         return vals;
      }
   };

   CSV load_file(const std::string& path);
   void save_file(const std::string& path, const CSV& csv);
}