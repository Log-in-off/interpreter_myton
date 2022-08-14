#include "runtime.h"

#include <cassert>
#include <optional>
#include <sstream>
#include <algorithm>

using namespace std;

namespace runtime {

ObjectHolder::ObjectHolder(std::shared_ptr<Object> data)
    : data_(std::move(data)) {
}

void ObjectHolder::AssertIsValid() const {
    assert(data_ != nullptr);
}

ObjectHolder ObjectHolder::Share(Object& object) {
    // Возвращаем невладеющий shared_ptr (его deleter ничего не делает)
    return ObjectHolder(std::shared_ptr<Object>(&object, [](auto* /*p*/) { /* do nothing */ }));
}

ObjectHolder ObjectHolder::None() {
    return ObjectHolder();
}

Object& ObjectHolder::operator*() const {
    AssertIsValid();
    return *Get();
}

Object* ObjectHolder::operator->() const {
    AssertIsValid();
    return Get();
}

Object* ObjectHolder::Get() const {
    return data_.get();
}

ObjectHolder::operator bool() const {
    return Get() != nullptr;
}

bool IsTrue(const ObjectHolder& object) {
    if (object)
    {
        String * str = object.TryAs<String>();
        if (str)
        {
            return !str->GetValue().empty();
        }

        Number * num = object.TryAs<Number>();
        if (num)
        {
            return num->GetValue() != 0;
        }

        Bool * bo = object.TryAs<Bool>();
        if (bo)
        {
            return bo->GetValue();
        }



        Class *cl = object.TryAs<Class>();
        if (cl)
            return false;

        ClassInstance *clIn = object.TryAs<ClassInstance>();
        if (clIn)
            return false;
    }
    return false;
}

void ClassInstance::Print(std::ostream& os, [[maybe_unused]] Context& context) {
    auto FunStr = cls_.GetMethod("__str__");
    if (FunStr)
    {
        auto res = Call(FunStr->name, {}, context);
        res->Print(os, context);
        //os << res.TryAs<String>()->GetValue();
    }
    else
        os << static_cast<void *> (this);
}

bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
    const Method * ptrMethod = cls_.GetMethod(method);
    if (ptrMethod)
    {
        if (ptrMethod->formal_params.size() == argument_count)
            return true;
    }
    return false;
}

Closure& ClassInstance::Fields() {
    return closures_;
}

const Closure& ClassInstance::Fields() const {
    return closures_;
}

ClassInstance::ClassInstance(const Class& cls):cls_(cls) {
    closures_["self"] = ObjectHolder::Share(*this);
}

ObjectHolder ClassInstance::Call(const std::string& method,
                                 const std::vector<ObjectHolder>& actual_args,
                                 Context& context) {
    if (HasMethod(method, actual_args.size()))
    {
        const Method * ptrMethod = cls_.GetMethod(method);
        Closure closures;
        //closures["self"] = closures_["self"];
        closures["self"] = ObjectHolder::Share(*this);
        for(size_t i = 0; i < actual_args.size(); i++)
            closures[ptrMethod->formal_params[i]] = actual_args[i];

        return ptrMethod->body->Execute(closures, context);
    }

    throw std::runtime_error("Class and his parents havn't method"s + method);
}

Class::Class(std::string name, std::vector<Method> methods, const Class* parent): Object(), name_(name), methods_(std::move(methods)), parent_(parent) {
}

const Method* Class::GetMethod(const std::string& name) const {
    auto f = std::find_if(methods_.begin(), methods_.end(),[name](const Method& value) { return value.name == name; });
    if (f != methods_.end())
        return f.base();

    if (parent_)
    {
        return parent_->GetMethod(name);
    }

    return nullptr;
}

[[nodiscard]] const std::string& Class::GetName() const {
    return name_;
}

void Class::Print(ostream& os, [[maybe_unused]] Context& context) {
    os << "Class " << GetName();
}

void Bool::Print(std::ostream& os, [[maybe_unused]] Context& context) {
    os << (GetValue() ? "True"sv : "False"sv);
}

bool Equal(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    String *strLhs = lhs.TryAs<String>();
    String *strRhs = rhs.TryAs<String>();
    if ( strLhs && strRhs)
        return strLhs->GetValue() == strRhs->GetValue();

    Number *numLhs = lhs.TryAs<Number>();
    Number *numRhs = rhs.TryAs<Number>();
    if (numLhs && numRhs)
        return numLhs->GetValue() == numRhs->GetValue();

    Bool *boolLhs = lhs.TryAs<Bool>();
    Bool *boolRhs = rhs.TryAs<Bool>();
    if (boolLhs && boolRhs)
        return boolLhs->GetValue() == boolRhs->GetValue();

    ClassInstance * clInsLhs = lhs.TryAs<ClassInstance>();
    if (clInsLhs)
    {
        auto res = clInsLhs->Call("__eq__", {rhs}, context);
        return res.TryAs<Bool>()->GetValue();
    }

    if (!lhs && !rhs)
        return true;

    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool Less(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    String *strLhs = lhs.TryAs<String>();
    String *strRhs = rhs.TryAs<String>();
    if ( strLhs && strRhs)
        return strLhs->GetValue() < strRhs->GetValue();

    Number *numLhs = lhs.TryAs<Number>();
    Number *numRhs = rhs.TryAs<Number>();
    if (numLhs && numRhs)
        return numLhs->GetValue() < numRhs->GetValue();

    Bool *boolLhs = lhs.TryAs<Bool>();
    Bool *boolRhs = rhs.TryAs<Bool>();
    if (boolLhs && boolRhs)
        return boolLhs->GetValue() < boolRhs->GetValue();

    ClassInstance * clInsLhs = lhs.TryAs<ClassInstance>();
    if (clInsLhs)
    {
        auto res = clInsLhs->Call("__lt__", {rhs}, context);
        return res.TryAs<Bool>()->GetValue();
    }

    throw std::runtime_error("Cannot compare objects for less"s);
}

bool NotEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    return !Equal(lhs, rhs, context);
}

bool Greater(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    return (!Less(lhs, rhs, context)) && NotEqual(lhs, rhs, context);
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool LessOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    return !Greater(lhs, rhs, context);
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool GreaterOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    return !Less(lhs, rhs, context);
    throw std::runtime_error("Cannot compare objects for equality"s);
}

}  // namespace runtime
