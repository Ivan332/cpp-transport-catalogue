#include <iostream>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport_catalogue;

int main() {
	TransportCatalogue transport_catalogue;
	input_reader::from_char_stream::ReadDB(transport_catalogue, cin);
	stat_reader::GetInfo(transport_catalogue);
	return 0;
}
