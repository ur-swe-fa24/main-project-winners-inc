#include "spdlog/spdlog.h"

#include <cstdint>
#include <iostream>
#include <vector>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

int main() 
{
    // change log pattern
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    spdlog::info("Welcome to hello_mongo!");
    spdlog::set_level(spdlog::level::info);

    mongocxx::instance mongo_instance {};
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);

    spdlog::info("Connected to the mongodb!");

    auto db = client["mydb"];
    auto collection = db["test"];

    // Make a json doc and extract an element.
    auto example_doc = make_document(
        kvp("name", "patrick"),
        kvp("college", "hsc"),
        kvp("degress", make_array("bs","ms","phd"))
    );
    auto doc_view = example_doc.view();
    auto name = doc_view["name"];

}
