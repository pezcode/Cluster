#pragma once

#include <assimp/Logger.hpp>
#include <assimp/LogStream.hpp>
#include "Log.h"

class AssimpLogSource : public Assimp::Logger
{
    virtual void OnVerboseDebug(const char* message) override
    {
        Log->trace(message);
    }
    virtual void OnDebug(const char* message) override
    {
        Log->debug(message);
    }
    virtual void OnInfo(const char* message) override
    {
        Log->info(message);
    }
    virtual void OnWarn(const char* message) override
    {
        Log->warn(message);
    }
    virtual void OnError(const char* message) override
    {
        Log->error(message);
    }

    virtual bool attachStream(Assimp::LogStream* pStream, unsigned int severity) override
    {
        delete pStream;
        return true;
    }
    virtual bool detachStream(Assimp::LogStream* pStream, unsigned int severity) override
    {
        return true;
    }
};
