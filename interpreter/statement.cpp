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
    else
    {
        Closure* cur_closure = &closure;
        for(auto &value: nameVars_)
        {
            auto f = cur_closure->find(value);
            if(f != cur_closure->end())
            {
                auto cl = f->second.TryAs<runtime::ClassInstance>();
                if (cl)
                {
                    cur_closure = &cl->Fields();
                }
                else
                    return f->second;
            }

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

        if (i-- > 1)
            tmp << ' ';
    }
    context.GetOutputStream() << tmp.str();
    context.GetOutputStream() << '\n';
    //std::string var = tmp.str();
    //return ObjectHolder().Own(runtime::String(var));
    return {};
}

MethodCall::MethodCall(std::unique_ptr<Statement> object, std::string method,
                       std::vector<std::unique_ptr<Statement>> args):object_(std::move(object)), method_(method), args_(std::move(args)){
    ;
}

ObjectHolder MethodCall::Execute(Closure& closure, Context& context) {
    auto obj = object_.get()->Execute(closure, context);
    auto cl = obj.TryAs<runtime::ClassInstance>();
    if (cl)
    {
        if (cl->HasMethod(method_, args_.size()))
        {
            std::vector<ObjectHolder> actual_args;
            for (auto &value:args_)
                actual_args.push_back(value.get()->Execute(closure, context));

            return cl->Call(method_, actual_args, context);
        }
    }
    return {};
}

