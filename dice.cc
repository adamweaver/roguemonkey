// -*- Mode: C++ -*-
// RogueMonkey (c) 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <cassert>
#include <cctype>
#include <ctime>
#include <cstdlib>
#include <stack>
#include <queue>
#include <utility>

#include "dice.h"


namespace
{
    Dice & globalDice()
    {
        static Dice gldice(0, 32767, long(std::time(0)));
        static struct bob
        {
            bob()
            {
                std::srand(static_cast<unsigned int>(std::time(0)));
            }
        } bobbob;

        return gldice;
    }

    int ParseNumber(char const * & ptr)
    {
        int num = 0;
        while (std::isdigit(*ptr))
        {
            num *= 10;
            num += (*ptr++ - '0');
        }
        return num;
    }

    bool IsOperator(char c)
    {
        return c == 'D' || c == 'd' || c == '^' || c == '*' || 
               c == '/' || c == '%' || c == '+' || c == '-' || 
               c == 'm' || c == 'M';
    }

    int GetOperatorPrecedence(char c)
    {
        int prec = 0;
        switch (c)
        {
        case 'D' : 
        case 'd' :  prec = 10; break;
        case '^' :  prec =  9; break;
        case '*' :  prec =  8; break;
        case '/' :  prec =  8; break;
        case '%' :  prec =  8; break;
        case '+' :  prec =  6; break;
        case '-' :  prec =  6; break;
        case 'm' :  prec =  4; break;
        case 'M' :  prec =  4; break;
        case '(' :  prec = -1; break;
        default: 
            assert(!"GetOperatorPrecedence called for non-operator");
            break;
        }
        return prec;
    }

    enum TokenType { Number, Operator };
    typedef std::stack<char>          OperStack;
    typedef std::pair<TokenType, int> Token;  
    typedef std::queue<Token>         Input;
    typedef std::stack<int>           OutputStack;    

}


//============================================================================
// Dice
//============================================================================

Dice::Dice(int mmin, int mmax, int seed) :
    m_rng(static_cast<boost::uint32_t>(seed)),
    m_type(mmin, mmax),
    m_variate(m_rng, m_type)
{
}


Dice::Dice(int mmin, int mmax) :
    m_rng(),
    m_type(mmin, mmax),
    m_variate(m_rng, m_type)
{
}


int
Dice::Random0(int mmax)
{
    static int const imax = std::numeric_limits<int>::max();
    return mmax > 0 ? std::abs(globalDice().rnd() / (imax / mmax + 1)) : 0;
}


int
Dice::Random(char const *desc)
{
    if (!desc || !*desc)
        return 0;

    // Shunting algorithm from http://montcs.bloomu.edu/~bobmon/Information/RPN/infix2rpn.shtml
    OperStack operators;
    Input    input;
    char const *ptr = desc;

    while (*ptr)
    {
        if (std::isdigit(*ptr))
            input.push(Token(Number, ParseNumber(ptr)));
        else if (IsOperator(*ptr))
        {
            int prec = GetOperatorPrecedence(*ptr);
            while (!operators.empty() && prec <= GetOperatorPrecedence(operators.top()))
            {
                input.push(Token(Operator, operators.top()));
                operators.pop();
            }
            operators.push(*ptr++);
        }
        else if (*ptr == '(')
            operators.push(*ptr++);
        else if (*ptr++ == ')')
        {
            while (!operators.empty() && operators.top() != '(')
            {
                input.push(Token(Operator, operators.top()));
                operators.pop();
            }
            if (operators.empty())
                return 0; // illegal input (unbalanced parentheses)
            operators.pop();
        }
        else
            return 0; // illegal input (unexpected character)
    }

    // now put in input queue
    while (!operators.empty())
    {
        if (operators.top() == '(')
            return 0; // illegal input (unbalanced parentheses)
        input.push(Token(Operator, operators.top()));
        operators.pop();
    }


    // now do calc
    OutputStack outputstack;
    while (!input.empty())
    {
        Token tok = input.front();
        input.pop();
        if (tok.first == Number)
            outputstack.push(tok.second);
        else
        {
            int right = outputstack.top(); outputstack.pop();
            int left  = outputstack.top(); outputstack.pop();
            int result = 0;
            switch (tok.second)
            {
            case 'd':
                for ( ; left; --left)
                    result += Random0(right) + 1;
                break;
            case '^':
                result = 1;
                for ( ; right; --right)
                    result *= left;
                break;
            case '*':
                result = left * right;
                break;
            case '/':
                result = left / right;
                break;
            case '%':
                result = left % right;
                break;
            case '+':
                result = left + right;
                break;
            case '-':
                result = left - right;
                break;
            case 'm':
                result = left < right ? right : left;
                break;
            case 'M':
                result = left > right ? right : left;
                break;
            default:
                assert(!"Unbalanced RPN during calculation!");
                break;
            }
            outputstack.push(result);
        }
    }

    int result = outputstack.top(); outputstack.pop();
    if (!outputstack.empty())
        assert(!"Unbalanced RPN after calculation!");

    return result;
}


int
Dice::Random(std::string const & desc)
{
    return Random(desc.c_str());
}


int
Dice::rnd()
{
    return m_rng();
}


