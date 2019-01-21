#include <sstream>
#include <experimental/filesystem>
#include "convert/convert.h"

namespace {

   namespace fs = std::experimental::filesystem;
   using namespace lookup;
   using strings_t = std::vector<std::string>;

   auto get_path(fs::path root, fs::path subdir) {
      const auto path = root / subdir;
      return fs::absolute(path).string();
   };

}

int main(int argc, char** argv) {
   if (argc != 2) {
      std::ostringstream os{};
      os << "Usage:\n";
      os << "\tArg 1: Path to data directory (2d, 3d, 4d directories).";
      throw std::runtime_error(os.str());
   }

   const auto root = fs::path(argv[1]);
   strings_t DIRS{
      get_path(root, "2d"),
      get_path(root, "3d"),
      get_path(root, "4d"),
   };

   std::vector<std::string> csvs{};
   std::vector<std::string> jsons{};
   auto convert = [&](size_t index, auto table) {
      using namespace std::string_literals;
      using convert::load;
      csvs.emplace_back(get_path(DIRS[index], "data.csv"s));
      auto csv = csv::load_file(csvs.back());
      load(csv, table);
      json_t json{};
      json = table;
      jsons.emplace_back(get_path(DIRS[index], "data.json"s));
      save_file(jsons.back(), json);
   };

   convert(0, table<2>{});
   convert(1, table<3>{});
   convert(2, table<4>{});

   table_map map1{};
   map1.emplace("table2d", load_file(jsons[0]).get<table<2>>());
   map1.emplace("table3d", load_file(jsons[1]).get<table<3>>());
   map1.emplace("table4d", load_file(jsons[2]).get<table<4>>());
   json_t json1{};
   json1 = map1;
   const auto combined = get_path(root, "combined.json");
   save_file(combined, json1);

   auto json2 = load_file(combined);
   auto map2 = json2.get<table_map>();

   std::cout << map2.lookup("table2d", 2.0, 0.1) << "\n";
   return 0;
}