#ifndef INCLUDE_FILEWATCHER_H
#define INCLUDE_FILEWATCHER_H

#include "ForwardDecs.h"
#ifndef OLD_VS
#include <experimental/filesystem>
#endif

class FileWatcher
{
public:
    explicit FileWatcher(const std::string& file, unsigned int delay = 0);

    bool changed();
    bool exists = true;

    const std::string file;

private:
    unsigned int delay;
    unsigned int ticks = 0;
#ifndef OLD_VS
    typedef std::experimental::filesystem::file_time_type file_time_type;
    typedef std::experimental::filesystem::path path;
    file_time_type time;
#endif
};

#endif
