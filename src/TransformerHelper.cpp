#include "TransformerHelper.hpp"
#include <rtt/OperationCaller.hpp>
#include <base/samples/RigidBodyState.hpp>
#include <smurf/Smurf.hpp>

#include <transformer/Transformer.hpp>
#include <transformer/BroadcastTypes.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>

using namespace orocos_cpp;

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

TransformerHelper::TransformerHelper(const smurf::Robot& robotConfiguration): conPolicy(RTT::ConnPolicy::buffer(DEFAULT_CONNECTION_BUFFER_SIZE)), robotConfiguration(robotConfiguration)
{
}


bool TransformerHelper::configureTransformer(RTT::TaskContext* task)
{
    const std::string opName("getNeededTransformations");
    
    //test if the task acutally uses the transformer
    if(!task->provides()->hasMember(opName))
        //does not, we take this as successfully configured
        return true;
    
    RTT::OperationInterfacePart *op = task->getOperation(opName);
    RTT::OperationCaller< ::std::vector< transformer::TransformationDescription >() >  caller(op);

    ::std::vector< transformer::TransformationDescription > neededTransforms = caller();
    
    transformer::TransformationTree tree;
    
    ::std::vector< ::base::samples::RigidBodyState > staticTransforms;

    for(const auto tr : robotConfiguration.getStaticTransforms())
    {
        base::samples::RigidBodyState transform;
        transform.sourceFrame = tr->getSourceFrame().getName();
        transform.targetFrame = tr->getTargetFrame().getName();
        transform.setTransform(tr->getTransformation());
        tree.addTransformation(new transformer::StaticTransformationElement(tr->getSourceFrame().getName(), tr->getTargetFrame().getName(), transform));
        staticTransforms.push_back(transform);
    }
    
    for(const auto tr : robotConfiguration.getDynamicTransforms())
    {
        tree.addTransformation(new TransformationProvider(tr->getSourceFrame().getName(), tr->getTargetFrame().getName(), tr->getProviderName(), tr->getProviderPortName()));
    }

    RTT::base::PortInterface *dynamicTransformsPort = task->getPort("dynamic_transformations");
    if(!dynamicTransformsPort)
    {
        std::cout << "Error, given task " << task->getName() << " has not input port 'dynamic_transformations' " << std::endl;
        throw std::runtime_error("Error, given task " + task->getName() + " has not input port 'dynamic_transformations' ");
        return false;
    }
    
    for( const transformer::TransformationDescription &rbs : neededTransforms)
    {
        std::vector<transformer::TransformationElement *> result;
        if(!tree.getTransformationChain(rbs.sourceFrame, rbs.targetFrame, result))
        {
            std::cout << "Error, there is no known transformation from " << rbs.sourceFrame << " to " << rbs.targetFrame << " which is needed by the component " << task->getName() << std::endl;
            throw std::runtime_error("Error, there is no known transformation from " + rbs.sourceFrame + " to " + rbs.targetFrame + " which is needed by the component " + task->getName());
            return false;
        }

        for(const transformer::TransformationElement *elem: result)
        {
            const transformer::InverseTransformationElement *inv = dynamic_cast<const transformer::InverseTransformationElement *>(elem);
            if(inv)
            {
                elem = inv->getElement();
            }
            
            std::cout << "   " << elem->getSourceFrame() << "2" << elem->getTargetFrame() << std::endl;
            const TransformationProvider *prov = dynamic_cast<const TransformationProvider *>(elem);
            if(!prov)
            {
                continue;
            }
            
            //get task context and connect them
            RTT::corba::TaskContextProxy *proxy = NULL;
            try {
                proxy = RTT::corba::TaskContextProxy::Create(prov->providerName, false);
            } catch (...) {
                //if below handles the error, nothing to do here
            }
            
            if(!proxy)
            {
                std::cout << "Error, could not connect to transformation provider '" << prov->providerName << "'" << std::endl;
                throw std::runtime_error("Error, could not connect to transformation provider '" + prov->providerName + "'");
                return false;
            }
            
            RTT::base::PortInterface *port = proxy->getPort(prov->portName);
            if(!port)
            {
                std::cout << "Error, task " << prov->providerName << " has not port named '" << prov->portName << "'"<< std::endl;
                throw std::runtime_error("Error, task " + prov->providerName + " has not port named '" + prov->portName + "'");
                return false;
            }
            if(!port->connectTo(dynamicTransformsPort, conPolicy))
            {
                throw std::runtime_error("Error, could not connect " + prov->providerName + "." + prov->portName + " to " + task->getName() + "." + dynamicTransformsPort->getName() );
            }
        }
    }
    
    //write static stransformations
    RTT::base::PropertyBase *pStaticTransformationsProperty = task->getProperty("static_transformations");
    if(!pStaticTransformationsProperty)
    {
        throw std::runtime_error("Error, could not get Property 'static_transformations' for transformer task " + task->getName());
    }
    
    RTT::Property< ::std::vector< ::base::samples::RigidBodyState > > *staticTransformationsProperty = dynamic_cast<RTT::Property< ::std::vector< ::base::samples::RigidBodyState > > *>(pStaticTransformationsProperty);
    if(!staticTransformationsProperty)
    {
        throw std::runtime_error("Error, property 'static_transformations' of task " + task->getName() + " has wrong type (not  RTT::Property< ::std::vector< ::base::samples::RigidBodyState > >)");
    }

    staticTransformationsProperty->set(staticTransforms);
    
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

