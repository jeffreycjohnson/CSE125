#include "FileWatcher.h"

FileWatcher::FileWatcher(const std::string& file, unsigned int delay) : file(file), delay(delay)
{
#ifndef OLD_VS
    time = last_write_time(path(file));
#endif
}

bool FileWatcher::changed()
{
#ifndef OLD_VS
    if (ticks++ < delay) return false;
    ticks = 0;
    auto t = last_write_time(path(file));
    if (time != t)
    {
        time = t;
        return true;
	}
#endif
    return false;
}
