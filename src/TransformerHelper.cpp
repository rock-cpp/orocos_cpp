#include "TransformerHelper.hpp"
#include <rtt/OperationCaller.hpp>
#include <base/samples/RigidBodyState.hpp>
#include <smurf/Smurf.hpp>

#include <transformer/Transformer.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>

class TransformationProvider : public transformer::TransformationElement
{
public:
    TransformationProvider(const std::string& sourceFrame, const std::string& targetFrame, const std::string &providerName, const std::string &portName) : TransformationElement(sourceFrame, targetFrame), providerName(providerName), portName(portName)
    {
    }
    
    virtual bool getTransformation(const base::Time& atTime, bool doInterpolation, transformer::TransformationType& tr)
    {
        return false;
    };
    
    std::string providerName;
    std::string portName;
};

TransformerHelper::TransformerHelper(const smurf::Robot& robotConfiguration): conPolicy(RTT::ConnPolicy::circularBuffer(DEFAULT_CONNECTION_BUFFER_SIZE)), robotConfiguration(robotConfiguration)
{
}


bool TransformerHelper::configureTransformer(RTT::TaskContext* task)
{
    RTT::OperationInterfacePart *op = task->getOperation("getNeededTransformations");

    //test if the task acutally uses the transformer
    if(!op)
        //does not, we take this as successfully configured
        return true;
    
    RTT::OperationCaller< ::std::vector< ::base::samples::RigidBodyState >() >  caller(op);

    ::std::vector< ::base::samples::RigidBodyState > neededTransforms = caller();
    
    transformer::TransformationTree tree;
    
    for(const auto tr : robotConfiguration.getStaticTransforms())
    {
        base::samples::RigidBodyState transform;
        transform.setTransform(tr->getTransformation());
        tree.addTransformation(new transformer::StaticTransformationElement(tr->getSourceFrame().getName(), tr->getTargetFrame().getName(), transform));
    }
    
    for(const auto tr : robotConfiguration.getDynamicTransforms())
    {
        tree.addTransformation(new TransformationProvider(tr->getSourceFrame().getName(), tr->getTargetFrame().getName(), tr->getProviderName(), tr->getProviderPortName()));
    }

    RTT::base::PortInterface *dynamicTransformsPort = task->getPort("dynamic_transformations");
    if(!dynamicTransformsPort)
    {
        std::cout << "Error, given task " << task->getName() << " has not input port 'dynamic_transformations' " << std::endl;
        return false;
    }
    
    for( const base::samples::RigidBodyState &rbs : neededTransforms)
    {
        std::vector<transformer::TransformationElement *> result;
        if(!tree.getTransformationChain(rbs.sourceFrame, rbs.targetFrame, result))
        {
            std::cout << "Error, there is no known transformation from " << rbs.sourceFrame << " to " << rbs.targetFrame << " which is needed by the component " << task->getName() << std::endl;
            return false;
        }

        for(const transformer::TransformationElement *elem: result)
        {
            const TransformationProvider *prov = dynamic_cast<const TransformationProvider *>(elem);
            if(!prov)
            {
                continue;
            }
            
            //get task context and connect them
            RTT::corba::TaskContextProxy *proxy = RTT::corba::TaskContextProxy::Create(prov->providerName, false);
            if(!proxy)
            {
                std::cout << "Error, could not connect to transformation provider '" << prov->providerName << "'" << std::endl;
                return false;
            }
            
            RTT::base::PortInterface *port = proxy->getPort(prov->portName);
            if(!port)
            {
                std::cout << "Error, task " << prov->providerName << " has not port named '" << prov->portName << "'"<< std::endl;
            }
            port->connectTo(dynamicTransformsPort, conPolicy);
        }
    }
    
    return true;
}

const RTT::ConnPolicy& TransformerHelper::getConnectionPolicy()
{
    return conPolicy;
}

void TransformerHelper::setConnectionPolicy(RTT::ConnPolicy& policy)
{
    conPolicy = policy;
}

