#include "FileWatcher.h"

FileWatcher::FileWatcher(const std::string& file, unsigned int delay) : file(file), delay(delay)
{
    time = last_write_time(path(file));
}

bool FileWatcher::changed()
{
    if (ticks++ < delay) return false;
    ticks = 0;
    auto t = last_write_time(path(file));
    if (time != t)
    {
        time = t;
        return true;
    }
    return false;
}
