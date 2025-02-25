#include "analyzer.hpp"

#include <ostream>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>

namespace vire
{
    bool VAnalyzer::isVariableDefined(const proto::IName& name)
    {
        return scope.count(name.get());
    }
    bool VAnalyzer::isStructDefined(const std::string& name)
    {
        return types::isTypeinMap(name);
    }
    bool VAnalyzer::isFunctionDefined(const std::string& name)
    {
        const auto& functions=ast->getFunctions();
        if(!functions.empty())
        {
            for(int i=0; i<functions.size(); ++i)
                if(functions[i]->getIName().name==name)
                    return true;
        }
        
        const auto& constructors=ast->getConstructors();
        if(!constructors.empty())
        {
            for(int i=0; i<constructors.size(); ++i)
                if(constructors[i]->getIName().name==name)
                    return true;
        }
        
        return false;
    }
    bool VAnalyzer::isClassDefined(const std::string& name)
    {
        const auto& classes=ast->getClasses();
        if(!classes.empty())
        {
            for(int i=0; i<classes.size(); ++i)
            {
                if(classes[i]->getName()==name)
                {
                    return true;
                }
            }
        }
        return false;
    }

    void VAnalyzer::defineVariable(VariableDefAST* const var, bool is_arg)
    {
        scope.insert(std::make_pair(var->getName(), var));
        
        if(scope_varref != nullptr)
        {
            scope_varref->push_back(var);
        }

        if(is_arg) return;
        if(current_func != nullptr)
        {
            current_func->addVariable(var);
        }
    }
    void VAnalyzer::undefineVariable(VariableDefAST* const var)
    {
        undefineVariable(var->getIName());
    }
    void VAnalyzer::undefineVariable(proto::IName const& name)
    {
        scope.erase(name.get());
    }

    void VAnalyzer::addConstructor(FunctionAST* func)
    {
        ast->addConstructor(func);
    }
    void VAnalyzer::addFunction(std::unique_ptr<FunctionBaseAST> func)
    {
        ast->addFunction(std::move(func));
    }

    ModuleAST* const VAnalyzer::getSourceModule()
    {
        return ast.get();
    }

    FunctionBaseAST* const VAnalyzer::getFunction(const std::string& name)
    {
        return getFunction(proto::IName(name, ""));
    }
    VariableDefAST* const VAnalyzer::getVariable(const std::string& name)
    {
        return getVariable(proto::IName(name, ""));
    }
    StructExprAST* const VAnalyzer::getStruct(const std::string& name)
    {
        return getStruct(proto::IName(name, ""));
    }
    FunctionBaseAST* const VAnalyzer::getFunction(const proto::IName& name)
    {
        const auto& functions=ast->getFunctions();
        for(int i=0; i<functions.size(); i++)
            if(functions[i]->getIName().name==name.name)
                return functions[i].get();

        const auto& constructors=ast->getConstructors();
        for(int i=0; i<constructors.size(); i++)
            if(constructors[i]->getIName().name==name.name)
                return constructors[i];

        if(current_func)
            if(current_func->getIName().name==name.name || name.name=="")
                return current_func;
        
        std::cout << "Function `" << name << "` not found" << std::endl;
        return nullptr;
    }
    VariableDefAST* const VAnalyzer::getVariable(const proto::IName& name)
    {
        if(!isVariableDefined(name))
        {
           std::cout << "Variable `" << name << "` not found" << std::endl;
           return nullptr;
        }
        return scope.at(name.get());
    }
    StructExprAST* const VAnalyzer::getStruct(const proto::IName& name)
    {
        if(!isStructDefined(name.get()))
        {
            std::cout << "Struct with name not defined: " << name << std::endl;
            return nullptr;
        }

        if(current_struct->getIName().name==name)
            return current_struct;

        auto const& structs=ast->getUnionStructs();
        for(auto const& expr : structs)
        {
            if(expr->asttype==ast_struct)
            {
                auto* st=(StructExprAST*)expr.get();

                if(st->getIName().name==name)
                {
                    return st;
                }
            }
        }

        std::cout << "Could not find struct with name: " << name << std::endl;

        return nullptr;
    }
    
