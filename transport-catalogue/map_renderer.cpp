#include "map_renderer.h"


/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

size_t id_color = -1;
size_t id_color_text = -1;

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

    void MapRenderer::DrawBusName(svg::ObjectContainer& container) const {
        using namespace svg;

        for (const auto& [bus_name, coordinates] : coordinates_of_buses_) {
            
            if (coordinates.size()) {

                std::string color = GetColorText();
                auto stat = catalogue_.FindBus(bus_name); 

                const Text base_text =
                        Text()
                            .SetFontFamily("Verdana")
                            .SetFontSize(render_settings_.bus_label_font_size_) 
                            .SetPosition(proj_(coordinates.back()))
                            .SetOffset(svg::Point{render_settings_.bus_label_offset_[0], render_settings_.bus_label_offset_[1]}) 
                            .SetFontWeight("bold")
                            .SetData(bus_name);
                    container.Add(Text{base_text}
                                .SetStrokeColor(render_settings_.underlayer_color_) 
                                .SetFillColor(render_settings_.underlayer_color_) 
                                .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                                .SetStrokeLineCap(StrokeLineCap::ROUND)
                                .SetStrokeWidth(render_settings_.underlayer_width_)); 
                    container.Add(Text{base_text}.SetFillColor(color)); 

                if (!stat->is_roundtrip) {
                    const auto bus = catalogue_.FindBus(bus_name);
                    if (bus->bus.front()->name != bus->bus[bus->bus.size() / 2]->name) {
                    const Text base_text =  
                        Text()
                            .SetFontFamily("Verdana")
                            .SetFontSize(render_settings_.bus_label_font_size_) 
                            .SetPosition(proj_(coordinates[coordinates.size() / 2]))
                            .SetOffset(svg::Point{render_settings_.bus_label_offset_[0], render_settings_.bus_label_offset_[1]}) 
                            .SetFontWeight("bold")
                            .SetData(bus_name);
                    container.Add(Text{base_text}
                                .SetStrokeColor(render_settings_.underlayer_color_) 
                                .SetFillColor(render_settings_.underlayer_color_) 
                                .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                                .SetStrokeLineCap(StrokeLineCap::ROUND)
                                .SetStrokeWidth(render_settings_.underlayer_width_ ));
                    container.Add(Text{base_text}.SetFillColor(color)); 

                    }
                } 
            }
            
        }
    }
    void MapRenderer::DrawStopsCircle(svg::ObjectContainer& container) const { 
        using namespace svg;

        for (const auto& [stop, coordinate] : catalogue_.GetSetStops()) {
            svg::Circle circle;
            circle.SetCenter(proj_(coordinate)).SetRadius(render_settings_.stop_radius_ ).SetFillColor("white");
            container.Add(circle);
        }
    }

    void MapRenderer::DrawStopsName(svg::ObjectContainer& container) const {
        using namespace svg;

        for (const auto& [stop, coordinate] : catalogue_.GetSetStops()) {
            const Text base_text = 
                Text()
                    .SetFontFamily("Verdana")
                    .SetFontSize(render_settings_.stop_label_font_size_)
                    .SetPosition(proj_(coordinate))
                    .SetOffset(svg::Point{render_settings_.stop_label_offset_[0], render_settings_.stop_label_offset_[1]}) 
                    .SetData(stop);
            container.Add(Text{base_text}
                        .SetStrokeColor(render_settings_.underlayer_color_) 
                        .SetFillColor(render_settings_.underlayer_color_) 
                        .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                        .SetStrokeLineCap(StrokeLineCap::ROUND)
                        .SetStrokeWidth(render_settings_.underlayer_width_));   
            container.Add(Text{base_text}.SetFillColor("black"));
        }
    }

    void MapRenderer::DrawLine(svg::ObjectContainer& container) const {
        using namespace svg;

        int ind = 0;
        for (const int x : count_stops_in_bus_) {
            svg::Polyline line;

            for (int i = 0; i < x; ++i) {
                line.AddPoint(proj_(geo_coords_[ind++]));

            }
            container.Add(line.SetFillColor("none").SetStrokeWidth(render_settings_.line_width_)
                            .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                            .SetStrokeColor(GetColor()));
        }
    }

    void MapRenderer::Draw(svg::ObjectContainer& container) const {
        DrawLine(container);
        DrawBusName(container);
        DrawStopsCircle(container);
        DrawStopsName(container);
    }

    
    std::string MapRenderer::GetColor() const {
        if (id_color == render_settings_.color_palette_.size() - 1) {
            id_color = 0;
        } else {
            ++id_color;
        }
        return render_settings_.color_palette_.at(id_color);
    }
    std::string MapRenderer::GetColorText() const {
        if (id_color_text == render_settings_.color_palette_.size() - 1) {
            id_color_text = 0;
        } else {
            ++id_color_text;
        }
        return render_settings_.color_palette_.at(id_color_text);
    }
    

    

void DrawMap(const TransportCatalogue::TransportCatalogue& catalogue, const RenderSettings& render_settings, std::ostringstream& ostream) {

    std::map<std::string, std::vector<geo::Coordinates>> coordinates_of_buses = catalogue.GetCoordinatesAllBuses();
    std::vector<std::unique_ptr<svg::Drawable>> picture;
    std::vector<geo::Coordinates> coord_;

    std::vector<int> stop_count;
    for (const auto& [name, coordinate] : coordinates_of_buses) {
        stop_count.push_back(coordinate.size());
        for (const auto& coord : coordinate) {
            coord_.push_back(coord);
        }
    }

    picture.emplace_back(std::make_unique<MapRenderer>(MapRenderer{coord_, render_settings, stop_count, coordinates_of_buses, catalogue}));

    svg::Document doc;
    DrawPicture(picture, doc);
    doc.Render(ostream);
}
