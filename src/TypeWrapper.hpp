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

    std::string getTypelibType() {
        return value.getType().getName();
    }

    template<class TYPE> TYPE* getAs(const std::string& type_name) {
        if (value.getType().getName() == type_name) {
            return reinterpret_cast<TYPE*>(value.getData());
        }
        return nullptr;
    }

    std::string toString();

    // const Typelib::Type* getType() {
    //     return type;
    // }

 protected:
    void printType(std::string ident);

 private:

    // void add(Typelib::Value& value);
    std::string numericToString();

    void addCompound();

    void addContainer();

    Typelib::Type::Category category;
    Typelib::Value value;

};
}  // namespace orocos_cpp