    // !!- CHANGES REQUIRED -!!
    // str to be implemented
    types::Base* VAnalyzer::getType(ExprAST* const expr)
    {
        switch (expr->asttype)
        {
            case ast_int: return expr->getType();
            case ast_float: return expr->getType();
            case ast_double: return expr->getType();
            case ast_char: return expr->getType();
            case ast_bool: return expr->getType();

            case ast_binop:
            {
                auto* binop=((BinaryExprAST*)expr);

                if(binop->getType()->getType()!=types::EType::Void)
                    return binop->getType();

                auto* t=binop->getOpType();

                if(t==nullptr)
                {
                    auto* lhs=getType(binop->getLHS());
                    auto* rhs=getType(binop->getRHS());

                    if(binop->getOp()->type==tok_div)
                    {
                        bool lhs_double=lhs->getType()==types::EType::Double;
                        bool rhs_double=rhs->getType()==types::EType::Double;

                        if(lhs_double || rhs_double)
                        {
                            binop->setType(types::construct(types::EType::Double));
                            return binop->getType();
                        }
                        else
                        {
                            binop->setType(types::construct(types::EType::Float));
                            return binop->getType();
                        }
                    }
                    else
                    {
                        return (lhs->precedence>rhs->precedence)?lhs:rhs;
                    }
                }

                return t;
            }

            case ast_incrdecr: 
            {
                auto* incrdecr=(IncrementDecrementAST*)expr;
                return getType(incrdecr->getExpr());
            }
            case ast_var:
            {
                auto* var=getVariable(((VariableExprAST*)expr)->getIName());
                return var->getType();
            }
            case ast_array_access:
            {
                auto* expr_cast=(VariableArrayAccessAST*)expr;
                auto* array_type=(types::Array*)getType(expr_cast->getExpr());

                // Loop over the indices and get the type of each index
                for(int i=0; i<expr_cast->getIndices().size(); ++i)
                {
                    array_type=(types::Array*)array_type->getChild();
                }

                return array_type;
            }

            case ast_call: return getFunction(((CallExprAST*)expr)->getIName().name)->getReturnType();

            case ast_array: return getType((ArrayExprAST*)expr);

            case ast_type_access:
            {
                auto* access=(TypeAccessAST*)expr;
                
                auto* st_type=getType(access->getParent());
                auto name=((types::Void*)st_type)->getName();
                auto* st=getStruct(name);

                while(access->getChild()->asttype==ast_type_access)
                {
                    name=access->getName();
                    access=(TypeAccessAST*)access->getChild();
                    st=(StructExprAST*)st->getMember(name);
                }

                return st->getMember(access->getChild()->getIName())->getType();
            }

            case ast_cast: return ((CastExprAST*)expr)->getType();

            default:
            {
                std::cout<<"Error: Unknown expr in getType()"<<std::endl;
                return nullptr;
            }
        }
    }
    
    // !!- CHANGES REQUIRED -!!
    // unhandled dynamic array typing
    types::Base* VAnalyzer::getType(ArrayExprAST* const array)
    {
        const auto& vec=array->getElements();
        auto type=types::copyType(getType(vec[0].get()));

        for(int i=0; i<vec.size(); ++i)
        {
            auto* new_type=getType(vec[i].get());

            if(!types::isSame(type.get(), new_type))
            {
                std::cout << "Error: Array element types do not match: " << *type << " " << *new_type << std::endl;
                return nullptr;
            }
        }
        
        unsigned int len=((types::Array*)array->getType())->getLength();

        auto type_uptr=std::make_unique<types::Array>(std::move(type), len);
        array->setType(std::move(type_uptr));

        return array->getType();
    }

    // Helper functions
    ReturnExprAST* const VAnalyzer::getReturnStatement(std::vector<std::unique_ptr<ExprAST>> const& block)
    {
        for(auto const& expr : block)
        {
            if(expr->asttype==ast_return) 
            { return ((std::unique_ptr<ReturnExprAST> const&)expr).get(); }
        }

        return nullptr;
    }
    std::unique_ptr<ExprAST> VAnalyzer::tryCreateImplicitCast(types::Base* target, types::Base* base, std::unique_ptr<ExprAST> expr)
    {
        bool types_are_user_defined=(types::isUserDefined(target) || types::isUserDefined(base));
        bool types_are_arrays=(target->getType()==types::EType::Array || base->getType()==types::EType::Array);
        if(!types_are_user_defined && !types_are_arrays)
        {
            auto src_type=types::copyType(base);
            auto dst_type=types::copyType(target);

            auto new_cast_value=std::make_unique<CastExprAST>(std::move(expr), std::move(dst_type), true);
            new_cast_value->setSourceType(std::move(src_type));

            base=new_cast_value->getSourceType();
            target=new_cast_value->getDestType();
            
            if(base->getSize() > target->getSize())
            {
                std::cout << "Warning: Analysis: Truncation, possible data loss while converting from "
                << *base << " to " << *target << std::endl;
            }
            else if(types::isTypeFloatingPoint(base) && !types::isTypeFloatingPoint(target))
            {
                std::cout << "Warning: Analysis: Decimal (Floating point) to Integer, possible data loss while converting from "
                << *base << " to " << *target << std::endl;
            }

            return std::move(new_cast_value);
        }
        else
        {
            return nullptr;
        }
    }

