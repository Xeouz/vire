#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

namespace vire
{

class IdentifierExprAST : public ExprAST
{
    proto::IName name;
public:
    bool is_const;
    IdentifierExprAST(std::unique_ptr<VToken> name, bool is_const=false, int asttype=ast_var) 
    : name(name->value), ExprAST("", asttype), is_const(is_const)
    {
        setToken(std::move(name));
    }

    virtual std::string const getName() const
    {
        return name.get();
    }
    virtual proto::IName const& getIName() const
    {
        return name;
    }
    virtual std::unique_ptr<VToken> moveToken()
    {
        return std::move(token);
    }
    
    virtual void setName(std::string const& _name)
    {
        name.setName(_name);
    }
    virtual void setName(proto::IName const& _name)
    {
        name=_name;
    }
};

// VariableExprAST - Class for referencing a variable, eg - `myvar`
class VariableExprAST : public IdentifierExprAST
{
public:
    VariableExprAST(std::unique_ptr<VToken> name) : 
    IdentifierExprAST(std::move(name))
    {
    } 
};

class TypeAccessAST : public IdentifierExprAST
{
    std::unique_ptr<ExprAST> parent;
    std::unique_ptr<IdentifierExprAST> child;
public:
    TypeAccessAST(std::unique_ptr<ExprAST> _parent, std::unique_ptr<IdentifierExprAST> _child)
    : parent(std::move(_parent)), child(std::move(_child)), 
    IdentifierExprAST(VToken::construct(""), false, ast_type_access)
    {
        if(child->asttype==ast_type_access)
        {
            auto* cast_child=(TypeAccessAST*)child.get();
            auto* cast_child_child=(VariableExprAST*)cast_child->getParent();
            setName(cast_child_child->getIName());
            setToken(VToken::construct(cast_child_child->getIName().name));
        }
        else
        {
            setName(child->getIName());
            setToken(VToken::construct(child->getIName().name));
        }
    }

    ExprAST* const getParent() const
    {
        return parent.get();
    }
    IdentifierExprAST* const getChild() const
    {
        return child.get();
    }

};

}