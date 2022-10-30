#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include <memory>
#include <vector>

namespace vire
{

class TypeAST : public ExprAST
{
    std::unordered_map<std::string, std::unique_ptr<ExprAST>> members;
    std::unordered_map<std::string, int> members_indx;
    std::unique_ptr<VToken> name;
public:
    TypeAST(std::unordered_map<std::string, std::unique_ptr<ExprAST>> members, std::unique_ptr<VToken> name, int asttype=ast_type)
    : members(std::move(members)), name(std::move(name)), ExprAST("void", asttype)
    {
        int i=this->members.size()-1;
        for(auto& [str, ptr] : this->members)
        {
            this->members_indx[str]=i--;
        }

    }

    virtual std::string const& getName() const
    {
        return name->value;
    }
    virtual void setName(std::string new_name)
    {
        name->value=new_name;
    }

    virtual std::unordered_map<std::string, std::unique_ptr<ExprAST>> const& getMembers()
    {
        return members;
    }
    virtual std::vector<ExprAST*> const getMembersValues() const
    {
        std::vector<ExprAST*> values;
        values.reserve(members.size());

        for(auto& [str,ptr]:members)
        {
            values.push_back(ptr.get());
        }

        return values;
    }
    virtual int const getMemberIndex(std::string const& name)
    {
        return members_indx.at(name);
    }

    virtual bool isMember(std::string const& name)
    {
        if(members.count(name)>0)
        {
            return true;
        }

        return false;
    }
    virtual ExprAST* getMember(std::string const& name)
    {
        return members.at(name).get();
    }
};

class UnionExprAST : public TypeAST
{
public:
    UnionExprAST(std::unordered_map<std::string, std::unique_ptr<ExprAST>> members, std::unique_ptr<VToken> name)
    : TypeAST(std::move(members), std::move(name), ast_union)
    {
    }
};

class StructExprAST : public TypeAST
{
public:
    StructExprAST(std::unordered_map<std::string, std::unique_ptr<ExprAST>> members, std::unique_ptr<VToken> name)
    : TypeAST(std::move(members), std::move(name), ast_struct)
    {
    }
};

}