    // Verification Functions
    bool VAnalyzer::verifyVariable(VariableExprAST* const var)
    {
        // Check if it is defined
        if(!isVariableDefined(var->getIName()))
        {
            // Variable is not defined
            std::cout << "Variable " << var->getName() << " not defined" << std::endl;
            return false;
        }
        
        return true;
    }
    bool VAnalyzer::verifyIncrementDecrement(IncrementDecrementAST* const incrdecr)
    {
        // Check if it is defined
        auto const& expr=incrdecr->getExpr();

        if(!verifyExpr(expr))
        {
            return false;
        }

        if(!types::isNumericType(getType(expr)))
        {
            return false;
        }
        
        return true;
    }
    bool VAnalyzer::verifyVariableDefinition(VariableDefAST* const var, bool add_to_scope)
    {
        if(var->getIName().name=="self")
        {
            std::cout << "Cannot name variable `self` as it is a keyword" << std::endl;
            return false;
        }

        if(!isVariableDefined(var->getName()))
        {
            bool is_var=!(var->isLet() || var->isConst());

            if(!is_var)
            {
                bool type_is_void=false;
                bool type_is_given=true;
                auto* ty=var->getType();

                type_is_void=(ty->getType()==types::EType::Void);
                if(type_is_void)
                {
                    auto* voidty=(types::Void*)ty;
                    if(types::isTypeinMap(voidty->getName()))
                    {
                        type_is_given=true;
                    }
                    else
                    {
                        type_is_given=false;
                    }
                }

                if(var->getValue()==nullptr && !type_is_given)
                {
                    // Requires a variable for definiton
                    unsigned char islet = var->isLet() ? 1 : 0;
                    // builder->addError<errortypes::analyze_requires_type>(this->code, islet, var->getName(), var->getLine(), var->getCharpos());

                    return false;
                }
            }
            auto* type=var->getType();
            types::Base* value_type;

            auto const& value=var->getValue();
            if(value==nullptr)
            {
                var->refreshType();

                if(add_to_scope)
                    defineVariable(var);

                return true;
            }

            bool is_auto=(type->getType()==types::EType::Void);
            if(is_auto && is_var)
            {
                std::cout << "`any` type not implement yet" << std::endl;
            }
            
            if(!verifyExpr(value))
            {
                std::cout << "Variable definition's value is invalid" << std::endl;
                return false;
            }
            
            value_type=getType(value);
            
            if(!is_auto)
            {
                if(!types::isSame(type, value_type))    
                {
                    auto new_cast_value=tryCreateImplicitCast(type, value_type, var->moveValue());
                        
                    if(!new_cast_value)
                    {
                        std::cout << "Error: VarDef Type Mismatch: " << *type << " and " << *value_type << std::endl;
                        return false;
                    }
                    else
                        var->setValue(std::move(new_cast_value));
                }
            }
            else
            {
                auto value_type_uptr=types::copyType(value_type);

                var->getValue()->setType(std::move(value_type_uptr));
                var->setUseValueType(true);
            }
            
            if(add_to_scope)
                defineVariable(var);
            
            return true;
        }
        
        std::cout << "Variable " << var->getName() << " is already defined" << std::endl;

        // Variable is already defined
        return false;
    }
    bool VAnalyzer::verifyVarAssign(VariableAssignAST* const assign)
    {
        bool is_valid=true;
        
        if(!verifyExpr(assign->getLHS()))
        {
            // Var is not defined
            return false;
        }
        if(!verifyExpr(assign->getRHS()))
        {
            return false;
        }

        auto* lhs_type=getType(assign->getLHS());
        auto* rhs_type=getType(assign->getRHS());

        if(!types::isSame(lhs_type, rhs_type))
        {
            auto cast=tryCreateImplicitCast(lhs_type, rhs_type, assign->moveRHS());

            if(!cast)
            {
                std::cout << "Error: Assigment: Variable and Value types do not match" << std::endl;
                return false;
            }
            else
            {
                assign->setRHS(std::move(cast));
            }
        }

        assign->getLHS()->setType(types::copyType(lhs_type));
        assign->getRHS()->setType(types::copyType(rhs_type));
        
        return is_valid;
    }
    bool VAnalyzer::verifyVarArrayAccess(VariableArrayAccessAST* const access)
    {
        if(!verifyExpr(access->getExpr()))
        {
            // Expr is not valid
            return false;
        }
        
        auto const& indices=access->getIndices();
        auto* type=getType(access->getExpr());
        
        if(type->getType()!=types::EType::Array)
        {
            std::cout << "Error: Variable is not an array" << std::endl;
            return false;
        }
        else
        {
            auto* array_type=(types::Array*)type;
            if(indices.size()!=array_type->getDepth())
            {
                std::cout << "Error: Array index mismatch" << std::endl;
                return false;
            }
            else
            {
                auto* child_array_type=array_type;
                for(auto const& index : indices)
                {
                    if(!verifyExpr(index.get()))
                        return false;

                    auto* index_type=getType(index.get());
                    
                    if(index_type->getType() == types::EType::Int)
                    {
                        if(index->asttype==ast_int)
                        {
                            auto* index_cast=(IntExprAST*)index.get();
                            if(index_cast->getValue() >= child_array_type->getLength())
                            {
                                std::cout << "Error: Array index out of bounds" << std::endl;
                                return false;
                            } 
                        }
                    }
                    else
                    {
                        std::cout << "Error: Array index is not of type integer, but is " << *index_type << std::endl;
                        return false;
                    }

                    child_array_type=(types::Array*)child_array_type->getChild();
                }
            }
        }
        
        auto* array_ty=(types::Array*)type;
        access->setType(types::copyType(array_ty->getChild()));
        access->getExpr()->setType(types::copyType(array_ty));

        return true;
    }

    bool VAnalyzer::verifyInt(IntExprAST* const int_) { return true; }
    bool VAnalyzer::verifyFloat(FloatExprAST* const float_) { return true; }
    bool VAnalyzer::verifyDouble(DoubleExprAST* const double_) { return true; }
    bool VAnalyzer::verifyChar(CharExprAST* const char_) { return true; }
    bool VAnalyzer::verifyStr(StrExprAST* const str) { return true; }
    bool VAnalyzer::verifyBool(BoolExprAST* const bool_) { return true; }
    bool VAnalyzer::verifyArray(ArrayExprAST* const array)
    {
        const auto& elems=array->getElements();

        for(const auto& elem : elems)
        {
            if(!verifyExpr(elem.get()))
            {
                // Element is not valid
                return false;
            }
        }
        
        std::size_t elem_size=array->getElements()[0]->getType()->getSize();
        std::size_t size=elem_size*elems.size();
        array->getType()->setSize(size);

        return true;
    }
    bool VAnalyzer::verifyCastExpr(CastExprAST* const cast)
    {
        if(!verifyExpr(cast->getExpr()))
            return false;
        auto* ty=getType(cast->getExpr());
        cast->setSourceType(types::copyType(ty));
        
        return true;
    }

