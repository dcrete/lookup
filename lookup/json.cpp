#include "lookup/json.h"
#include <fstream>

using namespace lookup;

void lookup::to_json(json_t& json, const ExtrapolationPolicy& policy) {
   using namespace lookup::keys::policy;
   json = json_t{
      { LOWER, static_cast<int_t>(policy.lower) },
      { UPPER, static_cast<int_t>(policy.upper) }
   };
}

void lookup::from_json(const json_t& json, ExtrapolationPolicy& policy) {
   using namespace lookup::keys::policy;
   auto get_mode = [&](const char* name) {
      return static_cast<ExtrapolationMode>(json.at(name).get<int_t>());
   };
   policy.lower = get_mode(LOWER);
   policy.upper = get_mode(UPPER);
}

json_t lookup::load_file(const std::string& path) {
   std::ifstream ifs(path);
   json_t json{};
   ifs >> json;
   return json;
}

void lookup::save_file(const std::string& path, const json_t& json) {
   std::ofstream ofs(path);
   ofs << json;
}