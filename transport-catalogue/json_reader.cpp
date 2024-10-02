#include "json_reader.h"

//#include <sstream>

using namespace std::literals;
#include <iostream>
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

    JSONReader::JSONReader(TransportCatalogue::TransportCatalogue& catalogue) 
        : catalogue_(catalogue), json_document_(json::Load(std::cin)) {
    }

    RenderSettings JSONReader::GetRenderSettings() const {
        RenderSettings render_settings_;
        
        const json::Node& root = json_document_.GetRoot(); 

        if (root.IsMap()) { 

            const json::Dict& dict = root.AsMap(); 
            for (const auto& [key, value] : dict) { 
                if (key == "render_settings") { 
                    if (value.IsMap()) { 

                        const json::Dict& baseRequests = value.AsMap(); 

                        render_settings_.width_ = baseRequests.at("width").AsDouble();
                            
                        render_settings_.height_ = baseRequests.at("height").AsDouble();
                        render_settings_.padding_ = baseRequests.at("padding").AsDouble();
                        render_settings_.line_width_ = baseRequests.at("line_width").AsDouble();
                        render_settings_.stop_radius_ = baseRequests.at("stop_radius").AsDouble();
                        render_settings_.bus_label_font_size_ = baseRequests.at("bus_label_font_size").AsInt();

                        
                        for (const auto& xy : baseRequests.at("bus_label_offset").AsArray()) {
                            if (xy.IsDouble()) {
                                render_settings_.bus_label_offset_.push_back(xy.AsDouble());
                            }
                        }

                        render_settings_.stop_label_font_size_ = baseRequests.at("stop_label_font_size").AsInt();

                        for (const auto& xy : baseRequests.at("stop_label_offset").AsArray()) {
                            if (xy.IsDouble()) {
                                render_settings_.stop_label_offset_.push_back(xy.AsDouble());
                            }
                        }


                        if (baseRequests.at("underlayer_color").IsArray()) {
                            render_settings_.underlayer_color_ = ColorToString(baseRequests.at("underlayer_color").AsArray());
                        } else if (baseRequests.at("underlayer_color").IsString()) {
                            render_settings_.underlayer_color_ = baseRequests.at("underlayer_color").AsString();
                        }
                        

                        render_settings_.underlayer_width_ = baseRequests.at("underlayer_width").AsDouble();

                        for (const auto& color : baseRequests.at("color_palette").AsArray()) {
                            if (color.IsString()) {
                                render_settings_.color_palette_.push_back(color.AsString());
                            } else if (color.IsArray()) {
                                render_settings_.color_palette_.push_back(ColorToString(color.AsArray()));
                            }
                        }

                        

                    }    

                } 

            } 

        } 
        return render_settings_;
    }

    std::string JSONReader::ColorToString(json::Node color) const {
        std::string s = "";
        std::stringstream stream;
        
       if (color.IsArray()) {
            if (color.AsArray().size() == 3) {
                // rgb(255,16,12)
                s = "rgb(" + std::to_string(color.AsArray().at(0).AsInt()) + "," 
                    + std::to_string(color.AsArray().at(1).AsInt()) + "," 
                    + std::to_string(color.AsArray().at(2).AsInt()) + ")";

            } else if (color.AsArray().size() == 4) {
                // rgba(255,200,23,0.85)
                s = "rgba(" + std::to_string(color.AsArray().at(0).AsInt()) + "," 
                    + std::to_string(color.AsArray().at(1).AsInt()) + "," 
                    + std::to_string(color.AsArray().at(2).AsInt()) + ",";

                stream << color.AsArray().at(3).AsDouble();
                s += stream.str();
                s +=")";
            }

        } else if (color.IsString()) {
            return color.AsString();
        }   
        return s;
    }

    void JSONReader::ReadBaseRequests() {
        SetStops();
        SetDistanceBetweenStops();
        SetBus();
    }

    json::Document JSONReader::GetStatRequests() const {
        using namespace std::literals;
        json::Array statReq; 
        const json::Node& root = json_document_.GetRoot(); 

 

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

                                if (type == "Map") {
                                    statReq.push_back(json::Node{GetMap(id)}); 

                                } else if (type == "Bus") { 
                                    std::string name = requestDict.at("name").AsString(); 
                                    statReq.push_back(json::Node{GetBusStat(id, name)}); 

                                } else if (type == "Stop") { 
                                    std::string name = requestDict.at("name").AsString(); 
                                    statReq.push_back(json::Node(GetStopStat(id, name))); 
                                } 
                            } 
                        } 
                    } 
                } 
            } 
        }             
        return json::Document{json::Node(statReq)}; 
    }

    void JSONReader::SetStops() {

        const json::Node& root = json_document_.GetRoot();

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

                                    TransportCatalogue::Stop stop;
                                    stop.name = name;
                                    stop.coordinates = {latitude, longitude};

                                    catalogue_.AddStop(stop);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void JSONReader::SetDistanceBetweenStops() {
        const json::Node& root = json_document_.GetRoot();

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
                                    const json::Dict& roadDistances = requestDict.at("road_distances").AsMap();

                                    for (const auto& [road, distance] : roadDistances) {
                                        catalogue_.SetDistanceBetweenStops(name, road, distance.AsDouble());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }        
    }
    
    void JSONReader::SetBus() {

        const json::Node& root = json_document_.GetRoot();

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
                                if (type == "Bus") {
                                    std::string name = requestDict.at("name").AsString();
                                    const json::Array& busStops = requestDict.at("stops").AsArray();

                                    TransportCatalogue::Bus bus;
                                    bus.name = name;

                                    for (const auto& stopName : busStops) {
                                        const std::string& stopNameStr = stopName.AsString();
                                        bus.bus.push_back(catalogue_.FindStop(stopNameStr));

                                    }
                                        
                                    bool is_roundtrip = requestDict.at("is_roundtrip").AsBool();
                                    bus.is_roundtrip = is_roundtrip;

                                    if (!is_roundtrip) {

                                        if (busStops.size() == 1) {
                                            const std::string& stopNameStr = busStops[0].AsString();
                                            
                                            bus.bus.push_back(catalogue_.FindStop(stopNameStr));
                                        } else {

                                            for (int i = busStops.size() - 2; i >= 0; --i) {
                                                const std::string& stopNameStr = busStops[i].AsString();
                                                
                                                bus.bus.push_back(catalogue_.FindStop(stopNameStr));
                    
                                            }
                                        }
                                    }

                                    catalogue_.AddBus(bus);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    json::Dict JSONReader::GetStopStat(int id, const std::string& name) const {
        json::Dict req;

        auto stat = catalogue_.GetInfoStop(name);
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
        
        return req;
    }

    json::Dict JSONReader::GetBusStat(int id, const std::string& name) const {
        json::Dict req;

        auto stat = catalogue_.GetInfoBus(name);

        req["request_id"] = json::Node(id);
        if (stat.has_value()) {
            req["curvature"] = json::Node{stat.value().curvature};
            req["route_length"] = json::Node{stat.value().route_length};
            req["stop_count"] = json::Node{stat.value().stops_on_route};
            req["unique_stop_count"] = json::Node{stat.value().unique_stops};
        } else if (!stat.has_value()) {
            req["error_message"] = json::Node{"not found"s};
        }
        
        return req;
    }
    
    json::Dict JSONReader::GetMap(int id) const {
        json::Dict req;
        req["request_id"] = json::Node(id);

        std::ostringstream ostream;
        DrawMap(catalogue_, GetRenderSettings(), ostream); 

        req["map"] = json::Node(ostream.str()); 
                
        return req;
    }
