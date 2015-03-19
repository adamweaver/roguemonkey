// -*- Mode: C++ -*-
// RogueMonkey copyright 2008 Adam White theroguemonkey@gmail.com
// Released under the GPL version 2 - refer to included file LICENCE.txt

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <sstream>
#include <utility>

#include "creature.h"
#include "textutils.h"

using std::string;

namespace
{
    // these are required because C++ inherited to[upp|low]er from C89, which
    // declares them as int (*) (int) functions... this does not deduce well
    // with the usual functors. I could hack around it with a currying
    // template, but obscure templates are evil.
    char MyToLower(char i)
    {
        return static_cast<char>(std::tolower(i));
    }

    char MyToUpper(char i)
    {
        return static_cast<char>(std::toupper(i));
    }

    bool MyCICompare(char const l, char const r)
    {
        return std::tolower(l) == std::tolower(r);
    }
}


string &
TextUtils::LowercaseString(string & orig)
{
    std::transform(orig.begin(), orig.end(), orig.begin(), MyToLower);
    return orig;
}


string &
TextUtils::UppercaseString(string & orig)
{
    std::transform(orig.begin(), orig.end(), orig.begin(), MyToUpper);
    return orig;
}


string
TextUtils::Cap1st(string const & orig)
{
    string out(orig.begin(), orig.end());
    if (!orig.empty())
        out[0] = MyToUpper(out[0]);
    return out;
}


bool
TextUtils::CaseInsensitiveCompare(string const & lhs, string const & rhs)
{
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin(), MyCICompare);
}


bool
TextUtils::IsVowel(char v)
{
    return v == 'a' || v == 'e' || v == 'i' || v == 'o' || v == 'u';
}


string &
TextUtils::TrimFrontAndBack(string & orig, string const & rubbish)
{
    string::size_type last = orig.find_last_not_of(rubbish);
    orig.erase(last + 1);
    last = orig.find_first_not_of(rubbish);
    orig.erase(0, last);
    return orig;
}


string 
TextUtils::GetToken(string const & str, string::const_iterator & out, 
                    string const & delims, string const & escapers, 
                    string const & quote)
{
    // make sure the iterator actually belongs to this string!
    assert(out >= str.begin() && out <= str.end());

    bool escaped = false;
    bool quoted = false;
    string curtoken;

    // strip leading delims
    while (out != str.end() && delims.find(*out) != string::npos)
        ++out;

    for ( ; out != str.end(); ++out)
    {
        char const curchar = *out;

        // is this an escaper?
        if (escapers.find(curchar) != string::npos)
        {
            if (escaped || quoted)
                curtoken += curchar;
            escaped = !escaped;
        }
        // what about a quote char?
        else if (quote.find(curchar) != string::npos)
        {
            if (escaped)
                curtoken += curchar;
            quoted = !quoted;
        }
        // is it a delimiter?
        else if (delims.find(curchar) != string::npos)
        {
            if (escaped || quoted)
            {
                curtoken += curchar;
                escaped = false;
            }
            else
            {
                while (out != str.end() && delims.find(*out) != string::npos)
                    ++out;
                break;
            }
        }
        // nope, we got a valid character
        else
        {
            curtoken += curchar;
        }
    }
    return curtoken;

}


// using iterators rather than indices. string::find_first_not_of() and pals
// might be more elegant, but indices into strings are icky.
std::vector<string>
TextUtils::Tokenise(string const & str, string const & delims,
                    string const & escapers, string const & quote)
{
    std::vector<string> tokens;
    string::const_iterator out = str.begin();
    while (out != str.end())
    {
        tokens.push_back(GetToken(str, out, delims, escapers, quote));
    }
    return tokens;

}



//============================================================================
// Pluralising Stuff
//============================================================================
namespace
{
    int const IrregsPerWordLength = 18;
    int const IrregsWordLength = 11;

    struct StringToString
    {
        char const * original;
        char const * replacement;
    };


