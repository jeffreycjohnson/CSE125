#include "FileWatcher.h"

FileWatcher::FileWatcher(const std::string& file, unsigned int delay) : file(file), delay(delay)
{
#ifndef OLD_VS
    try {
        time = last_write_time(path(file));
    }
    catch (...) {
        LOG(file + " does not exist.");
    }
#endif
}

bool FileWatcher::changed()
{
#ifndef OLD_VS
    if (ticks++ < delay) return false;
    ticks = 0;
    try
    {
        auto t = last_write_time(path(file));
        if (time != t)
        {
            time = t;
            return true;
        }
    }
    catch (...) {}
#endif
    return false;
}
