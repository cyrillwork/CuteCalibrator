#include "gui/inotifyfs.h"

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/inotify.h>


bool FileIsExist(const std::string& filePath)
{
    bool isExist = false;
    std::ifstream fin(filePath.c_str());

    if(fin.is_open())
        isExist = true;

    fin.close();
    return isExist;
}


InotifyFS::InotifyFS(int _id, const std::string& name):
    fd{-1}
  , id{_id}
  , fileName{name}
  , isRun{false}
  , mainPtr{nullptr}
  , timeout{100}
{        
}

InotifyFS::~InotifyFS()
{
    std::cout << "dectructor InotifyFS begin" << std::endl;
    isRun = false;

    if(mainPtr)
    {
        mainPtr->join();
    }

    if (::close(fd) < 0)
    {
        std::cerr << "error close (fd)" << std::endl;
    }

    std::cout << "dectructor InotifyFS end" << std::endl;
}


bool InotifyFS::Init(PtrAction action)
{
    bool result = true;

    if( !FileIsExist(fileName) )
    {
        return false;
    }

    fd = inotify_init();
    if (fd < 0)
    {
        std::cerr << "error inotify_init ()" << std::endl;
        return false;
    }

    if(inotify_add_watch (fd, fileName.c_str(), IN_ALL_EVENTS) < 0)
    {
        std::cerr << "error inotify_add_watch ()" << fileName << std::endl;
        return false;
    }

    OnAction.connect(action);

    isRun = true;
    mainPtr = std::make_shared<std::thread>(&InotifyFS::run, this);

    return result;
}


void InotifyFS::run()
{
    while (isRun)
    {
        if (eventCheck() > 0)
        {
            char buffer[16384];

            auto r = read (fd, buffer, 16384);
            if(r > 0)
            {
                ;
            }

            OnAction(id);
            break;
        }
    }
}

int InotifyFS::eventCheck()
{
    struct timeval tv = {};

    fd_set rfds = {};
    FD_ZERO (&rfds);
    FD_SET (fd, &rfds);

    tv.tv_sec = (int)(timeout / 1000000);
    tv.tv_usec = timeout - tv.tv_sec*1000000;

    /* Wait until an event happens or we get interrupted
         by a signal that we catch */

    return select (FD_SETSIZE, &rfds, NULL, NULL, &tv);
}
