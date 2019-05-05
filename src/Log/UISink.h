#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#include <mutex>

namespace spdlog
{
namespace ext
{

template<typename Mutex, typename Func>
class clusterui_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
    clusterui_sink(Func func) : func(func) {}

protected:
    Func func;

    virtual void sink_it_(const spdlog::details::log_msg& msg) override
    {
        // msg.payload is the raw string without any formatting
        fmt::memory_buffer formatted;
        sink::formatter_->format(msg, formatted);
        func(fmt::to_string(formatted).c_str(), msg.level);
    }

    virtual void flush_() override {}
};

template<typename Func> using clusterui_sink_mt = clusterui_sink<std::mutex, Func>;
template<typename Func> using clusterui_sink_st = clusterui_sink<spdlog::details::null_mutex, Func>;

} // namespace ext
} // namespace spdlog