    // Must be kept sorted into WORDLENGTH and ALPHABETICAL
    StringToString Irregulars[IrregsWordLength][IrregsPerWordLength] =
    {
        // 2
        {{"ox", "oxen"}},

        // 3
        {{"are", "is"}, {"cod", 0}, {"elk", 0}, {"pro", "pros"}},

        // 4
        {{"beef", "beeves"}, {"carp", 0}, {"have", "has"}, {"mews", 0}, {"news", 0}, {"tuna", 0}},

        // 5
        {{"afrit", "afriti"}, {"bison", 0}, {"bream", 0}, {"child", "children"},
        {"cherub", "cherubim"}, {"corps", 0}, {"djinn", 0}, {"focus", "foci"},
        {"goose", "geese"}, {"guano", "guanos"}, {"index", "indices"},
        {"money", "monies"}, {"mouse", "mice"}, {"mumak", "mumakil"},
        {"rhino", "rhinos"}, {"sinus", 0}, {"swine", 0}, {"trout", 0}},

        // 6
        {{"afreet", "afreeti"}, {"bonobo", "bonobos"}, {"debris", 0},
        {"efreet", "efreeti"}, {"eland", 0}, {"fungus", "fungi"},
        {"radius", "radii"}, {"salmon", 0}, {"seraph", "seraphim"},
        {"series", 0}, {"shears", 0}, {"sphynx", "sphynxes"}, {"status", 0},
        {"vortex", "vortices"}},

        // 7
        {{"albino", "albinos"},{"gallows", 0}, {"incubus", "incubi"},
        {"inferno", "infernos"}, {"mongoose", "mongooses"}, {"pincers", 0},
        {"species", 0}, {"whiting", 0}},

        // 8
        {{"breeches", 0}, {"britches", 0}, {"commando", "commandos"},
        {"flounder", 0}, {"graffiti", 0}, {"mackerel", 0},
        {"succubus", "succubi"}},

        // 9
        {{"apparatus", 0}, {"armadillo", "armadillos"},
        {"vertebra", "vertebrae"}},

        // 10
        {{"wildebeest", 0}},

        // 11
        {{"candelabrum", "candelabra"}},

        // 12
        {{"headquarters", 0}}
     };

     // Must be kept sorted into SUFFIXLENGTH
     StringToString IrregularSuffixes[] =
     {
         // 2
         {"ch", "ches"}, {"sh", "shes"}, {"ss", "sses"},

         // 3
         {"alf", "alves"}, {"elf", "elves"}, {"olf", "olves"}, {"eaf", "eaves"},
         {"arf", "arves"},  {"ois", 0}, {"eau", "eaux"}, {"ieu", "ieux"},
         {"inx", "inges"}, {"anx", "anges"}, {"ynx", "ynges"}, {"oof", "ooves"},

         // 4
         {"nife", "nives"}, {"wife", "wives"}, {"fish", 0}, {"deer", 0},
         {"pox", 0}, {"itis", 0}, {"trix", "trices"},

         // 5
         {"sheep", 0}
     };

     int const IrregularSuffixSize = sizeof(IrregularSuffixes) / sizeof(IrregularSuffixes[0]);
 }
//============================================================================




string
TextUtils::PluraliseString(string const & orig, int number)
{
    string::size_type start = orig.find('#');
    string str(orig, 0, start);

    while(start != string::npos)
    {
        string::size_type next = orig.find_first_of(" .,!?:", ++start);
        string word(orig, start, next - start);

        if(number != 1)
            str += PluraliseWord(word);
        else
            str += word;

        if(next != string::npos)
        {
            start = orig.find('#', next);
            str.append(orig, next, start - next);
        }
        else
            start = string::npos;
    }
    return str;
}



string
TextUtils::PluraliseWord(string const & orig)
{
    // 1st Is the word irregular?
    // 2nd Does it have an irregular suffix?
    // 3rd *[aeiou]y => *[aeiou]ys
    //     else *y => *ies
    // 4th *[aeiou]o => *[aeiou]os
    //     else *o => *oes
    // 5th *[aeiou]s => *[aeiou]ses
    // 6th * => *s

    char const *origcstr = orig.c_str();
    int const origlen = static_cast<int>(orig.length());

    if (origlen < IrregsWordLength + 2)
    {
        int ol = origlen - 2;

        // 1st
        for (int i = 0; i < IrregsPerWordLength && Irregulars[ol][i].original; ++i)
        {
            int result = std::strcmp(origcstr, Irregulars[ol][i].original);
            if (result == 0)
            {
                return Irregulars[ol][i].replacement ?
                    Irregulars[ol][i].replacement : Irregulars[ol][i].original;
            }
            if (result < 0)
                break;
        }
    }

    // 2nd
    for (int i = 0; i < IrregularSuffixSize; ++i)
    {
        int sz = static_cast<int>(std::strlen(IrregularSuffixes[i].original));
        if (sz > origlen)
            break;
        if (strcmp(IrregularSuffixes[i].original, origcstr + origlen - sz) == 0)
        {
            return string(orig.begin(), orig.end() - sz) +
                (IrregularSuffixes[i].replacement ?
                IrregularSuffixes[i].replacement : IrregularSuffixes[i].original);
        }
    }

    if (origlen > 2)
    {
        // 3rd
        if (*(orig.end() - 1) == 'y')
        {
            if (IsVowel(*(orig.end() - 2)))
                return orig + 's';
            else
                return string(orig.begin(), orig.end() - 1) + "ies";
        }

        // 4th
        if (*(orig.end() - 1) == 'o')
        {
            return orig + (IsVowel(*(orig.end() - 2)) ? "s" : "es");
        }

        // 5th
        if (*(orig.end() - 1) == 's' && IsVowel(*(orig.end() - 2)))
        {
            return orig + "es";
        }
    }

    // 6th
    return orig + 's';
}




string
TextUtils::IndefiniteArticle(string const & orig)
{
    assert(orig.size() > 0);

    // TODO: honour, etc
    return  IsVowel(orig[0]) ? "an" : "a";
}


