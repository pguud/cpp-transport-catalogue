#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

static size_t id_color = -1;
static size_t id_color_text = -1;


std::string RenderSettings::GetColor() const {
        if (id_color == color_palette_.size() - 1) {
            id_color = 0;
        } else {
            ++id_color;
        }
        
        json::Node color = color_palette_.at(id_color);

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

    std::string RenderSettings::GetColorText() const {
        
    json::Array color_palette_text_ = color_palette_;


        if (id_color_text == color_palette_text_.size() - 1) {
            id_color_text = 0;
        } else {
            ++id_color_text;
        }
        
        json::Node color = color_palette_text_.at(id_color_text);

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

    std::vector<double> RenderSettings::GetBusLabelOffset() const {
        std::vector<double> res;
        for (const auto& xy : bus_label_offset_) {
            if (xy.IsDouble()) {
                res.push_back(xy.AsDouble());
            }
        }
        return res;
    }

    std::string RenderSettings::GetUnderlayerColor() const {
        if (underlayer_color_.IsString()) {
            return underlayer_color_.AsString();
        }
        std::string s = "";
        std::stringstream stream;
        if (underlayer_color_.IsArray()) {
            if (underlayer_color_.AsArray().size() == 3) {
                // rgb(255,16,12)
                s = "rgb(" + std::to_string(underlayer_color_.AsArray().at(0).AsInt()) + "," 
                    + std::to_string(underlayer_color_.AsArray().at(1).AsInt()) + "," 
                    + std::to_string(underlayer_color_.AsArray().at(2).AsInt()) + ")";

            } else if (underlayer_color_.AsArray().size() == 4) {
                // rgba(255,200,23,0.85)
                s = "rgba(" + std::to_string(underlayer_color_.AsArray().at(0).AsInt()) + "," 
                    + std::to_string(underlayer_color_.AsArray().at(1).AsInt()) + "," 
                    + std::to_string(underlayer_color_.AsArray().at(2).AsInt()) + ",";

                stream << underlayer_color_.AsArray().at(3).AsDouble();
                s += stream.str();
                s +=")";
            }

        }
        return s;
    }

    std::vector<double> RenderSettings::GetStopLabelOffset() const {
        std::vector<double> res;
        for (const auto& xy : stop_label_offset_) {
            if (xy.IsDouble()) {
                res.push_back(xy.AsDouble());
            }
        }
        return res;
    }

//////////////////////////////////////////////////////////////////////////

void SetRenderSettings(RenderSettings& render, const json::Document& doc) {
    const json::Node& root = doc.GetRoot();

    if (root.IsMap()) {
        const json::Dict& dict = root.AsMap();

        for (const auto& [key, value] : dict) {
            if (key == "render_settings") {
                
                if (value.IsMap()) {
                    const json::Dict& baseRequests = value.AsMap();
                    
                    render.width_ = baseRequests.at("width").AsDouble();
                    render.height_ = baseRequests.at("height").AsDouble();
                    render.padding_ = baseRequests.at("padding").AsDouble();
                    render.line_width_ = baseRequests.at("line_width").AsDouble();
                    render.stop_radius_ = baseRequests.at("stop_radius").AsDouble();

                    render.bus_label_font_size_ = baseRequests.at("bus_label_font_size").AsInt();
                    render.bus_label_offset_ = baseRequests.at("bus_label_offset").AsArray();

                    render.stop_label_font_size_ = baseRequests.at("stop_label_font_size").AsInt();
                    render.stop_label_offset_ = baseRequests.at("stop_label_offset").AsArray();

                    if (baseRequests.at("underlayer_color").IsArray()) {
                        render.underlayer_color_ = baseRequests.at("underlayer_color").AsArray();
                    } else if (baseRequests.at("underlayer_color").IsString()) {
                        render.underlayer_color_ = baseRequests.at("underlayer_color").AsString();
                    }
                    render.underlayer_width_ = baseRequests.at("underlayer_width").AsDouble();

                    render.color_palette_ = baseRequests.at("color_palette").AsArray();
                }   
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////

inline const double EPSILON = 1e-6;
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

// Проецирует широту и долготу в координаты внутри SVG-изображения
svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}


    MapRenderer::MapRenderer(const std::vector<geo::Coordinates>& geo_coords, const RenderSettings& render_settings, std::vector<int> count_stops_in_bus, std::map<std::string, std::vector<geo::Coordinates>> coordinates_of_buses, const TransportCatalogue::TransportCatalogue& catalogue) 
        : geo_coords_(geo_coords), render_settings_(render_settings), count_stops_in_bus_(count_stops_in_bus), coordinates_of_buses_(coordinates_of_buses), catalogue_(catalogue) {
        SphereProjector proj{geo_coords_.begin(), geo_coords_.end(), render_settings_.width_, 
                            render_settings_.height_, render_settings_.padding_};
        proj_ = std::move(proj);
    }

    /*
    void Test() const {
        for (const auto &geo_coord: geo_coords_) {
            using namespace std;

        SphereProjector proj{geo_coords_.begin(), geo_coords_.end(), render_settings_.width_, render_settings_.height_, render_settings_.padding_};

            const svg::Point screen_coord = proj(geo_coord);
            std::cout << '(' << geo_coord.lat << ", "sv << geo_coord.lng << ") -> "sv;
            std::cout << '(' << screen_coord.x << ", "sv << screen_coord.y << ')' << std::endl;
        }
    }
    */

    void MapRenderer::DrawLine(svg::ObjectContainer& container) const {
        using namespace svg;

        std::vector<std::string> color;

        int ind = 0;
        for (const int x : count_stops_in_bus_) {
            svg::Polyline line;

            for (int i = 0; i < x; ++i) {
                line.AddPoint(proj_(geo_coords_[ind++]));

            }
            
            container.Add(line.SetFillColor("none").SetStrokeWidth(render_settings_.line_width_)
                            .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                            .SetStrokeColor(render_settings_.GetColor()));
        }

        

        for (const auto& [bus_name, coordinates] : coordinates_of_buses_) {
            
            if (coordinates.size()) {

                std::string color = render_settings_.GetColorText();
                auto stat = catalogue_.FindBus(bus_name); 

                const Text base_text =  //
                        Text()
                            .SetFontFamily("Verdana")
                            .SetFontSize(render_settings_.bus_label_font_size_)
                            .SetPosition(proj_(coordinates.back()))
                            .SetOffset(svg::Point{render_settings_.GetBusLabelOffset()[0], render_settings_.GetBusLabelOffset()[1]}) //////////// bus_label_offset
                            .SetFontWeight("bold")
                            .SetData(bus_name);
                    container.Add(Text{base_text}
                                .SetStrokeColor(render_settings_.GetUnderlayerColor()) // underlayer_color_
                                .SetFillColor(render_settings_.GetUnderlayerColor()) // underlayer_color_
                                .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                                .SetStrokeLineCap(StrokeLineCap::ROUND)
                                .SetStrokeWidth(render_settings_.underlayer_width_));
                    container.Add(Text{base_text}.SetFillColor(color)); //
                    
                    //std::cout << coordinates.back().lng << " " << coordinates.back().lat << std::endl
                    //<< coordinates.front().lng << " " << coordinates.front().lat << std::endl;
                    //std::cout << bus_name << " " << !stat->is_roundtrip << std::endl;
                    //std::cout << !(coordinates.back() == coordinates.front()) << std::endl;

                    //std::cout << bus_name << std::endl;
                    //const auto bus = catalogue_.FindBus(bus_name);
                    //std::cout << bus->bus.front()->name << " " << bus->bus[bus->bus.size() / 2]->name;



                if (!stat->is_roundtrip) {
                    const auto bus = catalogue_.FindBus(bus_name);
                    if (bus->bus.front()->name != bus->bus[bus->bus.size() / 2]->name) {
                    
                    //std::cout << bus_name << std::endl;
                    
                    // 1 0 = 0
                    /*
                    {
                        
                    const Text base_text =  //
                        Text()
                            .SetFontFamily("Verdana")
                            .SetFontSize(render_settings_.bus_label_font_size_)
                            .SetPosition(proj_(coordinates.front()))
                            .SetOffset(svg::Point{render_settings_.GetBusLabelOffset()[0], render_settings_.GetBusLabelOffset()[1]}) //////////// bus_label_offset
                            .SetFontWeight("bold")
                            .SetData(bus_name);
                    container.Add(Text{base_text}
                                .SetStrokeColor(render_settings_.GetUnderlayerColor()) // underlayer_color_
                                .SetFillColor(render_settings_.GetUnderlayerColor()) // underlayer_color_
                                .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                                .SetStrokeLineCap(StrokeLineCap::ROUND)
                                .SetStrokeWidth(render_settings_.underlayer_width_));
                    container.Add(Text{base_text}.SetFillColor(color)); //
                    }
                    */
                    const Text base_text =  //
                        Text()
                            .SetFontFamily("Verdana")
                            .SetFontSize(render_settings_.bus_label_font_size_)
                            .SetPosition(proj_(coordinates[coordinates.size() / 2]))
                            .SetOffset(svg::Point{render_settings_.GetBusLabelOffset()[0], render_settings_.GetBusLabelOffset()[1]}) //////////// bus_label_offset
                            .SetFontWeight("bold")
                            .SetData(bus_name);
                    container.Add(Text{base_text}
                                .SetStrokeColor(render_settings_.GetUnderlayerColor()) // underlayer_color_
                                .SetFillColor(render_settings_.GetUnderlayerColor()) // underlayer_color_
                                .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                                .SetStrokeLineCap(StrokeLineCap::ROUND)
                                .SetStrokeWidth(render_settings_.underlayer_width_));
                    container.Add(Text{base_text}.SetFillColor(color)); // 

                    }
                } /* else {
                    const Text base_text =  //
                        Text()
                            .SetFontFamily("Verdana")
                            .SetFontSize(render_settings_.bus_label_font_size_)
                            .SetPosition(proj_(coordinates.front()))
                            .SetOffset(svg::Point{render_settings_.GetBusLabelOffset()[0], render_settings_.GetBusLabelOffset()[1]}) //////////// bus_label_offset
                            .SetFontWeight("bold")
                            .SetData(bus_name);
                    container.Add(Text{base_text}
                                .SetStrokeColor(render_settings_.GetUnderlayerColor()) // underlayer_color_
                                .SetFillColor(render_settings_.GetUnderlayerColor()) // underlayer_color_
                                .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                                .SetStrokeLineCap(StrokeLineCap::ROUND)
                                .SetStrokeWidth(render_settings_.underlayer_width_));
                    container.Add(Text{base_text}.SetFillColor(color)); // 
                } */
            }
            
        }

        for (const auto& [stop, coordinate] : catalogue_.GetSetStops()) {
            svg::Circle circle;
            circle.SetCenter(proj_(coordinate)).SetRadius(render_settings_.stop_radius_).SetFillColor("white");
            container.Add(circle);
        }

        for (const auto& [stop, coordinate] : catalogue_.GetSetStops()) {
            const Text base_text =  //
                Text()
                    .SetFontFamily("Verdana")
                    .SetFontSize(render_settings_.stop_label_font_size_)
                    .SetPosition(proj_(coordinate))
                    .SetOffset(svg::Point{render_settings_.GetStopLabelOffset()[0], render_settings_.GetStopLabelOffset()[1]}) //////////// bus_label_offset
                    .SetData(stop);
            container.Add(Text{base_text}
                        .SetStrokeColor(render_settings_.GetUnderlayerColor()) // underlayer_color_
                        .SetFillColor(render_settings_.GetUnderlayerColor()) // underlayer_color_
                        .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                        .SetStrokeLineCap(StrokeLineCap::ROUND)
                        .SetStrokeWidth(render_settings_.underlayer_width_));
            container.Add(Text{base_text}.SetFillColor("black"));
        }
    }

    void MapRenderer::Draw(svg::ObjectContainer& container) const {
        DrawLine(container);
    }

    

void DrawMap(const TransportCatalogue::TransportCatalogue& catalogue, const RenderSettings& render_settings, std::ostringstream& ostream) {

    std::map<std::string, std::vector<geo::Coordinates>> coordinates_of_buses = catalogue.GetCoordinatesAllBuses();

    std::vector<std::unique_ptr<svg::Drawable>> picture;

    std::vector<geo::Coordinates> c;

    std::vector<int> stop_count;
    for (const auto& [name, coordinate] : coordinates_of_buses) {
        stop_count.push_back(coordinate.size());
        for (const auto& coord : coordinate) {
            c.push_back(coord);
        }
    }

    picture.emplace_back(std::make_unique<MapRenderer>(MapRenderer{c, render_settings, stop_count, coordinates_of_buses, catalogue}));

    


    svg::Document doc;
    DrawPicture(picture, doc);
    //doc.Render(std::cout);
    doc.Render(ostream);

}
