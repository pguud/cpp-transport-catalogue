#include "json_reader.h"

#include  <sstream>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

using namespace std;

void SetBaseRequestsInTransportCatalogue(TransportCatalogue::TransportCatalogue& catalogue, const json::Document& doc) {
    const json::Node& root = doc.GetRoot();

    if (root.IsMap()) {
        const json::Dict& dict = root.AsMap();

        for (const auto& [key, value] : dict) {
            if (key == "base_requests") {
                
                if (value.IsArray()) {
                const json::Array& baseRequests = value.AsArray();
                
                    for (const auto& request : baseRequests) {
                        if (request.IsMap()) {
                            const json::Dict& requestDict = request.AsMap();
                            std::string type = requestDict.at("type").AsString();

                            if (type == "Stop") {
                                std::string name = requestDict.at("name").AsString();
                                double latitude = requestDict.at("latitude").AsDouble();
                                double longitude = requestDict.at("longitude").AsDouble();
                                const json::Dict& roadDistances = requestDict.at("road_distances").AsMap();

                                TransportCatalogue::Stop stop;
                                stop.name = name;
                                stop.coordinates = {latitude, longitude};

                                catalogue.AddStop(stop);

                                for (const auto& [road, distance] : roadDistances) {
                                    catalogue.SetDistanceBetweenStops(name, road, distance.AsDouble());
                                }
                            }
                        }
                    }
                    
                    for (const auto& request : baseRequests) {
                        if (request.IsMap()) {
                            const json::Dict& requestDict = request.AsMap();
                            std::string type = requestDict.at("type").AsString();

                            if (type == "Stop") {
                                std::string name = requestDict.at("name").AsString();
                                const json::Dict& roadDistances = requestDict.at("road_distances").AsMap();

                                for (const auto& [road, distance] : roadDistances) {
                                    catalogue.SetDistanceBetweenStops(name, road, distance.AsDouble());
                                }
                            }

                            if (type == "Bus") {
                                std::string name = requestDict.at("name").AsString();
                                
                                const json::Array& busStops = requestDict.at("stops").AsArray();
                                TransportCatalogue::Bus bus;
                                bus.name = name;

                                for (const auto& stopName : busStops) {
                                    const std::string& stopNameStr = stopName.AsString();
                                    bus.bus.push_back(catalogue.FindStop(stopNameStr));

                                }
                                
                                bool is_roundtrip = requestDict.at("is_roundtrip").AsBool();
                                bus.is_roundtrip = is_roundtrip;

                                if (!is_roundtrip) {
                                    if (busStops.size() == 1) {
                                        const std::string& stopNameStr = busStops[0].AsString();
                                        
                                        bus.bus.push_back(catalogue.FindStop(stopNameStr));
                                    } else {

                                        for (int i = busStops.size() - 2; i >= 0; --i) {
                                            const std::string& stopNameStr = busStops[i].AsString();
                                            
                                            bus.bus.push_back(catalogue.FindStop(stopNameStr));
                
                                        }
                                    }
                                }

                                catalogue.AddBus(bus);
                            }
                        }
                    }
                }
            } 
        }
    }
}

json::Document SetStateRequestsInDocument(const TransportCatalogue::TransportCatalogue& catalogue, const json::Document& doc) {
    json::Array statReq;

    const json::Node& root = doc.GetRoot();

    if (root.IsMap()) {
        const json::Dict& dict = root.AsMap();

        for (const auto& [key, value] : dict) {
            if (key == "stat_requests") {
                if (value.IsArray()) {
                    const json::Array& statRequests = value.AsArray();
                    for (const auto& request : statRequests) {
                        if (request.IsMap()) {
                            const json::Dict& requestDict = request.AsMap();

                            int id = requestDict.at("id").AsInt();
                            std::string type = requestDict.at("type").AsString();

                            //////////////////////////////
                            if (type == "Map") {
                                json::Dict req;
                                req["request_id"] = json::Node(id);

                                RenderSettings render_settings;
                                SetRenderSettings(render_settings, doc);   

                                std::ostringstream ostream;
                                DrawMap(catalogue, render_settings, ostream);

                                req["map"] = json::Node(ostream.str()); // string res
                                statReq.push_back(json::Node{req});

                            }
                            //////////////////////////////


                            if (type == "Bus") {
                                std::string name = requestDict.at("name").AsString();

                                auto stat = catalogue.GetInfoBus(name);
                                json::Dict req;
                                req["request_id"] = json::Node(id);
                                if (stat.has_value()) {
                                    req["curvature"] = json::Node{stat.value().curvature};
                                    req["route_length"] = json::Node{stat.value().route_length}; // не верно !
                                    req["stop_count"] = json::Node{stat.value().stops_on_route};
                                    req["unique_stop_count"] = json::Node{stat.value().unique_stops};
                                } else {
                                    req["error_message"] = json::Node{"not found"s};
                                }
                                statReq.push_back(json::Node{req});
                                
                                
                            } else if (type == "Stop") {
                                std::string name = requestDict.at("name").AsString();

                                auto stat = catalogue.GetInfoStop(name);
                                json::Dict req;
                                req["request_id"] = json::Node(id);

                                if (stat != nullptr) {
                                    json::Array buses;
                                    for (const auto& s : *stat) {
                                        buses.push_back(s);
                                    }
                                    if (buses.size() > 0) {
                                        req["buses"] = buses;
                                    } else {

                                        req["buses"] = json::Node{json::Array{}};
                                    }
                                } else {
                                    req["error_message"] = json::Node{"not found"s};
                                }
                                statReq.push_back(json::Node(req));
                                
                            }
                        }
                    }
                }
            }
        }
    }
    json::Document res{json::Node(statReq)};            
    return res;
}
