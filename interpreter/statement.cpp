#include "statement.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace ast {

using runtime::Closure;
using runtime::Context;
using runtime::ObjectHolder;

namespace {
const string ADD_METHOD = "__add__"s;
const string INIT_METHOD = "__init__"s;
}  // namespace

ObjectHolder Assignment::Execute(Closure& closure, Context& context) {
    closure[name_] = value_.get()->Execute(closure, context);
    return closure[name_];
}

Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv):name_(var), value_(move(rv)) {
}

VariableValue::VariableValue(const std::string& var_name) {
    nameVar_ = var_name;
}

VariableValue::VariableValue(std::vector<std::string> dotted_ids) {
    bool first = true;
    // нужно хранить целый вектор и как-то различать использование. возможно по size
    for (auto & value :dotted_ids)
    {
        if (!first)
            nameVar_ += ".";
        first = false;
        nameVar_ += value;
    }
}

ObjectHolder VariableValue::Execute(Closure& closure, Context& /*context*/) {

    auto f = closure.find(nameVar_);
    if (f != closure.end())
    {

        return f->second;
    }

    throw std::runtime_error("Varaible "s + nameVar_ + " is not defined");
}

unique_ptr<Print> Print::Variable(const std::string& /*name*/) {
    // Заглушка, реализуйте метод самостоятельно
    throw std::logic_error("Not implemented"s);
}

Print::Print(unique_ptr<Statement> /*argument*/) {
    // Заглушка, реализуйте метод самостоятельно
}

Print::Print(vector<unique_ptr<Statement>> /*args*/) {
    // Заглушка, реализуйте метод самостоятельно
}

ObjectHolder Print::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

MethodCall::MethodCall(std::unique_ptr<Statement> /*object*/, std::string /*method*/,
                       std::vector<std::unique_ptr<Statement>> /*args*/) {
    // Заглушка. Реализуйте метод самостоятельно
}

ObjectHolder MethodCall::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Stringify::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Add::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Sub::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Mult::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Div::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Compound::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Return::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ClassDefinition::ClassDefinition(ObjectHolder /*cls*/) {
    // Заглушка. Реализуйте метод самостоятельно
}

ObjectHolder ClassDefinition::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

FieldAssignment::FieldAssignment(VariableValue object, std::string field_name,
                                 std::unique_ptr<Statement> rv):object_(object), field_name_(field_name), rv_(std::move(rv)) {
}

ObjectHolder FieldAssignment::Execute(Closure& closure, Context& context) {
    auto f = closure.find(object_.nameVar_);
    // нет учета вложенных полей
    if (f != closure.end())
    {
        auto cla = f->second.TryAs<runtime::ClassInstance>();
        cla->Fields()[field_name_] = rv_->Execute(closure, context);
        return cla->Fields()[field_name_];
    }
     throw std::runtime_error("Class hasn't self"s);
}

IfElse::IfElse(std::unique_ptr<Statement> /*condition*/, std::unique_ptr<Statement> /*if_body*/,
               std::unique_ptr<Statement> /*else_body*/) {
    // Реализуйте метод самостоятельно
}

ObjectHolder IfElse::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Or::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder And::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Not::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

Comparison::Comparison(Comparator /*cmp*/, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs)
    : BinaryOperation(std::move(lhs), std::move(rhs)) {
    // Реализуйте метод самостоятельно
}

ObjectHolder Comparison::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

NewInstance::NewInstance(const runtime::Class& /*class_*/, std::vector<std::unique_ptr<Statement>> /*args*/){
    // Заглушка. Реализуйте метод самостоятельно
}

NewInstance::NewInstance(const runtime::Class& /*class_*/) {
    // Заглушка. Реализуйте метод самостоятельно
}

ObjectHolder NewInstance::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

MethodBody::MethodBody(std::unique_ptr<Statement>&& /*body*/) {
}

ObjectHolder MethodBody::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

}  // namespace ast
