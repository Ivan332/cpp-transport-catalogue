#include <iostream>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport_catalogue;

int main() {
	TransportCatalogue transport_catalogue;
	input_reader::from_char_stream::ReadDB(transport_catalogue, cin);

	stat_reader::to_char_stream::StatsPrinter printer(cout);
	stat_reader::from_char_stream::StatsRequestProcessor Out(cin);
	Out.ProcessRequests(transport_catalogue, printer);
	return 0;
}
