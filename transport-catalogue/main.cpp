#include <iostream>

#include "request_handler.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "json_builder.h"

using namespace std; 

int main()
{
    using request_handler::BufferingRequestHandler;
    transport_catalogue::TransportCatalogue transport_catalogue;
    json_reader::BufferingRequestReader request_reader{ cin };
    BufferingRequestHandler request_handler{transport_catalogue, request_reader};
    json_reader::ResponsePrinter response_printer{ cout };
    request_handler.ProcessRequests(response_printer);
    return 0;
}
