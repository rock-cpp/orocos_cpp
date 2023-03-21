#pragma once

#include <typelib/typemodel.hh>
#include <typelib/value.hh>
#include <map>
#include <string>
#include <memory>

namespace orocos_cpp {


/**
 * @brief This wrapper gived convinoent access to the data of a typelib type without linking the actual type
 * 
 * The constructor creates a map so that a call on e.g. /base/samples/Joints data linke thsi is possible
 * Typelib::Value joints;
 * TypeWrapper jointwrapper(joints);
 * TypeWrapper element = jointwrapper["elements"][0];
 * std::cout << element.toString() << std::endl;
 * 
 * 
 */
class TypeWrapper : public std::map<std::string, std::shared_ptr<TypeWrapper> > {
 public:
    TypeWrapper(Typelib::Value& value);

    void printType() {
        printType("");
    }

    TypeWrapper& operator[](const std::string& key) {
        return *(std::map<std::string, std::shared_ptr<TypeWrapper>>::operator[](key));
    }

    const Typelib::Value& getTypelibValue() {
        return value;
    }

    std::string getTypelibTypeName() {
        return value.getType().getName();
    }

    template<class TYPE> TYPE* getAs(const std::string& type_name) {
        if (value.getType().getName() == type_name) {
            return reinterpret_cast<TYPE*>(value.getData());
        }
        return nullptr;
    }

    std::string toString();

    double toValue();

 protected:
    void printType(std::string ident);

 private:

    // void add(Typelib::Value& value);
    std::string numericToString();

    std::string containerToString();

    void addCompound();

    void addContainer();

    void addArray();

    Typelib::Type::Category category;
    Typelib::Value value;

};
}  // namespace orocos_cpp
