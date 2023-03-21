#pragma once

#include <typelib/typemodel.hh>
#include <typelib/value.hh>
#include <map>
#include <string>
#include <memory>

namespace orocos_cpp {

class TypeWrapper : public std::map<std::string, std::shared_ptr<TypeWrapper> > {
 public:
    TypeWrapper(Typelib::Value& value);

    void printType() {
        printType("");
    }

    const Typelib::Value& getTypelibValue() {
        return value;
    }

    template<class TYPE> TYPE* getAs() {
        return reinterpret_cast<TYPE*>(value.getData());
    }

    // const Typelib::Type* getType() {
    //     return type;
    // }

 protected:
    void printType(std::string ident);

 private:

    // void add(Typelib::Value& value);

    void addCompound();

    void addContainer();

    Typelib::Type::Category category;
    Typelib::Value value;

};
}  // namespace orocos_cpp