    bool VAnalyzer::verifyFor(ForExprAST* const for_)
    { 
        bool is_valid=true;
        const auto& init=for_->getInit();
        const auto& cond=for_->getCond();
        const auto& incr=for_->getIncr();
        
        if(!(init->asttype==ast_var || init->asttype==ast_varassign || init->asttype==ast_vardef))
        {
            // Init is not a variable definition
            is_valid=false;
        }
        if(!(cond->asttype==ast_var || cond->asttype==ast_unop || cond->asttype==ast_binop))
        {
            // Cond is not a boolean expression
            is_valid=false;
        }
        if(!(incr->asttype==ast_var || incr->asttype==ast_varassign || incr->asttype==ast_incrdecr))
        {
            // Incr is not a step operation
            is_valid=false;
        }   
        
        if(!verifyExpr(init))
        {
            // Init is not valid
            is_valid=false;
        }
        if(!verifyExpr(cond))
        {
            // Cond is not valid
            is_valid=false;
        }
        if(!verifyExpr(incr))
        {
            // Incr is not valid
            is_valid=false;
        }

        if(!verifyBlock(for_->getBody()))
        {
            // Block is not valid
            return false;
        }
        
        
        return is_valid;
    }
    bool VAnalyzer::verifyWhile(WhileExprAST* const while_)
    {
        const auto& cond=while_->getCond();

        if(cond->asttype!=ast_var && cond->asttype!=ast_unop && cond->asttype!=ast_binop)
        {
            // Cond is not a boolean expression
            return false;
        }

        if(!verifyExpr(cond))
        {
            // Cond is not valid
            return false;
        }
        if(!verifyBlock(while_->getBody()))
        {
            // Block is not valid
            return false;
        }

        return true;
    }
    bool VAnalyzer::verifyBreak(BreakExprAST* const break_) { return true; }
    bool VAnalyzer::verifyContinue(ContinueExprAST* const continue_) { return true; }    

    bool VAnalyzer::verifyCall(CallExprAST* const call)
    {
        bool is_valid=true;
        auto name=call->getIName().name;

        bool is_recursive_call=false;
        if(current_func)
        {
            if(current_func->getName()==name)
            {
                is_recursive_call=true;
            }
        }

        if(!isFunctionDefined(name) && !is_recursive_call)
        {
            std::cout << "Function `" << name << "` is not defined" << std::endl;
            // Function is not defined
            return false;
        }

        if(name == "main")
        {
            std::cout << "Verification Error: Cannot call the main function, it is an entry point" << std::endl;
            is_valid=false;
        }

        auto args=call->moveArgs();
        const auto* func=getFunction(name);
        const auto& func_args=func->getArgs();

        if(args.size() != (func_args.size()-func->doesRequireSelfRef()))
        {
            // Argument count mismatch
            std::cout << "Call arg count mismatch" << std::endl;
            is_valid=false;
        }

        for(unsigned int i=func->doesRequireSelfRef(); i<args.size(); ++i)
        {
            auto arg=std::move(args[i]);

            if(!verifyExpr(arg.get()))
            {
                // Argument is not valid
                std::cout << "Call argument is not valid" << std::endl;
                is_valid=false;
                continue;
            }

            // This is to be changed, implemented for arguments with default value
            auto* arg_type=getType(arg.get());
            if(!types::isSame(func_args[i]->getType(), arg_type))
            {
                auto cast=tryCreateImplicitCast(func_args[i]->getType(), arg_type, std::move(arg));

                if(!cast)
                {
                    std::cout << "Error: Function call type mismatch, " << *func_args[i]->getType() << " : " << *arg_type << std::endl;
                    is_valid=false;
                }
                else
                {
                    arg=std::move(cast);
                }
            }
            arg->setType(types::copyType(arg_type));

            args[i]=std::move(arg);
        }
        
        if(is_valid)
        {
            call->setArgs(std::move(args));
        }

        call->setType(types::copyType(func->getReturnType()));

        return is_valid;
    }

    bool VAnalyzer::verifyReturn(ReturnExprAST* const ret)
    {
        auto* func=(FunctionAST*)getFunction(ret->getIName().name);
        auto* ret_type=func->getReturnType();

        if(!verifyExpr(ret->getValue()))
        {
            // Return value is not valid
            return false;
        }

        auto* ret_expr_type=getType(ret->getValue());
        if(!types::isSame(ret_type, ret_expr_type))
        {
            auto cast=tryCreateImplicitCast(ret_expr_type, ret_type, ret->moveValue());

            if(!cast)
            {
                std::cout << "Error: Return type mismatch, " << *ret_expr_type << " : " << *ret_type << std::endl;
                return false;
            }
            else
            {
                ret->setValue(std::move(cast));
            }
        }

        current_func->addReturnStatement(ret);
        if(ret->getValue()->asttype==ast_var)
        {
            auto var_name=((VariableExprAST*)ret->getValue())->getName();
            getVariable(var_name)->isReturned(true);
        }

        ret->getValue()->setType(types::copyType(ret_expr_type));

        return true;
    }

