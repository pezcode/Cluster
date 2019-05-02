#include "Log.h"

std::shared_ptr<spdlog::sinks::dist_sink_mt> Sinks = std::make_shared<spdlog::sinks::dist_sink_mt>();
std::shared_ptr<spdlog::logger> Log = std::make_shared<spdlog::logger>("Cluster", Sinks);
