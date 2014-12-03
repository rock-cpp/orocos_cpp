#ifndef TRANSFORMERHELPER_H
#define TRANSFORMERHELPER_H

#include <rtt/TaskContext.hpp>
#include <smurf/Smurf.hpp>

class TransformerHelper
{
private:
    static const size_t DEFAULT_CONNECTION_BUFFER_SIZE = 500;
    RTT::ConnPolicy conPolicy;
    smurf::Robot robotConfiguration;
public:
    TransformerHelper(const smurf::Robot &robotConfiguration);
    
    bool configureTransformer(RTT::TaskContext *task);
    
    const RTT::ConnPolicy &getConnectionPolicy();
    void setConnectionPolicy(RTT::ConnPolicy &policy);
};

#endif // TRANSFORMERHELPER_H
