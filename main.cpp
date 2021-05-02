#include <fstream>
#include <iostream>
#include <string_view>
#include "json_reader.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    database::transport_catalogue catalogue;
    auto map = renderer::MapRenderer();
    RequestHandler requestHandler(catalogue, map);
    JsonReader reader;
    router::TransportRouter transportRouter;
    std::fstream file;
    if (mode == "make_base"sv) {
        transportRouter = reader.ReadJsonDocument(std::cin, requestHandler);
        file.open(reader.GetFileName(), std::ios::out | std::ios::binary);
        Serialization(file, requestHandler, transportRouter);
    } else if (mode == "process_requests"sv) {
        reader.ReadFileNameAndStatRequest(std::cin);
        file.open(reader.GetFileName(), std::ios::in | std::ios::binary);
        Deserialization(file, requestHandler, transportRouter);
        reader.WriteStatistic(std::cout, requestHandler, transportRouter);
    } else {
        PrintUsage();
        return 1;
    }
}