    bool VAnalyzer::verifyBinop(BinaryExprAST* const binop)
    {
        const auto& left=binop->getLHS();
        const auto& right=binop->getRHS();

        bool is_valid=true;

        if(!verifyExpr(left))
        {
            // Left is not valid
            is_valid=false;
        }
        if(!verifyExpr(right))
        {
            // Right is not valid
            is_valid=false;
        }

        if(is_valid)
        {
            auto* left_type=getType(left);
            auto* right_type=getType(right);

            left->setType(types::copyType(left_type));
            right->setType(types::copyType(right_type));

            left_type=left->getType();
            right_type=right->getType();
            
            if(!types::isSame(left_type, right_type))
            {
                bool left_is_fp=types::isTypeFloatingPoint(left_type);
                bool right_is_fp=types::isTypeFloatingPoint(right_type);

                if(left_is_fp xor right_is_fp)
                {
                    if(left_is_fp)
                    {
                        binop->setRHS(tryCreateImplicitCast(left_type, right_type, binop->moveRHS()));
                    }
                    else
                    {
                        binop->setLHS(tryCreateImplicitCast(right_type, left_type, binop->moveLHS()));
                    }
                }
                else if(left_type->getSize() > right_type->getSize())
                {
                    binop->setRHS(tryCreateImplicitCast(left_type, right_type, binop->moveRHS()));
                }
                else
                {
                    binop->setLHS(tryCreateImplicitCast(right_type, left_type, binop->moveLHS()));
                }
            }

            binop->setType(types::copyType(binop->getLHS()->getType()));
        }

        return is_valid;
    }
    bool VAnalyzer::verifyUnop(UnaryExprAST* const unop)
    {
        const auto& expr=unop->getExpr();

        if(!verifyExpr(expr))
        {
            // Expr is not valid
            return false;
        }
        
        return true;
    }

    bool VAnalyzer::verifyPrototype(PrototypeAST* const proto)
    {
        bool is_valid=true;

        if(isFunctionDefined(proto->getIName().name))
        {
            // Function is already defined
            return false;
        }

        if(types::isSame(proto->getReturnType(), "any") /*|| types::isSame(proto->getReturnType(), "auto")*/)
        {
            std::cout << "Function type cannot be `" << *proto->getReturnType() << "`" << std::endl;
            // Type is not valid
            is_valid=false;
        }
        else if(proto->getReturnType()->getType() == types::EType::Void)
        {
            auto* void_ty=(types::Void*)proto->getReturnType();

            if(void_ty->getName() != "")
            {
                if(types::isTypeinMap(void_ty->getName()))
                {
                    proto->setReturnType(types::construct(void_ty->getName(), true));
                }
                else
                {
                    std::cout << "Prototype's return type is a non-defined struct" << std::endl;
                }
            }
        }

        auto it=proto->getArgs().begin();
        if(proto->doesRequireSelfRef()) ++it;
        for(; it!=proto->getArgs().end(); ++it)
        {   
            auto const& arg=*it;
            if(types::isSame(arg->getType(), "any"))
            {
                // Type is not valid
                is_valid=false;
                
                std::cout << "Type cannot be `auto` or `any`" << std::endl;
            }

            if(!verifyVariableDefinition(arg.get(), false))
            {
                std::cout << "Verification Error: Function Prototype argument is not valid" << std::endl;

                // Argument is not valid
                is_valid=false;
            }
            
            arg->isArgument(true);
        }

        return is_valid;
    }
    bool VAnalyzer::verifyProto(PrototypeAST* const proto)
    {
        if(!verifyPrototype(proto))
        {
            // Prototype is not valid
            return false;
        }

        return true;
    }
    bool VAnalyzer::verifyExtern(ExternAST* const extern_)
    {
        if(!verifyPrototype(extern_->getProto()))
        {
            // Prototype is not valid
            return false;
        }
        
        return true;
    }
    bool VAnalyzer::verifyFunction(FunctionAST* const func)
    {
        //std::cout << ("Verifying function " + func->getNameToken()->value) << std::endl;
        bool is_valid=true;

        if(!verifyPrototype(func->getProto()))
        {
            // Prototype is not valid
            is_valid=false;
        }
        func->setReturnType(types::copyType(func->getProto()->getReturnType()));

        for(auto const& var: func->getArgs())
        {
            defineVariable(var.get(), true);
        }

        if(!verifyBlock(func->getBody()))
        {
            // Block is not valid
            is_valid=false;
        }
        
        for(auto const& var: func->getArgs())
        {
            func->addVariable(var.get());
            undefineVariable(var.get());
        }

        return is_valid;
    }

