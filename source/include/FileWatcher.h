#ifndef INCLUDE_FILEWATCHER_H
#define INCLUDE_FILEWATCHER_H

#include "ForwardDecs.h"
#include <experimental/filesystem>

class FileWatcher
{
public:
    explicit FileWatcher(const std::string& file, unsigned int delay = 0);

    bool changed();

    const std::string file;

private:
    unsigned int delay;
    int ticks = 0;
    typedef std::experimental::filesystem::file_time_type file_time_type;
    typedef std::experimental::filesystem::path path;
    file_time_type time;
};

#endif
