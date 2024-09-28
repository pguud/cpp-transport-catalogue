#pragma once

#include "svg.h"
#include "json.h"
#include "geo.h"
#include "transport_catalogue.h"

#include <string>
#include  <sstream>
#include <algorithm>


// #include <iostream>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */


struct RenderSettings {
public:
    double width_;
    double height_;

    double padding_;

    double line_width_;
    double stop_radius_;

    int bus_label_font_size_;

    int stop_label_font_size_;

    double underlayer_width_;

    
    std::string GetColor() const;
    std::string GetColorText() const;
    std::vector<double> GetBusLabelOffset() const;
    std::string GetUnderlayerColor() const;
    std::vector<double> GetStopLabelOffset() const;

private:
    json::Array bus_label_offset_;  

    json::Array stop_label_offset_;

    json::Node underlayer_color_;

    json::Array color_palette_;

    friend void SetRenderSettings(RenderSettings& render, const json::Document& doc);
};

void SetRenderSettings(RenderSettings& render, const json::Document& doc);

bool IsZero(double value);

class SphereProjector {
public:
    SphereProjector() = default;
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer: public svg::Drawable {
public:
    MapRenderer(const std::vector<geo::Coordinates>& geo_coords, const RenderSettings& render_settings, 
                std::vector<int> count_stops_in_bus, std::map<std::string, std::vector<geo::Coordinates>> coordinates_of_buses, 
                const TransportCatalogue::TransportCatalogue& catalogue);

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

    void DrawLine(svg::ObjectContainer& container) const;
    void Draw(svg::ObjectContainer& container) const override;

private:
    std::vector<geo::Coordinates> geo_coords_;
    RenderSettings render_settings_;
    // Создаём проектор сферических координат на карту
    SphereProjector proj_;

    std::vector<int> count_stops_in_bus_;

    std::map<std::string, std::vector<geo::Coordinates>> coordinates_of_buses_;

    TransportCatalogue::TransportCatalogue catalogue_;
};

template <typename DrawableIterator>
void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) {
    for (auto it = begin; it != end; ++it) {
        (*it)->Draw(target);
    }
}

template <typename Container>
void DrawPicture(const Container& container, svg::ObjectContainer& target) {
    using namespace std;
    DrawPicture(begin(container), end(container), target);
}

//void DrawMap(const TransportCatalogue::TransportCatalogue& catalogue, const RenderSettings& render_settings);
void DrawMap(const TransportCatalogue::TransportCatalogue& catalogue, const RenderSettings& render_settings, std::ostringstream& ostream);