    bool VAnalyzer::verifyUnionStructBody(std::vector<ExprAST*> const& body)
    {
        bool is_valid=true;

        std::map<std::string, VariableDefAST*> scope;

        for(auto* expr: body)
        {
            if(expr->asttype==ast_vardef)
            {
                auto* var=(VariableDefAST*)expr;
                if(scope.count(var->getName())>0)
                {
                    std::cout << "Redeclaration of variable in struct" << std::endl;
                    is_valid=false;
                }
                else
                {
                    scope.insert(std::make_pair(var->getName(),var));
                }
            }
            else if(expr->asttype==ast_struct)
            {
                auto* struct_=(StructExprAST*)expr;
                struct_->setName("_"+struct_->getName());

                if(scope.count(struct_->getName())>0)
                {
                    std::cout << "Redeclaration of struct-variable in struct" << std::endl;
                    is_valid=false;
                }
                if(!verifyUnionStructBody(struct_->getMembersValues()))
                {
                    // Struct is not valid
                    is_valid=false;
                }

                unsigned int size=0;
                for(auto const& member : struct_->getMembersValues())
                {
                    size+=member->getType()->getSize();
                }

                struct_->getType()->setSize(size);
            }
            else if(expr->asttype==ast_union)
            {
                auto* union_=(UnionExprAST*)expr;
                union_->setName("_"+union_->getName());

                if(scope.count(union_->getName())>0)
                {
                    std::cout << "Redeclaration of union-variable in struct" << std::endl;
                    is_valid=false;
                }
                if(!verifyUnionStructBody(union_->getMembersValues()))
                {
                    // Union is not valid
                    is_valid=false;
                }
            }
        }

        return is_valid;
    }
    bool VAnalyzer::verifyUnion(UnionExprAST* const union_)
    {
        auto const& members=union_->getMembersValues();
        
        if(!verifyUnionStructBody(members))
        {
            // Union body is not valid
            return false;
        }

        return true;
    }
    bool VAnalyzer::verifyStruct(StructExprAST* const struct_)
    {
        bool is_valid=true;

        auto const& members=struct_->getMembersValues();
        if(!verifyUnionStructBody(members))
        {
            // Struct body is not valid
            return is_valid=false;
        }

        unsigned int size=0;
        for(auto const& member : members)
        {
            size+=member->getType()->getSize();
        }

        if(!is_valid)   return is_valid;

        std::string st_name=struct_->getName();
        if(!types::isTypeinMap(st_name))
        {
            types::addTypeToMap(st_name);
            types::addTypeSizeToMap(st_name, size);
        }
        else
        {
            std::cout << "Struct `" << st_name << "` already defined" << std::endl;
            is_valid=false;
        }
        auto struct_ty=types::construct(st_name, true);

        current_struct=struct_;

        if(auto* constructor=struct_->getConstructor())
        {
            constructor->isConstructor(true);
            constructor->doesRequireSelfRef(true);
            constructor->setReturnType(types::copyType(struct_ty.get()));
            constructor->setName(proto::IName(struct_->getIName().name, "struct_construct_"));
            
            auto self_ref=std::make_unique<VariableDefAST>(VToken::construct("self", tok_id), types::copyType(struct_ty.get()), nullptr);
            self_ref->isArgument(true);
            defineVariable(self_ref.get(), true);
            constructor->getModifyableArgs().insert(constructor->getArgs().begin(), std::move(self_ref));

            current_func=constructor;
            if(!verifyFunction(constructor))
            {
                is_valid=false;
            }
        
            undefineVariable(self_ref->getIName());
        }
        else
        {
            // Create a default constructor //

            auto st_iname=struct_->getIName();
            auto func_name=proto::IName(st_iname.name, "struct_construct_");
            constexpr const char* self_ref_name="self";

            std::vector<std::unique_ptr<VariableDefAST>> args;
            std::vector<std::unique_ptr<ExprAST>> new_constructor_body;
            std::vector<VariableDefAST*> vars;
            ReturnExprAST* ret_stm;

            // Create the args
            std::unique_ptr<ExprAST> empty_val;
            auto vardef=std::make_unique<VariableDefAST>(VToken::construct("", tok_id), types::copyType(struct_ty.get()), std::move(empty_val), false, true);
            vardef->setName(proto::IName(self_ref_name, ""));
            vardef->isArgument(true);
            vars.push_back(vardef.get());
            args.push_back(std::move(vardef));
            for(auto const& member : members)
            {
                std::unique_ptr<VariableDefAST> arg;
                std::unique_ptr<ExprAST> empty_val=nullptr;
                if(member->asttype==ast_struct)
                {
                    auto* st=(StructExprAST*)member;
                    arg=std::make_unique<VariableDefAST>(VToken::construct(st->getIName().name, tok_id), types::construct(st->getName(), true), std::move(empty_val));
                }
                else if(member->asttype==ast_vardef)
                {
                    auto* var=(VariableDefAST*)member;
                    arg=std::make_unique<VariableDefAST>(VToken::construct(var->getIName().name, tok_id), types::copyType(var->getType()), std::move(empty_val));
                }

                arg->isArgument(true);
                vars.push_back(arg.get());
                args.push_back(std::move(arg));
            }

            // Create the body
            for(auto const& member : members)
            {
                std::string member_name;

                if(member->asttype==ast_struct)
                    member_name=((StructExprAST*)member)->getIName().name;
                else if(member->asttype==ast_vardef)
                    member_name=((VariableDefAST*)member)->getIName().name;

                auto self_ref=std::make_unique<VariableExprAST>(VToken::construct("", tok_id));
                self_ref->setName(proto::IName(self_ref_name, ""));
                self_ref->setType(types::copyType(struct_ty.get()));
                auto mem=std::make_unique<VariableExprAST>(VToken::construct(member_name, tok_id));
                mem->setType(types::copyType(member->getType()));

                auto lhs=std::make_unique<TypeAccessAST>(std::move(self_ref), std::move(mem));
                lhs->setType(types::copyType(member->getType()));
                
                auto rhs=std::make_unique<VariableExprAST>(VToken::construct(member_name, tok_id));
                rhs->setType(types::copyType(member->getType()));

                new_constructor_body.push_back(std::make_unique<VariableAssignAST>(std::move(lhs), std::move(rhs)));
            }

            // Set the constructor
            auto new_constructor_proto=std::make_unique<PrototypeAST>(VToken::construct(st_iname.name), std::move(args), types::copyType(struct_ty.get()));
            auto new_constructor=std::make_unique<FunctionAST>(std::move(new_constructor_proto), std::move(new_constructor_body));

            new_constructor->setName(func_name);
            new_constructor->addVariables(vars, true);
            new_constructor->addReturnStatement(ret_stm);
            new_constructor->doesRequireSelfRef(true);
            new_constructor->isConstructor(true);

            struct_->setConstructor(std::move(new_constructor));
        }

        addConstructor(struct_->getConstructor());

        return is_valid;
    }
    bool VAnalyzer::verifyTypeAccess(TypeAccessAST* const access)
    {
        bool is_valid=true;

        // Load the struct
        StructExprAST* st=nullptr;

        auto* ptype=getType(access->getParent());
        if(ptype->getType() != types::EType::Custom)
        {
            std::cout << "Parent is not a type" << std::endl;
            return false;
        }
        
        auto* ptype_custom=(types::Custom*)ptype;
        if(!types::isTypeinMap(ptype_custom->getName()))
        {
            std::cout << "Type " << *ptype_custom << " is not defined" << std::endl;
            return false;
        }

        access->getParent()->setType(types::copyType(ptype_custom));
        st=getStruct(ptype_custom->getName());

        IdentifierExprAST* possible_access=access;
        ExprAST* possible_struct_child=st;

        while(possible_access->asttype==ast_type_access)
        {
            auto const* casted_pos_access=(TypeAccessAST*)possible_access;
            auto* child=(TypeAccessAST*)possible_access;

            if(possible_struct_child->asttype!=ast_struct)
            {
                std::cout << "The type is not a struct" << std::endl;
                is_valid=false;
                break;
            }

            auto* casted_pos_stchild=(StructExprAST*)possible_struct_child;

            if(!casted_pos_stchild->isMember(child->getIName()))
            {
                std::cout << "No member as `" << child->getIName().name << "` in struct `" << casted_pos_stchild->getName() << "`." << std::endl;
                is_valid=false;
                break;
            }
            else
            {
                possible_struct_child->setType(std::make_unique<types::Custom>(casted_pos_stchild->getName(), 1));
                casted_pos_access->getParent()->setType(std::make_unique<types::Custom>(casted_pos_stchild->getName(), 1));
                possible_access=child->getChild();
                possible_struct_child=casted_pos_stchild->getMember(child->getIName());
            }
        }

        // Set the type for the tail of the access
        possible_access->setType(types::copyType(possible_struct_child->getType()));

        if(is_valid)
        {
            auto* type=getType(access);
            access->setType(types::copyType(type));
        }

        return is_valid;
    }

