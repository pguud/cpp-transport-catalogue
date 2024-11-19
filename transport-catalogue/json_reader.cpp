#include "json_reader.h"

#include "json_builder.h"

using namespace std::literals;
#include <iostream>

//#include "transport_router.h"
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

        if (root.IsDict()) { 

            const json::Dict& dict = root.AsDict(); 
            for (const auto& [key, value] : dict) { 
                if (key == "render_settings") { 
                    if (value.IsDict()) { 

                        const json::Dict& baseRequests = value.AsDict(); 

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

    // Во входной JSON добавляется ключ routing_settings, значение которого — словарь с двумя ключами
        SetRoutingSettings();
    }

   json::Document JSONReader::GetStatRequests() const {
        using namespace std::literals;

        json::Builder json_builder;
        json_builder.StartArray();
        const json::Node& root = json_document_.GetRoot(); 
 
        if (root.IsDict()) { 
            const json::Dict& dict = root.AsDict(); 

            for (const auto& [key, value] : dict) { 
                if (key == "stat_requests") { 
                    if (value.IsArray()) { 
                        const json::Array& statRequests = value.AsArray(); 
                        for (const auto& request : statRequests) { 
                            if (request.IsDict()) { 
                                const json::Dict& requestDict = request.AsDict(); 
                                int id = requestDict.at("id").AsInt(); 
                                std::string type = requestDict.at("type").AsString(); 

                                if (type == "Map") {
                                    json_builder.StartDict().Key("request_id").Value(id);

                                    std::ostringstream ostream;
                                    DrawMap(catalogue_, GetRenderSettings(), ostream);

                                    json_builder.Key("map").Value(ostream.str()).EndDict();

                                } else if (type == "Bus") { 
                                    std::string name = requestDict.at("name").AsString(); 
                                    json_builder.StartDict();
                                    json_builder.Key("request_id").Value(id);

                                    auto stat = catalogue_.GetInfoBus(name);
                                    if (stat.has_value()) {
                                        json_builder.Key("curvature").Value(stat.value().curvature);
                                        json_builder.Key("route_length").Value(stat.value().route_length);
                                        json_builder.Key("stop_count").Value(stat.value().stops_on_route);
                                        json_builder.Key("unique_stop_count").Value(stat.value().unique_stops);
                                    } else if (!stat.has_value()) {
                                        json_builder.Key("error_message").Value("not found"s);

                                    }

                                    json_builder.EndDict();

                                } else if (type == "Stop") { 
                                    std::string name = requestDict.at("name").AsString(); 
                                    json_builder.StartDict();
                                    json_builder.Key("request_id").Value(id);

                                    auto stat = catalogue_.GetInfoStop(name);
                                    if (stat != nullptr) {
                                        json_builder.Key("buses");
                                        json_builder.StartArray();
                                        
                                        for (const auto& s : *stat) {
                                            json_builder.Value(s);
                                        
                                        }
                                        json_builder.EndArray();
                                        
                                    } else {
                                        json_builder.Key("error_message").Value("not found"s);

                                    }                                
                                    
                                    json_builder.EndDict();
                                } else if (type == "Route") {
                                    ///////////////////////////////////////
                                    std::string from = requestDict.at("from").AsString(); 
                                    std::string to = requestDict.at("to").AsString(); 

                                    Router router{catalogue_, routing_settings_};
                                    std::optional<RouteInfo> stat = router.GetRouteInfo(from, to); 
                                    if (stat.has_value()) {

                                    
                                        json::Array items;

                                        for (const auto* passage: stat.value().passages) {
                                            items.emplace_back(
                                                json::Builder{}
                                                    .StartDict()
                                                    .Key("type"s).Value("Wait"s)
                                                    .Key("stop_name").Value(std::string(passage->start))
                                                    .Key("time"s).Value(passage->wait_time)
                                                    .EndDict()
                                                    .Build().AsDict()

                                            );


                                            items.emplace_back(
                                                json::Builder{}
                                                    .StartDict()
                                                    .Key("type"s).Value("Bus"s)
                                                    .Key("bus").Value(std::string(passage->bus))
                                                    .Key("span_count"s).Value(passage->span_count)
                                                    .Key("time"s).Value(passage->total_time - passage->wait_time)
                                                    .EndDict()
                                                    .Build().AsDict()
                                            );
                                        }

                                        
                                            json_builder.StartDict()
                                            .Key("total_time"s).Value(stat.value().total_time)
                                            .Key("items"s).Value(std::move(items))
                                            .Key("request_id").Value(id)
                                            .EndDict();


                                    } else {
                                        json_builder.StartDict()
                                        .Key("error_message").Value("not found")
                                        .Key("request_id").Value(id)
                                        .EndDict();
                                    }
                                }
                            } 
                        } 
                    } 
                } 
            } 
        }
        json_builder.EndArray();
        return json::Document{json_builder.Build()}; 
    }

    void JSONReader::SetStops() {

        const json::Node& root = json_document_.GetRoot();

        if (root.IsDict()) {
            const json::Dict& dict = root.AsDict();

            for (const auto& [key, value] : dict) {
                if (key == "base_requests") {
                    if (value.IsArray()) {
                        const json::Array& baseRequests = value.AsArray();
                        for (const auto& request : baseRequests) {
                            if (request.IsDict()) {
                                const json::Dict& requestDict = request.AsDict();
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

        if (root.IsDict()) {
            const json::Dict& dict = root.AsDict();

            for (const auto& [key, value] : dict) {
                if (key == "base_requests") {
                    if (value.IsArray()) {
                        const json::Array& baseRequests = value.AsArray();

                        for (const auto& request : baseRequests) {
                            if (request.IsDict()) {
                                const json::Dict& requestDict = request.AsDict();
                                std::string type = requestDict.at("type").AsString();

                                if (type == "Stop") {
                                    std::string name = requestDict.at("name").AsString();
                                    const json::Dict& roadDistances = requestDict.at("road_distances").AsDict();

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

        if (root.IsDict()) {
            const json::Dict& dict = root.AsDict();

            for (const auto& [key, value] : dict) {
                if (key == "base_requests") {
                    if (value.IsArray()) {
                        const json::Array& baseRequests = value.AsArray();

                        for (const auto& request : baseRequests) {
                            if (request.IsDict()) {
                                const json::Dict& requestDict = request.AsDict();
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
    
    void JSONReader::SetRoutingSettings() {
        const json::Node& root = json_document_.GetRoot();
        if (root.IsDict()) {
            const json::Dict& dict = root.AsDict();

            for (const auto& [key, value] : dict) {
                if (key == "routing_settings") {
                    if (value.IsDict()) {
                        const json::Dict& routingDict = value.AsDict();
                        
                        routing_settings_.wait_time = routingDict.at("bus_wait_time").AsInt();
                        routing_settings_.velocity = routingDict.at("bus_velocity").AsInt();
                    }
                }
            }
        }   
    }
