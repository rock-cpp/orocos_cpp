
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
        default:
            std::cout << "category "<<  value.getType().getCategory() << " of type " << value.getType().getName() << std::endl;
            break;
    }

}

// void TypeWrapper::add(Typelib::Value& value) {

//     // std::cout << "add is a " << value.getType().getName() << std::endl;


// }


void TypeWrapper::addCompound() {
    category = Typelib::Type::Category::Compound;
    // iterate fields
    const Typelib::Compound& compound = static_cast<const Typelib::Compound&>(value.getType());


    uint8_t *data = static_cast<uint8_t *>(value.getData());
    for (const auto& field : compound.getFields()) {
        // Typelib::Value newvalue = Typelib::value_get_field(value, field.getName());

        Typelib::Value newvalue(data + field.getOffset(), field.getType());

        std::cout << "compound " << newvalue.getType().getName() << std::endl;

        (*this)[field.getName()] = std::make_shared<TypeWrapper>(newvalue);

    }
}

void TypeWrapper::addContainer() {
    category = Typelib::Type::Category::Container;
    const Typelib::Container& container = dynamic_cast<const Typelib::Container&>(value.getType());

    if (container.isRandomAccess()) {
        printf("%s:%i\n", __PRETTY_FUNCTION__, __LINE__);
        for (int i = 0; i < container.getElementCount(value.getData()); ++i) {
            Typelib::Value newvalue = container.getElement(value.getData(), i);
            (*this)[std::to_string(i)] = std::make_shared<TypeWrapper>(newvalue);
        }
    } else {
        // should be a /std/string
        //printf("%s:%i\n", __PRETTY_FUNCTION__, __LINE__);
    }
}

void TypeWrapper::printType(std::string ident) {
    std::cout << ident << this->value.getType().getName() << std::endl;
    ident = ident + "    ";
    for (auto& entry : *this) {
        std::cout << ident << entry.first << "\t";
        entry.second->printType(ident);
    }
}

}  // namespace orocos_cpp