    bool VAnalyzer::verifyIfThen(IfThenExpr* const if_then)
    {
        bool is_valid=true;
        const auto& cond=if_then->getCondition();
        const auto& then_block=if_then->getThenBlock();
        
        auto* cond_type=getType(cond);

        if(cond_type->getType()!=types::EType::Bool)
        {
            auto bool_type=types::construct(types::EType::Bool);
            auto cast=tryCreateImplicitCast(bool_type.get(), cond_type, if_then->moveCondition());

            if(!cast)
            {
                std::cout << "Condition needs to be of a boolean type or a numeric type";
                is_valid=false;
            }
            else
            {
                if_then->setCondition(std::move(cast));
            }
        }

        if(!verifyExpr(cond))
        {
            // Cond is not valid
            is_valid=false;
        }
        if(!verifyBlock(then_block))
        {
            // Then block is not valid
            is_valid=false;
        }

        return is_valid;
    }
    bool VAnalyzer::verifyIf(IfExprAST* const if_)
    {
        bool is_valid=true;

        if(!verifyIfThen(if_->getIfThen()))
        {
            // IfThen is not valid
            is_valid=false;
        }
        for(const auto& else_if : if_->getElifLadder())
        {
            if(else_if->asttype!=ast_if)
            {
                if(!verifyIfThen(((std::unique_ptr<IfThenExpr> const&)else_if).get()))
                {
                    // ElseIf is not valid
                    is_valid=false;
                }
            }
            else
            {
                if(!verifyBlock(else_if->getThenBlock()))
                {
                    // ElseIf block is not valid
                    is_valid=false;
                }
            }
        }
        
        return is_valid;
    }

    bool VAnalyzer::verifyUnsafe(UnsafeExprAST* const unsafe)
    {
        const auto& block=unsafe->getBody();
        if(!verifyBlock(block))
        {
            // Block is not valid
            return false;
        }
        return true;
    }
    bool VAnalyzer::verifyReference(ReferenceExprAST* const reference)
    {
        const auto& expr=reference->getVariable();
        if(!verifyExpr(expr))
        {
            // Expr is not valid
            return false;
        }
        return true;
    }

    bool VAnalyzer::verifyClass(ClassAST* const cls)
    {
        if(isClassDefined(cls->getName()))
        {
            // Class is already defined
            return false;
        }
        
        const auto& members=cls->getMembers();
        const auto& funcs=cls->getFunctions();

        bool is_valid=true;
        for(auto const& member : members)
        {
            if(!verifyVariableDefinition((VariableDefAST*const&)member))
            {
                // Member is not valid
                return false;
            }
        }
        for(auto const& func : funcs)
        {
            if(func->is_extern())
            {
                is_valid=false;
            }
            else if(func->is_proto())
            {
                if(!verifyPrototype((PrototypeAST*const&)func))
                {
                    // Prototype is not valid
                    return false;
                }
            }
            else
            {
                const auto& func_cast=(FunctionAST*const&)func;

                if(!verifyPrototype((func_cast->getProto())))
                {
                    // Prototype is not valid
                    return false;
                }
                if(!verifyBlock(func_cast->getBody()))
                {
                    // Block is not valid
                    return false;
                }
            }
        }

        return true;
    }

