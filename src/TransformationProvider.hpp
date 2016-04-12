#include <transformer/Transformer.hpp>

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
