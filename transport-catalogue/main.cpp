#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    TransportCatalogue::TransportCatalogue catalogue;

// ParseBaseRequest
    ParseBaseRequest(catalogue, cin);

// ParseRequest
    ParseRequest(catalogue, cin);
    
}
