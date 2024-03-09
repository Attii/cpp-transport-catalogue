#include <iostream>

#include "json_reader.h"
#include "request_handler.h"

using namespace json;
using namespace std;

int main() {
    TransportCatalogue catalogue;
    MapRenderer renderer;
    
    Document input_json(Load(cin));
    // RequestHandler handler(catalogue, renderer);

    // handler.ProcessJSON(input_json.GetRoot());
    // handler.RenderMap().Render(cout);

    InputReader reader;

    reader.ParseJSON(input_json.GetRoot());

    reader.ApplyBaseRequests(catalogue);

    renderer.SetRenderSettings(reader.GetRenderSettings());
    renderer.SetSphereProjector(catalogue);

    auto res = reader.ApplyStatRequests(catalogue, renderer);
    json::PrintNode(res, cout);
}