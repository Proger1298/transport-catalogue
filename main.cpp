#include <iostream>
#include <string>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

using namespace std;
using namespace transport;

int main() {
    TransportCatalogue transport_catalogue;
    JsonReader json_reader(std::cin);
    json_reader.ApplyBaseRequests(transport_catalogue);

    renderer::RenderSettings render_settings = json_reader.ParseRenderSettings();
    renderer::MapRenderer map_renderer{render_settings};
    transport::TransportRouteSettings route_setiings = json_reader.ParseRouteSettings();
    transport::TransportRouter router{route_setiings, transport_catalogue};
    RequestHandler request_handler(
        transport_catalogue,
        map_renderer,
        router
    );
    
    json_reader.ParseStatAndPrepareAnswer(transport_catalogue, request_handler);
    request_handler.RenderMap().Render(std::cout); // Создание карты
    //json_reader.PrintJSON(std::cout); // Ответ на запросы
}