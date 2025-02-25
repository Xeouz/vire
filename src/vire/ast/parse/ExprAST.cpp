#pragma once

#include <string>
#include <memory>
#include <vector>

#include "vire/lex/include.hpp"
#include "vire/ast/types/type.hpp"

#include "ASTType.hpp"

#include "vire/proto/iname.hpp"

namespace vire
{

class ExprAST
{
protected:
    std::unique_ptr<types::Base> type;
    std::unique_ptr<VToken> token;
public:
    int asttype;
    ExprAST(const std::string& type, int asttype, std::unique_ptr<VToken> token=nullptr)
    : asttype(asttype), token(std::move(token)), type(types::construct(type))
    {}

    ExprAST(std::unique_ptr<types::Base> type, int asttype, std::unique_ptr<VToken> token=nullptr)
    : asttype(asttype), token(std::move(token)), type(std::move(type))
    {}

    virtual ~ExprAST() = default;

    virtual std::unique_ptr<ExprAST> copyAST() const
    {
        return std::make_unique<ExprAST>(types::copyType(type.get()), asttype, VToken::construct(token.get()));
    }

    virtual types::Base* getType() const 
    {
        return type.get(); 
    }

    virtual bool refreshType()
    {
        bool refreshed=refreshType(this->getType());
        return refreshed;
    }
    virtual bool refreshType(types::Base* t)
    {
        if(t->getType()==types::EType::Void)
        {
            auto* voidty=(types::Void*)t;
            if(types::isTypeinMap(voidty->getName()))
            {
                type=types::construct(voidty->getName(), true);
                return true;
            }
        }

        return false;
    }
    virtual void setType(std::unique_ptr<types::Base> t)
    {
        bool refreshed=refreshType(t.get());
        if(!refreshed)
        {
            type=std::move(t);
        }
    }
    virtual void setType(std::string const& newtype) 
    {
        setType(types::construct(newtype)); 
    }
    virtual void setType(types::Base* t)
    {
        setType(std::unique_ptr<types::Base>(t));
    }

    virtual const std::size_t& getLine()    const 
    {
        return token->line;
    }
    virtual const std::size_t& getCharpos() const 
    {
        return token->charpos;
    } 
    virtual void setToken(std::unique_ptr<VToken> token) 
    {
        this->token.reset(); this->token=std::move(token);
    }
    virtual VToken* const getToken()     const
    {
        return token.get();
    }
};

}