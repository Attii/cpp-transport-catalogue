#include <iostream>

#include "json_reader.h"

using namespace json;
using namespace std;

int main() {
    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
     * с ответами.
     * Вывести в stdout ответы в виде JSON
     */

    Document input_json(Load(cin));
    InputReader reader;
    reader.ParseJSON(input_json.GetRoot());
    TransportCatalogue catal;
    reader.ApplyBaseRequests(catal);
    auto arr = reader.ApplyStatRequests(catal);

    json::PrintNode(arr, cout);
}