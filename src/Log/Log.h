#pragma once

#include <spdlog/logger.h>
#include <spdlog/sinks/dist_sink.h>
#include <memory>

// global log object
// initially empty
// any part of the code can add sinks to Sinks
extern std::shared_ptr<spdlog::logger> Log;
// multithreaded
// TODO make this depend on bgfx setting
extern std::shared_ptr<spdlog::sinks::dist_sink_mt> Sinks;
