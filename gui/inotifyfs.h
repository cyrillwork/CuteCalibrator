#ifndef INOTIFYFS_H
#define INOTIFYFS_H

#include <iostream>
#include <string>
#include <thread>
#include <memory>
#include <atomic>

#include <boost/signals2.hpp>

#include "gtkmm.hpp"

using PtrAction = void(*)(CalibrationArea*, int);

class InotifyFS
{
public:
    InotifyFS(int _id, const std::string &name, CalibrationArea *_area = nullptr);
    ~InotifyFS();

    bool Init(PtrAction action);
    void run();
    int eventCheck();

    static bool FileIsExist(const std::string& filePath);

private:
    int fd;
    int id;
    std::string fileName;
    std::atomic_bool isRun;
    std::shared_ptr<std::thread> mainPtr;   
    size_t timeout; //millisec
    CalibrationArea *area;

    boost::signals2::signal<void(CalibrationArea*, int)> OnAction;
};

#endif // INOTIFYFS_H
