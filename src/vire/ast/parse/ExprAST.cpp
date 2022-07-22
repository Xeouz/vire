#pragma once

#include <string>
#include <memory>
#include <vector>

#include "vire/lex/include.hpp"
#include "vire/ast/types/type.cpp"

namespace vire
{

class ExprAST{
protected:
    std::unique_ptr<types::Base> type;
    std::unique_ptr<Viretoken> token;
public:
    int asttype;
    ExprAST(const std::string& type, int asttype, std::unique_ptr<Viretoken> token=nullptr)
    : asttype(asttype), token(std::move(token)) 
    {
        std::unique_ptr<types::Base> t(types::construct(type));
        this->type = std::move(t);
    }

    virtual ~ExprAST() {}
    virtual types::Base* getType() const {return type.get();}
    virtual void setType(std::string const& newtype) { type.reset(types::construct(newtype)); }

    virtual const std::size_t& getLine()    const {return token->line;}
    virtual const std::size_t& getCharpos() const {return token->charpos;} 
    virtual void setToken(std::unique_ptr<Viretoken> token) 
    { this->token.reset(); this->token=std::move(token); }
};

}