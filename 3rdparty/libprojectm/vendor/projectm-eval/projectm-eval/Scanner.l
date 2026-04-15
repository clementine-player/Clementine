%option reentrant
%option nounput
%option noinput
%option noyywrap
%option 8bit
%option yylineno
%option bison-bridge bison-locations
%option prefix="prjm_eval_"
%option extra-type="struct prjm_eval_scanner_state *"
%option never-interactive

%top{
#include "CompilerTypes.h"
#include "Compiler.h"

#include <stdio.h>
#include <stdint.h>

#define YY_NO_UNISTD_H
#define YY_STDINIT
#define YY_USER_ACTION                           \
    {                                            \
        int old_column = yycolumn;               \
        yycolumn += strlen(yytext);              \
        yylloc_param->first_line = yylineno;     \
        yylloc_param->first_column = old_column; \
        yylloc_param->last_line = yylineno;      \
        yylloc_param->last_column = yycolumn;    \
    }

struct prjm_eval_scanner_state;
}


    /* States */
%x ML_COMMENT
%x IL_COMMENT

    /* Rule definitions */
DIGIT           [0-9]
HEX             [0-9a-fA-F]
SIGN            [+-]
EXPO            [eE]{SIGN}?{DIGIT}+
FLOAT           ({DIGIT}+|{DIGIT}+\.{DIGIT}*|{DIGIT}*\.{DIGIT}+){EXPO}?
INT             {DIGIT}+
NAME            [_a-zA-Z][_a-zA-Z0-9]*

%%

    /* Comments */
"/*"                { BEGIN(ML_COMMENT); }
<ML_COMMENT>"*/"    { BEGIN(INITIAL); }
<ML_COMMENT>\n      { }

"//"                { BEGIN(IL_COMMENT); }
<IL_COMMENT>\n      { BEGIN(INITIAL); }
<IL_COMMENT>.       { }

    /* Constants */
"$"[Xx]{HEX}+       {
                        char* p = yytext + 2;
                        yylval->NUM = (PRJM_EVAL_F)strtoul(yytext + 2, &p, 16);
                        return NUM;
                        }
"$'"."'"            { yylval->NUM = *(char*)(yytext + 2); return NUM; }
"$"(?i:pi)          { yylval->NUM = 3.141592653589793f; return NUM; }
"$"[eE]             { yylval->NUM = 2.71828183f; return NUM; }
"$"(?i:phi)         { yylval->NUM = 1.61803399f; return NUM; }

(?i:gmem)           { return GMEM; }

{FLOAT}             {
                        yylval->NUM = atof(yytext);
                        return NUM;
                    }
{INT}               {
                        yylval->NUM = (PRJM_EVAL_F)atoi(yytext);
                        return NUM;
                    }
{NAME}              {
                        if (prjm_eval_compiler_name_is_function(cctx, yytext))
                        {
                            yylval->FUNC = strdup(yytext);
                            return FUNC;
                        }
                        yylval->VAR = strdup(yytext);
                        return VAR;
                    }


    /* Operators */
"+="                { return ADDOP; }
"-="                { return SUBOP; }
"%="                { return MODOP; }
"|="                { return OROP; }
"&="                { return ANDOP; }
"/="                { return DIVOP; }
"*="                { return MULOP; }
"^="                { return POWOP; }
"=="                { return EQUAL; }
"<="                { return BELEQ; }
">="                { return ABOEQ; }
"!="                { return NOTEQ; }
"||"                { return BOOLOR; }
"&&"                { return BOOLAND; }
"<"	                { return '<'; }
">"	                { return '>'; }
"+"	                { return '+'; }
"-"	                { return '-'; }
"*"	                { return '*'; }
"/"                 { return '/'; }
"%"                 { return '%'; }
"^"	                { return '^'; }
"&"                 { return '&'; }
"|"	                { return '|'; }
"!"	                { return '!'; }
"="	                { return '='; }

    /* Syntactic elements */
"("                 { return '('; }
")"                 { return ')'; }
"["                 { return '['; }
"]"                 { return ']'; }
"?"                 { return '?'; }
":"                 { return ':'; }
","                 { return ','; }

    /* Expression terminator */
";"			        { return ';'; }

    /* Ignored whitespace */
[ \r\n\t\v\f]	    { }

    /* All other tokens are invalid */
.                   { return PRJM_EVAL_UNDEF; }

%%
