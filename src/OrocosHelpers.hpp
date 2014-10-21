#ifndef OROCOSHELPERS_H
#define OROCOSHELPERS_H
#include <string>

namespace RTT
{
    class TaskContext;
}

RTT::TaskContext *getClientTask();

void loadAllPluginsInDir(const std::string &path);

class OrocosHelpers
{
};

#endif // OROCOSHELPERS_H
