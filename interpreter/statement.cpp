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
    nameVars_.push_back(var_name);
}

VariableValue::VariableValue(std::vector<std::string> dotted_ids):nameVars_(dotted_ids.begin(), dotted_ids.end()) {
}

ObjectHolder VariableValue::Execute(Closure& closure, Context& /*context*/) {

    if (1 == nameVars_.size())
    {
        auto f = closure.find(nameVars_.front());
        if (f != closure.end())
        {
            return f->second;
        }
    }

    throw std::runtime_error("Varaible "s + nameVars_.front() + " is not defined");
}

unique_ptr<Print> Print::Variable(const std::string& name) {
    return std::make_unique<Print>(make_unique<StringConst>(name));
}

Print::Print(unique_ptr<Statement> argument) {
    args_.push_back(std::move(argument));
}

Print::Print(vector<unique_ptr<Statement>> args) {
    for(auto &var: args)
        args_.push_back(std::move(var));
}

ObjectHolder Print::Execute(Closure& closure, Context& context) {
    int i = args_.size();
    std::stringstream tmp;

    for(const auto &arg: args_)
    {
        auto obj = arg.get()->Execute(closure, context);
        if(!obj)
        {
            //context.GetOutputStream() << "None";
            tmp << "None";
        }
        else
        {
            auto st = obj.TryAs<runtime::String>();
            if (st)
            {
                auto f = closure.find(st->GetValue());
                if (f != closure.end())
                    //f->second.Get()->Print(context.GetOutputStream(), context);
                    f->second.Get()->Print(tmp, context);
                else
                    //st->Print(context.GetOutputStream(), context);
                    st->Print(tmp, context);
            }
            else
                //obj->Print(context.GetOutputStream(), context);
                obj->Print(tmp, context);
        }

        if (i-- > 1)
            //context.GetOutputStream() << ' ';
            tmp << ' ';
    }
    context.GetOutputStream() << tmp.str();
    context.GetOutputStream() << '\n';
    //std::string var = tmp.str();
    //return ObjectHolder().Own(runtime::String(var));
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

ObjectHolder Stringify::Execute(Closure& closure, Context& context) {
    //return Print(std::move(argument_)).Execute(closure, context);
    std::stringstream tmp;
    auto obj = argument_.get()->Execute(closure, context);
    if(!obj)
    {
        tmp << "None";
    }
    else
    {
        auto st = obj.TryAs<runtime::String>();
        if (st)
        {
            auto f = closure.find(st->GetValue());
            if (f != closure.end())
                f->second.Get()->Print(tmp, context);
            else
                st->Print(tmp, context);
        }
        else
            obj->Print(tmp, context);
    }
    return ObjectHolder().Own(runtime::String(tmp.str()));
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

    runtime::ClassInstance *cl = nullptr;
    Closure* cur_closure = &closure;
    for(std::string &name:object_.nameVars_)
    {
        auto f = cur_closure->find(name);
        if (f != cur_closure->end())
        {
            cl = f->second.TryAs<runtime::ClassInstance>();
            cur_closure = &cl->Fields();
        }
        else
        {
            cl = nullptr;
            break;
        }
    }

    if (cl)
    {
        cl->Fields()[field_name_] = rv_->Execute(closure, context);
        return cl->Fields()[field_name_];
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

NewInstance::NewInstance(const runtime::Class& class_, std::vector<std::unique_ptr<Statement>> /*args*/):clas_(class_){
    // Заглушка. Реализуйте метод самостоятельно
}

NewInstance::NewInstance(const runtime::Class& class_):clas_(class_) {
    // Заглушка. Реализуйте метод самостоятельно
}

ObjectHolder NewInstance::Execute(Closure& /*closure*/, Context& /*context*/) {
    ObjectHolder tmp = ObjectHolder().Own(runtime::ClassInstance(clas_));
    // нужно выполнить поле init при наличии
    return tmp;
}

MethodBody::MethodBody(std::unique_ptr<Statement>&& /*body*/) {
}

ObjectHolder MethodBody::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

}  // namespace ast
