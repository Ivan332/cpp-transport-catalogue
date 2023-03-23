#pragma once

#include <istream>
#include <optional>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue::stat_reader {
  
    void GetInfo(const TransportCatalogue& transport_catalogue);

}  // namespace transport_catalogue::stat_reader
