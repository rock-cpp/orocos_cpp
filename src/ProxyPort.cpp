#include "ProxyPort.hpp"
#include <boost/lexical_cast.hpp>

std::string ProxyPortBase::getFreePortName(RTT::TaskContext* clientTask, RTT::base::PortInterface* portIf)
{
    int cnt = 0;
    while(true)
    {
        std::string localName = portIf->getName() + boost::lexical_cast<std::string>(cnt);
        if(clientTask->getPort(localName))
        {
            cnt++;
        }
        else
        {
            return localName;
        }
    }
}
