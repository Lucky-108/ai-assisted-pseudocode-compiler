%{
#include <cstdlib>
#include <string>
#include "parser.tab.h"
%}

%option noyywrap nounput noinput

%%

"PRINT"                     { return PRINT; }
"INT"                       { return INT; }
"FLOAT"                     { return FLOAT; }

[0-9]+(\.[0-9]+)? {
    yylval.numberLiteral = new NumberLiteral{
        std::atof(yytext),
        (strchr(yytext, '.') != nullptr)
            ? DataType::FLOAT
            : DataType::INT
    };
    return NUMBER;
}

"WHILE"                     { return WHILE; }
"IF"                        { return IF; }
"ELSE"                      { return ELSE; }

[a-zA-Z_][a-zA-Z0-9_]*      {
                                yylval.identifier = new std::string(yytext);
                                return IDENTIFIER;
                            }

"=="                        { return EQ; }
"!="                        { return NE; }
"<="                        { return LE; }
">="                        { return GE; }
"<"                         { return LT; }
">"                         { return GT; }
"="                         { return '='; }
"+"                         { return '+'; }
"-"                         { return '-'; }
"*"                         { return '*'; }
"/"                         { return '/'; }
"("                         { return '('; }
")"                         { return ')'; }
";"                         { return ';'; }

[ \t\r\n]+                  { /* Ignore whitespace */ }

.                           { return yytext[0]; }

%%
