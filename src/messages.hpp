#pragma once

#include <spdlog/spdlog.h>

#include <string>

namespace Message {

namespace Error {

void unknownType(std::string type) {
    spdlog::error("Unknown type {}", type);
}

}
    
}