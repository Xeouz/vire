#pragma once

#include <string>
#include <memory>

#include "ASTType.hpp"
#include "ExprAST.cpp"

namespace vire
{
// VariableExprAST - Class for referencing a variable, eg - `myvar`
class VariableExprAST : public ExprAST
{
    std::string Name;
public:
    VariableExprAST(std::unique_ptr<Viretoken> Name) : Name(Name->value), ExprAST("",ast_var) 
    {setToken(std::move(Name));} 

    std::string const& getName() const {return Name;}
    Viretoken* const getToken() const {return token.get();}
    std::unique_ptr<Viretoken> moveToken() {return std::move(token);}
};

class VariableAssignAST: public ExprAST
{
    std::string Name;
    std::unique_ptr<ExprAST> Value;
public: 
    VariableAssignAST(std::unique_ptr<Viretoken> Name, std::unique_ptr<ExprAST> Value)
    : Name(Name->value), Value(std::move(Value)), ExprAST("void",ast_varassign) {setToken(std::move(Name));}

    std::string const& getName() const {return Name;}
    ExprAST* const getValue() const {return Value.get();}
};

class VariableDefAST : public ExprAST
{
    std::string Name;
    std::unique_ptr<ExprAST> Value;
    bool is_const, is_let, is_array;
public:
    VariableDefAST(std::unique_ptr<Viretoken> Name, std::unique_ptr<types::Base> type, std::unique_ptr<ExprAST> Value,
    bool is_const=0, bool is_let=0)
    : Name(Name->value),Value(std::move(Value)),ExprAST(std::move(type),ast_vardef), 
    is_const(is_const),is_let(is_let) 
    { setToken(std::move(Name)); }

    std::string const& getName() const {return Name;}
    const bool& isConst() const {return is_const;}
    const bool& isLet() const {return is_let;}

    ExprAST* const getValue() const {return Value.get();}
    std::unique_ptr<ExprAST> moveValue() {return std::move(Value);}
    void setValue(std::unique_ptr<ExprAST> Value) {this->Value=std::move(Value);}
};

class TypedVarAST : public ExprAST
{
    std::unique_ptr<Viretoken> Name;
public:
    TypedVarAST(std::unique_ptr<Viretoken> Name, std::unique_ptr<Viretoken> Type) 
    : Name(std::move(Name)), ExprAST(Type->value,ast_typedvar)
    {setToken(std::move(Type));}

    std::string const& getName() const {return Name->value;}
};

class VariableIncrDecrAST : public ExprAST
{
    std::string Name;
    bool isincr, ispre;
public:
    VariableIncrDecrAST(std::unique_ptr<Viretoken> Name, bool isincr, bool ispre)
    : Name(Name->value), ExprAST("void",ast_varincrdecr), isincr(isincr), ispre(ispre)
    {setToken(std::move(Name));}
    
    std::string const& getName() const {return Name;}
    bool isIncr() const {return isincr;}
    bool isPre() const {return ispre;}
};

}