#include "convert/csv.h"
#include <algorithm>
#include <fstream>

using namespace csv;

namespace {
   template<class T, class... Args>
   auto& emplace_back(std::vector<T>& v, Args&&... args) {
      v.emplace_back(std::forward<Args>(args)...);
      return v.back();
   }

   using string_v = std::vector<std::string>;

   auto split(std::string s, const std::string& delim) {
      string_v parts{};
      size_t pos = 0;
      while ((pos = s.find(delim)) != std::string::npos) {
         auto& str = emplace_back(parts, s.substr(0, pos));
         s.erase(0, pos + delim.length());
         while (!str.empty() && str.back() == ' ') {
            str.pop_back();
         }
         while (!str.empty() && str.front() == ' ') {
            str = str.substr(1);
         }
      }
      parts.emplace_back(s);
      return parts;
   }

   std::string read_file(std::istream& stream, const std::string& newline) {
      std::string txt;
      stream.seekg(0, std::ios::end);
      txt.reserve(stream.tellg());
      stream.seekg(0, std::ios::beg);
      using iterator_t = std::istreambuf_iterator<char>;
      txt.assign((iterator_t(stream)), iterator_t());

      auto fix_newline = [&](const std::string& find) {
         if (find == newline) return;
         size_t pos = 0;
         while ((pos = txt.find(find, pos)) != std::string::npos) {
            txt.replace(pos, find.length(), newline);
            pos += newline.length();
         }
      };

      fix_newline("\r\n");
      fix_newline("\r");
      fix_newline("\n");
      return txt;
   }

   template<class Parts>
   auto all_empty(const Parts& parts)-> bool {
      return std::all_of(std::begin(parts),
                         std::end(parts),
                         [](auto& s) -> bool {
         return std::empty(s);
      });
   };
}

void CSV::load(std::istream& stream) {
   headers.clear();
   rows.clear();

   auto append_row = [&](string_v& parts) {
      while (parts.size() > headers.size()) {
         parts.pop_back();
      }
      while (parts.size() < headers.size()) {
         parts.emplace_back();
      }
      auto& row = emplace_back(rows);
      for (auto i = 0U; i < headers.size(); ++i) {
         row[headers[i]] = parts[i];
      }
   };

   const std::string txt = read_file(stream, newline);
   const string_v lines = split(txt, newline);
   for (auto& line : lines) {
      string_v parts = split(line, delimiter);
      if (all_empty(parts)) continue;
      if (headers.empty()) {
         headers = parts;
      }
      else {
         append_row(parts);
      }
   }
}

void CSV::save(std::ostream& stream) const {
   auto append_line = [&](const auto& parts) {
      const auto last = std::end(parts) - 1;
      for (auto it = std::begin(parts); it != last; ++it) {
         stream << *it << delimiter;
      }
      stream << *last << newline;
   };

   append_line(headers);
   for (auto& row : rows) {
      string_v values{};
      for (auto& header : headers) {
         values.emplace_back(row.at(header));
      }
      append_line(values);
   }
}

CSV csv::load_file(const std::string& path) {
   std::ifstream ifs{ path };
   CSV csv{};
   csv.load(ifs);
   return csv;
}

void csv::save_file(const std::string& path, const CSV& csv) {
   std::ofstream ofs{ path };
   csv.save(ofs);
}