bool
TextUtils::IsTrue(string const & str)
{
    static string const Yes("yes");
    static string const True("true");

    return CaseInsensitiveCompare(str, Yes) || CaseInsensitiveCompare(str, True) || 
           (!str.empty() && (str[0] == '1' || MyToLower(str[0]) == 'y'));
}

//============================================================================
// Grammar & Formatting
//============================================================================
namespace
{
    string const Personals[3]   = { "he", "she", "it" };
    string const Pronouns[3]    = { "him", "her", "it" };
    string const Possessives[3] = { "his", "hers", "its" }; 
    string const Reflexives[3]  = { "himself", "herself", "itself" };    

    int GetNumber(string::const_iterator & b, string::const_iterator e)
    {
        int num = 0;
        while (b != e && std::isdigit(*b))
        {
            num *= 10;
            num += (*b++ - '0');
        }
        return num;
    }

    string GetWord(string::const_iterator & b, string::const_iterator e)
    {
        string::const_iterator start = b;
        while (b != e && isalnum(*b))
            ++b;
        return string(start, b);
    }
}


/*
$1n $hit $2a with $3p $4a.

You hit the orc with your sword.
The orc hits itself with its sword.
You hit the goblin with the orc’s sword.
*/
string 
TextUtils::FormatMessage(string const & message, CreatureH viewer, std::vector<DescribableH> const & desc)
{
    static DescribableH last;
    DescribableH you = viewer->getDescribableHandle();
    std::vector<string> names(desc.size());
    string output;
    string::const_iterator pos = message.begin();
    DescribableH current_nom = you;
    Gender current_gen = current_nom->getGender();

    while (pos != message.end())
    {
        string::const_iterator pos2 = std::find(pos, message.end(), '$');
        output.append(pos, pos2);

        if (pos2 == message.end() || ++pos2 == message.end())
            return output;

        pos = pos2;
        if (std::isdigit(*pos))
        {
            int num = GetNumber(pos, message.end()) - 1;
            if (num < 0 || num >= static_cast<int>(desc.size()))
            {
                output.append("BUGGY NOUN");
                continue;
            }

            char noun_type = 'a';
            if (pos != message.end())
                noun_type = *pos++;

            switch (noun_type)
            {
            case 'n' : case 'N' :
                current_nom = desc[num];
                current_gen = current_nom->getGender();
                if (current_nom == viewer)
                    output.append("you");
                else if (current_nom == last)
                    output.append(Personals[current_gen]);
                else
                    output.append(current_nom->describe());
                break;

            case 'a' : case 'A' : case 'd' : case 'D' :
                if (desc[num] == viewer)
                    output.append(last == viewer ? "yourself" : "you");
                else if (desc[num] == last)
                    output.append(current_nom == desc[num] ? Reflexives[current_gen] :  Pronouns[current_gen]);
                else
                    output.append(desc[num]->describe());
                break;

            case 'p' : case 'P' :
                if (desc[num] == viewer)
                    output.append("your");
                else if (desc[num] == last)
                    output.append(Possessives[last->getGender()]);
                else
                {
                    output.append(desc[num]->describe());
                    char final = (output.empty()) ? ' ' : *(output.end() - 1);
                    if (final == 's' || final == 'z' || final == 'x')
                        output += '\'';
                    else 
                        output.append("\'s");
                }
                break;

            default:
                output.append(desc[num]->describe());
                output.append("{missing case}");
                break;
            }
        }
        else
        {
            string verb = GetWord(pos, message.end());
            output.append(current_nom == viewer ? verb : PluraliseWord(verb));
        }
    }
    return output;
}


string 
TextUtils::FormatMessage(string const & message, CreatureH viewer, DescribableH d1, DescribableH d2, 
                         DescribableH d3, DescribableH d4, DescribableH d5, DescribableH d6)
{
    std::vector<DescribableH> desc; desc.reserve(6);
    desc.push_back(d1); desc.push_back(d2);
    desc.push_back(d3); desc.push_back(d4);
    desc.push_back(d5); desc.push_back(d6);
    return FormatMessage(message, viewer, desc);
}


namespace
{
    typedef std::pair<int, char const *> RomanVal;
    RomanVal romans[] = 
    {
        RomanVal(1000, "M"), RomanVal(900, "CM"), RomanVal(500, "D"), RomanVal(400, "CD"),
        RomanVal( 100, "C"), RomanVal( 90, "XC"), RomanVal( 50, "L"), RomanVal( 40, "XL"),
        RomanVal(  10, "X"), RomanVal(  9, "IX"), RomanVal(  5, "V"), RomanVal(  4, "IV"),
        RomanVal(   1, "I")
    };
    int const romans_size = sizeof(romans) / sizeof(romans[0]);
}


string 
TextUtils::RomanNumerals(int num)
{
    if (num > 3999 || num < 1)
        return "N/A";
    string word;

    for (int i = 0; num > 0 && i < romans_size; ++i)
    {
        while (num >= romans[i].first)
        {
            word.append(romans[i].second);
            num -= romans[i].first;
        }
    }
    return word;
}
