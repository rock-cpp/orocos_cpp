
#include "TypeWrapper.hpp"
#include <iostream>


// /home/planthaber/dfki/workspace/terrain_exploration/workspace/gui/rock-display/src/configuration.cpp

namespace orocos_cpp {

TypeWrapper::TypeWrapper(Typelib::Value& value):value(value) {
    switch (value.getType().getCategory()) {
        case Typelib::Type::Category::Compound:
            addCompound();
            break;
        case Typelib::Type::Category::Container:
            addContainer();
            break;
        case Typelib::Type::Category::Array:
            addArray();
            break;
        case Typelib::Type::Category::Numeric:
            category = Typelib::Type::Category::Numeric;
            break;
        default:
            std::cout << "category "<<  value.getType().getCategory() << " of type " << value.getType().getName() << std::endl;
            break;
    }
}

void TypeWrapper::addCompound() {
    category = Typelib::Type::Category::Compound;
    // iterate fields
    const Typelib::Compound& compound = static_cast<const Typelib::Compound&>(value.getType());

    uint8_t *data = static_cast<uint8_t *>(value.getData());
    for (const auto& field : compound.getFields()) {
        Typelib::Value newvalue(data + field.getOffset(), field.getType());
        // as the [] operator is overloaded, we need to call directly
        std::map<std::string, std::shared_ptr<TypeWrapper>>::operator[](field.getName()) = std::make_shared<TypeWrapper>(newvalue);
    }
}

void TypeWrapper::addContainer() {
    category = Typelib::Type::Category::Container;
    const Typelib::Container& container = dynamic_cast<const Typelib::Container&>(value.getType());

    if (container.isRandomAccess()) {
        for (size_t i = 0; i < container.getElementCount(value.getData()); ++i) {
            Typelib::Value newvalue = container.getElement(value.getData(), i);
            // as the [] operator is overloaded, we need to call directly
            std::map<std::string, std::shared_ptr<TypeWrapper>>::operator[](std::to_string(i)) = std::make_shared<TypeWrapper>(newvalue);
        }
    } else {
        // should be a /std/string
        //printf("%s:%i\n", __PRETTY_FUNCTION__, __LINE__);
    }
}

void TypeWrapper::addArray() {
    category = Typelib::Type::Category::Array;
    const Typelib::Array& array = dynamic_cast<const Typelib::Array&>(value.getType());
    const Typelib::Type &indirect(array.getIndirection());

    for (size_t i = 0; i < array.getDimension(); ++i) {
        Typelib::Value newvalue(static_cast<uint8_t *>(value.getData()) + i * indirect.getSize(), indirect);
        // as the [] operator is overloaded, we need to call directly
        std::map<std::string, std::shared_ptr<TypeWrapper>>::operator[](std::to_string(i)) = std::make_shared<TypeWrapper>(newvalue);
    }
}

std::string TypeWrapper::toString() {
    switch (value.getType().getCategory()) {
        //case Typelib::Type::Category::Compound: return compoundToString();
        case Typelib::Type::Category::Numeric: return numericToString();
        case Typelib::Type::Category::Container: return containerToString();
        default: return "";
    }
}

double TypeWrapper::toDouble() {
    if (value.getType().getCategory() != Typelib::Type::Category::Numeric) {
        throw std::runtime_error("Type is not numeric");
    }
    if (value.getType().getCategory() == Typelib::Type::Category::Numeric) {
        if (value.getType().getName() == "/bool") {
            return (static_cast<bool>(value.getData()));
        }
        const Typelib::Numeric& numeric = dynamic_cast<const Typelib::Numeric&>(value.getType());
        switch (numeric.getNumericCategory()) {
            case Typelib::Numeric::Float:
                if (numeric.getSize() == sizeof(float)) {
                    return *static_cast<float*>(value.getData());
                } else {
                    return *static_cast<double*>(value.getData());
                }
            case Typelib::Numeric::SInt:
                switch (numeric.getSize()) {
                    case sizeof(int8_t):  return static_cast<int>(*static_cast<int8_t *>(value.getData()));
                    case sizeof(int16_t): return *static_cast<int16_t*>(value.getData());
                    case sizeof(int32_t): return *static_cast<int32_t*>(value.getData());
                    case sizeof(int64_t): return *static_cast<int64_t*>(value.getData());
                    default:
                        std::cerr << "Error, got integer of unexpected size " << numeric.getSize() << std::endl;
                        throw std::runtime_error("got integer of unexpected size");
                }
            case Typelib::Numeric::UInt: {
                switch (numeric.getSize()) {
                    case sizeof(uint8_t):  return static_cast<unsigned int>(*static_cast<uint8_t *>(value.getData()));
                    case sizeof(uint16_t): return *static_cast<uint16_t*>(value.getData());
                    case sizeof(uint32_t): return *static_cast<uint32_t*>(value.getData());
                    case sizeof(uint64_t): return *static_cast<uint64_t*>(value.getData());
                    default:
                        std::cout << "Error, got integer of unexpected size " << numeric.getSize() << std::endl;
                        throw std::runtime_error("got integer of unexpected size");
                }
            }
            case Typelib::Numeric::NumberOfValidCategories:
            default: throw std::runtime_error("Internal Error: Got invalid Category");
        }
    }
    return 0;

}

std::string TypeWrapper::numericToString() {
    if (value.getType().getCategory() == Typelib::Type::Category::Numeric) {
        if (value.getType().getName() == "/bool") {
            return (*static_cast<bool *>(value.getData())?"true":"false");
        }
        return std::to_string(toDouble());
    }
    return "";
}

std::string TypeWrapper::containerToString() {
    const Typelib::Container& container = dynamic_cast<const Typelib::Container&>(value.getType());

    if (container.getName() == "/std/string") {
        return *static_cast<const std::string *>(value.getData());
    }
    // this is a real container, will be represented by other typewrappers
    return "";
}


void TypeWrapper::printType(std::string ident) {
    std::cout << ident << this->value.getType().getName()  << ": \t" << this->toString() << std::endl;
    ident = ident + "    ";
    for (auto& entry : *this) {
        std::cout << ident << entry.first << "\t";
        entry.second->printType(ident);
    }
}

}  // namespace orocos_cpp
