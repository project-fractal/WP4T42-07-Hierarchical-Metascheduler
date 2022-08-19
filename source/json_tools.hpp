#ifndef DAKODIS_JSON_TOOLS
#define DAKODIS_JSON_TOOLS

#include <fstream>
#include "utils/json.hpp"

namespace ludwig {
  inline nlohmann::json
  read_json (std::string filename)
  {
    std::ifstream f (filename);
    nlohmann::json tmp;
    f >> tmp;
    f.close ();
    
    return tmp;
  }

  inline void
  dump_json (std::string filename, nlohmann::json data)
  {
    std::ofstream f (filename);
    f << data.dump (2);
    f.close ();
  }
} // namespace ludwig
#endif