ObjectHolder Stringify::Execute(Closure& closure, Context& context) {
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

ObjectHolder Add::Execute(Closure& closure, Context& context) {
    auto lhs = lhs_.get()->Execute(closure, context);
    auto rhs = rhs_.get()->Execute(closure, context);
    auto lhsNum = lhs.TryAs<runtime::Number>();
    auto rhsNum = rhs.TryAs<runtime::Number>();
    if(lhsNum && rhsNum)
    {
        return ObjectHolder().Own(runtime::Number(lhsNum->GetValue()+rhsNum->GetValue()));
    }

    auto lhsString = lhs.TryAs<runtime::String>();
    auto rhsString = rhs.TryAs<runtime::String>();
    if(lhsString && rhsString)
    {
        return ObjectHolder().Own(runtime::String(lhsString->GetValue()+rhsString->GetValue()));
    }

    auto lhsClass = lhs.TryAs<runtime::ClassInstance>();
    if (lhsClass && lhsClass->HasMethod(ADD_METHOD, 1))
    {
        return lhsClass->Call(ADD_METHOD, {rhs}, context);
    }

    throw std::runtime_error("The operation add isn't supported"s);
}

ObjectHolder Sub::Execute(Closure& closure, Context& context) {
    auto lhs = lhs_.get()->Execute(closure, context);
    auto rhs = rhs_.get()->Execute(closure, context);
    auto lhsNum = lhs.TryAs<runtime::Number>();
    auto rhsNum = rhs.TryAs<runtime::Number>();
    if(lhsNum && rhsNum)
    {
        return ObjectHolder().Own(runtime::Number(lhsNum->GetValue()-rhsNum->GetValue()));
    }
    throw std::runtime_error("The operation sub isn't supported"s);
}

ObjectHolder Mult::Execute(Closure& closure, Context& context) {
    auto lhs = lhs_.get()->Execute(closure, context);
    auto rhs = rhs_.get()->Execute(closure, context);
    auto lhsNum = lhs.TryAs<runtime::Number>();
    auto rhsNum = rhs.TryAs<runtime::Number>();
    if(lhsNum && rhsNum)
    {
        return ObjectHolder().Own(runtime::Number(lhsNum->GetValue()*rhsNum->GetValue()));
    }
    throw std::runtime_error("The operation mult isn't supported"s);
}

ObjectHolder Div::Execute(Closure& closure, Context& context) {
    auto lhs = lhs_.get()->Execute(closure, context);
    auto rhs = rhs_.get()->Execute(closure, context);
    auto lhsNum = lhs.TryAs<runtime::Number>();
    auto rhsNum = rhs.TryAs<runtime::Number>();
    if(lhsNum && rhsNum)
    {
        return ObjectHolder().Own(runtime::Number(lhsNum->GetValue()/rhsNum->GetValue()));
    }
    throw std::runtime_error("The operation sub isn't supported"s);
}

ObjectHolder Compound::Execute(Closure& closure, Context& context) {
    for(auto & value:comp_)
    {
        value.get()->Execute(closure, context);
    }
    return ObjectHolder().None();
}

ObjectHolder Return::Execute(Closure& closure, Context& context) {
    throw statement_.get()->Execute(closure, context);
    return {};
}

ClassDefinition::ClassDefinition(ObjectHolder cls):cls_(cls) {
    ;
}

ObjectHolder ClassDefinition::Execute(Closure& closure, Context&/* context*/) {
    closure[cls_.TryAs<runtime::Class>()->GetName()] = cls_;
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

IfElse::IfElse(std::unique_ptr<Statement> condition, std::unique_ptr<Statement> if_body,
               std::unique_ptr<Statement> else_body):condition_(std::move(condition)), if_body_(std::move(if_body)),else_body_(std::move(else_body)) {
    ;
}

ObjectHolder IfElse::Execute(Closure& closure, Context& context) {
    auto cond = condition_.get()->Execute(closure, context);
    auto tmpBool = cond.TryAs<runtime::Bool>();
    if (tmpBool)
    {
        if(tmpBool->GetValue())
            return if_body_.get()->Execute(closure,context);
        else if (else_body_.get())
        {
            return else_body_.get()->Execute(closure, context);
        }
    }
    return {};
}

ObjectHolder Or::Execute(Closure& closure, Context& context) {
    auto lhs = lhs_.get()->Execute(closure, context);
    auto lhsBool = lhs.TryAs<runtime::Bool>();
    if (lhsBool && lhsBool->GetValue())
    {
        return ObjectHolder().Own(runtime::Bool(true));
    }

    auto rhs = rhs_.get()->Execute(closure, context);
    auto rhsBool = rhs.TryAs<runtime::Bool>();
    if(rhsBool && rhsBool->GetValue())
    {
         return ObjectHolder().Own(runtime::Bool(true));
    }

    return ObjectHolder().Own(runtime::Bool(false));
}

ObjectHolder And::Execute(Closure& closure, Context& context) {
    auto lhs = lhs_.get()->Execute(closure, context);
    auto lhsBool = lhs.TryAs<runtime::Bool>();
    if (lhsBool && !lhsBool->GetValue())
    {
        return ObjectHolder().Own(runtime::Bool(false));
    }

    auto rhs = rhs_.get()->Execute(closure, context);
    auto rhsBool = rhs.TryAs<runtime::Bool>();
    if(rhsBool && rhsBool->GetValue())
    {
         return ObjectHolder().Own(runtime::Bool(true));
    }

    return ObjectHolder().Own(runtime::Bool(false));
}

ObjectHolder Not::Execute(Closure& closure, Context& context) {
    auto lhs = argument_.get()->Execute(closure, context);
    auto lhsBool = lhs.TryAs<runtime::Bool>();
    if (lhsBool)
    {
        return ObjectHolder().Own(runtime::Bool(!lhsBool->GetValue()));
    }
     throw std::runtime_error("The operation 'not' isn't supported"s);
}

Comparison::Comparison(Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs)
    : BinaryOperation(std::move(lhs), std::move(rhs)), cmp_(cmp) {
    ;
}

ObjectHolder Comparison::Execute(Closure& closure, Context& context) {
    auto lh = lhs_->Execute(closure, context);
    auto rh = rhs_->Execute(closure, context);

    auto res = cmp_(lh,rh, context);
    return ObjectHolder().Own(runtime::Bool(res));
}

NewInstance::NewInstance(const runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args):clas_(class_){
    for(auto &var: args)
        args_.push_back(std::move(var));
}

NewInstance::NewInstance(const runtime::Class& class_):clas_(class_) {
}

ObjectHolder NewInstance::Execute(Closure& closure, Context& context) {
    ObjectHolder tmp = ObjectHolder().Own(runtime::ClassInstance(clas_));
    auto cl = tmp.TryAs<runtime::ClassInstance>();
    if (cl)
    {
        if (cl->HasMethod(INIT_METHOD, args_.size()))
        {
            std::vector<ObjectHolder> tmpArgs;
            for (auto & val:args_)
                tmpArgs.push_back(val.get()->Execute(closure, context));
            cl->Call(INIT_METHOD, tmpArgs, context);
        }
    }
    return tmp;
}

MethodBody::MethodBody(std::unique_ptr<Statement>&& body):body_(std::move(body)) {
    ;
}

ObjectHolder MethodBody::Execute(Closure& closure, Context& context) {
    try
    {
        body_.get()->Execute(closure, context);
    }
    catch(ObjectHolder var)
    {
        return var;
    }

    return ObjectHolder().None();
}

}  // namespace ast
