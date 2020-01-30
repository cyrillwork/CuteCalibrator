#ifndef INOTIFYFS_H
#define INOTIFYFS_H

#include <iostream>

#include <string>
#include <thread>
#include <memory>
#include <atomic>

#include <boost/signals2.hpp>

using PtrAction = void(*)(int);

class InotifyFS
{
public:
    InotifyFS(int _id, const std::string &name);
    ~InotifyFS();

    bool Init(PtrAction action);
    void run();

    int eventCheck();

private:
    int fd;
    int id;
    std::string fileName;
    std::atomic_bool isRun;
    std::shared_ptr<std::thread> mainPtr;   
    size_t timeout; //millisec

    boost::signals2::signal<void(int)> OnAction;
};

#endif // INOTIFYFS_H