    bool VAnalyzer::verifyBlock(std::vector<std::unique_ptr<ExprAST>> const& block)
    {
        auto refscope=std::vector<VariableDefAST*>();
        this->scope_varref=&refscope;

        for(auto const& expr : block)
        {
            auto* ptr=expr.get();
            if(!verifyExpr(ptr))
            {
                // Expr is not valid
                return false;
            }
        }
        for(const auto& var : refscope)
        {
            undefineVariable(var);
        }

        this->scope_varref=nullptr;
        
        return true;
    }

    bool VAnalyzer::verifySourceModule(std::unique_ptr<ModuleAST> code)
    {
        if(!code)
        {
            return false;
        }

        bool is_valid=true;
        ast=std::move(code);

        auto classes=ast->moveClasses();
        auto funcs=ast->moveFunctions();
        auto union_structs=ast->moveUnionStructs();
        auto pre_stms=ast->movePreExecutionStatements();
        auto constructors=ast->moveConstructors();

        bool has_main=false;
        unsigned int main_func_indx=0;

        // Verify all unions and structs
        for(unsigned int it=0; it<union_structs.size(); ++it)
        {
            const auto& union_struct=union_structs[it];
            if(union_struct->asttype==ast_union)
            {
                if(!verifyUnion((UnionExprAST*)union_struct.get()))
                {
                    // Union is not valid
                    is_valid=false;
                }
            }
            else
            {
                if(!verifyStruct((StructExprAST*)union_struct.get()))
                {
                    // Struct is not valid
                    is_valid=false;
                }
            }

            ast->addUnionStruct(std::move(union_structs[it]));
        }

        // Verify all functions
        for(unsigned int it=0; it<funcs.size(); ++it)
        {
            const auto& func=funcs[it];
            
            if(func->is_extern())
            {
                if(!verifyExtern(((std::unique_ptr<ExternAST> const&)func).get()))
                {
                    // Extern is not valid
                    is_valid=false;
                }
            }
            else if(func->is_proto())
            {
                if(!verifyPrototype(((std::unique_ptr<PrototypeAST> const&)func).get()))
                {
                    // Prototype is not valid
                    is_valid=false;
                }
            }
            else
            {
                auto const& casted_func=((std::unique_ptr<FunctionAST>const&)func).get();
                current_func=casted_func;
                if(!verifyFunction(casted_func))
                {
                    // Function is not valid
                    is_valid=false;
                }

                if(casted_func->getName()=="main")
                {
                    has_main=true;
                    main_func_indx=it;
                }
            }
        
            addFunction(std::move(funcs[it]));
        }

        // Verify all statements in global scope
        current_func=nullptr;
        auto global_refscope=std::vector<VariableDefAST*>();
        this->scope_varref=&global_refscope;
        for(const auto& expr : pre_stms)
        {
            if(!verifyExpr(expr.get()))
            {
                is_valid=false;
            }
        }
        this->scope_varref=nullptr;

        ast->addPreExecutionStatements(std::move(pre_stms));
        ast->addPreExecutionStatementVariables(global_refscope);
        ast->addConstructors(constructors);

        return is_valid;
    }

    bool VAnalyzer::verifyExpr(ExprAST* const expr)
    {
        switch(expr->asttype)
        {
            case ast_int: return verifyInt((IntExprAST*const&)expr);
            case ast_float: return verifyFloat((FloatExprAST*const&)expr);
            case ast_double: return verifyDouble((DoubleExprAST*const&)expr);
            case ast_str: return verifyStr((StrExprAST*const&)expr);
            case ast_bool: return verifyBool((BoolExprAST*const&)expr);
            case ast_char: return verifyChar((CharExprAST*const&)expr);

            case ast_binop: return verifyBinop((BinaryExprAST*const&)expr);
            case ast_unop: return verifyUnop((UnaryExprAST*const&)expr);

            case ast_incrdecr: return verifyIncrementDecrement((IncrementDecrementAST*const&)expr);
            case ast_var: return verifyVariable((VariableExprAST*const&)expr);
            case ast_vardef: return verifyVariableDefinition((VariableDefAST*const&)expr);
            case ast_varassign: return verifyVarAssign((VariableAssignAST*const&)expr);
            case ast_array_access: return verifyVarArrayAccess((VariableArrayAccessAST*const&)expr);

            case ast_type_access: return verifyTypeAccess((TypeAccessAST*const&)expr);

            case ast_call: return verifyCall((CallExprAST*const&)expr);

            case ast_for: return verifyFor((ForExprAST*const&)expr);
            case ast_while: return verifyWhile((WhileExprAST*const&)expr);
            case ast_array: return verifyArray((ArrayExprAST*const&)expr);

            case ast_break: return verifyBreak((BreakExprAST*const&)expr);
            case ast_continue: return verifyContinue((ContinueExprAST*const&)expr);
            case ast_return: return verifyReturn((ReturnExprAST*const&)expr);

            case ast_ifelse: return verifyIf((IfExprAST*const&)expr);
            
            case ast_cast: return verifyCastExpr((CastExprAST*const&)expr);

            default: return false;
        }
    }
}