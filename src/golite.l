%{
    #include <stdio.h>
    #include <string.h>
    #include "y.tab.h"
    #include <ctype.h> 

    #define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;

    #pragma GCC diagnostic ignored "-Wunused-function"

    extern int print_tokens;

    #define RETURN(ret_token) last_token = ret_token; return ret_token;
    void illegal_int(char* input)
    {
        fprintf(stderr, "Illegal integer %s \n", input);
        exit(1);
    }

    int oct_to_int(char* input)
    {
        int value = 0;
        int temp = 0;
        char* buffer = input;
        while(*buffer != '\0'){
            temp = *buffer - '0';
            if(temp>=0 && temp <=7){
                value = (value << 3) + temp;
            } else {
                illegal_int(input);
            }
            buffer++;
        }
        return value;
    }
    
    int hex_to_int(char* input)
    {
        int value = 0;
        int temp_num = 0, temp_letter = 0;
        char* buffer = strdup(input+2);
        int i =0;
        for(i = 0; buffer[i] != '\0'; i++) {
            buffer[i] = tolower(buffer[i]);
            temp_num = buffer[i] - '0';
            temp_letter = buffer[i] - 'a';
            if(temp_num >=0 && temp_num <=9){
                value = (value << 4) + temp_num;
            } else if (temp_letter>=0 && temp_letter <=5) {
                value = (value << 4) + temp_letter + 10;
            } else {
                illegal_int(input);
            }
        } 
        return value;
    }

    void weed_string(char* string){
        while(*string != '\0')
        {
            if(*string == '\\')
            {
                string++;
                switch(*string)
                {
                    case 'a':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                    case 'v':
                    case '\\':
                    case '"':
                        break;
                    default:
                        printf("Error: [line %d] escape character %c not allowed.\n", yylineno, *string);
                        exit(1);
                }
            }
            string++;
        }

    }

    char* convert_rawtostring(char* raw_string)
    {
        int required_length = 0;
        char* raw_ptr = raw_string;
        while(*raw_ptr)
        {
            required_length++;
            if(*raw_ptr == '\\' || *raw_ptr == '"')
                required_length++;
            raw_ptr++;
        }
        char* converted_str = (char*) malloc(sizeof(char)*required_length);
        
        raw_ptr = raw_string;
        char* conv_ptr = converted_str;
        
        while(*raw_ptr){
            
            if(*raw_ptr == '\\' || *raw_ptr == '"'){
                *conv_ptr = '\\';
                conv_ptr++;
            }
            
            *conv_ptr = *raw_ptr;
            raw_ptr++;
            conv_ptr++;
        }
        free(raw_string);
        *conv_ptr = '\0';
        return converted_str;
    }

    int last_token = 0;

%}


%option yylineno

%%
[ \r\t]+

[\n]  { 
         switch(last_token)
         {
             case tIDENTIFIER:
             case tINTVAL:
             case tFLOATVAL:
             case tCSTRING:
             case tRUNEVAL:
             case tBREAK:
             case tCONTINUE:
             case tFALLTHROUGH:
             case tRETURN:
             case tINCREMENT:
             case tDECREMENT:
             case ')':
             case ']':
             case '}':
                if(print_tokens) printf("tSEMICOLON \n");
                RETURN(';');
             default : ;
          }
             
      }

 /*Operators definition*/ 
"+"   { if(print_tokens) printf("tADD \n"); RETURN('+');}
"-"   { if(print_tokens) printf("tMINUS \n"); RETURN('-');}
"*"   { if(print_tokens) printf("tMULTIPLY \n"); RETURN('*');}
"/"   { if(print_tokens) printf("tDIVIDE \n"); RETURN('/');}
"="   { if(print_tokens) printf("tASSIGN \n"); RETURN('=');}
"("   { if(print_tokens) printf("tLEFTPAR \n"); RETURN('(');}
")"   { if(print_tokens) printf("tRIGHTPAR \n"); RETURN(')');}
":"   { if(print_tokens) printf("tCOLON \n"); RETURN(':');}
";"   { if(print_tokens) printf("tSEMICOLON \n"); RETURN(';');}
"{"   { if(print_tokens) printf("tLEFTBRACKET \n"); RETURN('{');}
"}"   { if(print_tokens) printf("tRIGHTBRACKET \n"); RETURN('}');}
"!"   { if(print_tokens) printf("tLOGICALNOT \n"); RETURN('!');}
"["   { if(print_tokens) printf("tLEFTSQUAREBRACKET \n"); RETURN('[');}
"]"   { if(print_tokens) printf("tRIGHTSQAUREBRACKET \n"); RETURN(']');}
"&"   { if(print_tokens) printf("tLOGICALAND \n"); RETURN('&');}
"|"   { if(print_tokens) printf("tLOGICALOR \n"); RETURN('|');}
"."   { if(print_tokens) printf("tDOT \n"); RETURN('.');}
"^"   { if(print_tokens) printf("tXOR \n"); RETURN('^');}
"%"   { if(print_tokens) printf("tMODULO \n"); RETURN('%');}
","   { if(print_tokens) printf("tCOMMA \n"); RETURN(',');}
":="  { if(print_tokens) printf("tSHORTDCL \n"); RETURN(tSHORTDCL);}

 /*Comparators */
">"   { if(print_tokens) printf("tGREATERTHAN \n"); RETURN(tGT);}
"<"   { if(print_tokens) printf("tLESSTHAN \n"); RETURN(tLT);}
"<="  { if(print_tokens) printf("tLESSEQUALS \n"); RETURN(tLTE);}
">="  { if(print_tokens) printf("tGREATEREQUALS \n"); RETURN(tGTE);}
"=="  { if(print_tokens) printf("tEQUAL \n"); RETURN(tEQ);}
"!="  { if(print_tokens) printf("tNOTEQUAL \n"); RETURN(tNEQ);}

"&&"  { if(print_tokens) printf("tBOOLAND \n"); RETURN(tBOOLAND);}
"||"  { if(print_tokens) printf("tBOOLOR \n"); RETURN(tBOOLOR);}
"<<"  { if(print_tokens) printf("tLEFTSHIFT \n"); RETURN(tLSHIFT);}
">>"  { if(print_tokens) printf("tRIGHTSHIFT \n"); RETURN(tRSHIFT);}
"<-"  { if(print_tokens) printf("tBACKARROW \n"); RETURN(tBACKARROW);}
"++"  { if(print_tokens) printf("tINCREMENT \n"); RETURN(tINCREMENT);}
"--"  { if(print_tokens) printf("tDECREMENT \n"); RETURN(tDECREMENT);}
"..."  { if(print_tokens) printf("tELIPSIS \n"); RETURN(tELLIPSIS);}
"&^"  { if(print_tokens) printf("tBITCLEAR \n"); RETURN(tBITCLEAR);}

 /*Operator + Assign*/
"+="  { if(print_tokens) printf("tPLUSEQUAL \n"); RETURN(tPLUSEQUAL);}
"-="  { if(print_tokens) printf("tMINUSEQUAL \n"); RETURN(tMINUSEQUAL);}
"*="  { if(print_tokens) printf("tMULEQUAL \n"); RETURN(tMULEQUAL);}
"/="  { if(print_tokens) printf("tDIVEQUAL \n"); RETURN(tDIVEQUAL);}
"%="  { if(print_tokens) printf("tMODEQUAL \n"); RETURN(tMODEQUAL);}
"<<="  { if(print_tokens) printf("tLEFTSHIFTEQUAL \n"); RETURN(tLSHIFTEQUAL);}
">>="  { if(print_tokens) printf("tRIGHTSHIFTEQUAL \n"); RETURN(tRSHIFTEQUAL);}
"&="  { if(print_tokens) printf("tANDEQUAL \n"); RETURN(tANDEQUAL);}
"|="  { if(print_tokens) printf("tOREQUAL \n"); RETURN(tOREQUAL);}
"^="  { if(print_tokens) printf("tOREQUAL \n"); RETURN(tNOTEQUAL);}
"&^="  { if(print_tokens) printf("tBITCLEAREQUAL \n"); RETURN(tBITCLEAREQUAL);}

 /*Keywords*/
"var"  { if(print_tokens) printf("tVAR \n"); RETURN(tVAR); }
"if"     { if(print_tokens) printf("tIF \n"); RETURN(tIF);}
"else"   { if(print_tokens) printf("tELSE \n"); RETURN(tELSE);}
"for"  { if(print_tokens) printf("tFOR \n"); RETURN(tFOR);}
"func"  { if(print_tokens) printf("tFUNC \n"); RETURN(tFUNC);}
"break"  { if(print_tokens) printf("tBREAK \n"); RETURN(tBREAK);}
"default"  { if(print_tokens) printf("tDEFAULT \n"); RETURN(tDEFAULT);}
"interface"  { if(print_tokens) printf("tINTERFACE \n"); RETURN(tINTERFACE);}
"select"  { if(print_tokens) printf("tSELECT \n"); RETURN(tSELECT);}
"case"  { if(print_tokens) printf("tCASE \n"); RETURN(tCASE);}
"defer"  { if(print_tokens) printf("tDEFER \n"); RETURN(tDEFER);}
"go"  { if(print_tokens) printf("tGO \n"); RETURN(tGO);}
"map"  { if(print_tokens) printf("tMAP \n"); RETURN(tMAP);}
"struct"  { if(print_tokens) printf("tSTRUCT \n"); RETURN(tSTRUCT);}
"chan"  { if(print_tokens) printf("tCHAN \n"); RETURN(tCHAN);}
"goto"  { if(print_tokens) printf("tGOTO \n"); RETURN(tGOTO);}
"package"  { if(print_tokens) printf("tPACKAGE \n"); RETURN(tPACKAGE);}
"switch"  { if(print_tokens) printf("tSWITCH \n"); RETURN(tSWITCH);}
"const"  { if(print_tokens) printf("tCONST \n"); RETURN(tCONST);}
"fallthrough"  { if(print_tokens) printf("tFALLTHROUGH \n"); RETURN(tFALLTHROUGH);}
"range"  { if(print_tokens) printf("tRANGE \n"); RETURN(tRANGE);}
"type"  { if(print_tokens) printf("tTYPE \n"); RETURN(tTYPE);}
"continue"  { if(print_tokens) printf("tCONTINUE \n"); RETURN(tCONTINUE);}
"return"  { if(print_tokens) printf("tRETURN \n"); RETURN(tRETURN);}
"println"  { if(print_tokens) printf("tPRINTLN \n"); RETURN(tPRINTLN);}
"print"  { if(print_tokens) printf("tPRINT \n"); RETURN(tPRINT);}
"append"  { if(print_tokens) printf("tAPPEND \n"); RETURN(tAPPEND);}
"len"  { if(print_tokens) printf("tLEN \n"); RETURN(tLEN);}
"cap"  { if(print_tokens) printf("tCAP \n"); RETURN(tCAP);}
 /*Import handled */
"import"  { if(print_tokens) printf("tIMPORT \n"); RETURN(tIMPORT);}



 /* Handling comments*/
\/\/[^\n]*
\/\/[^\n]*/\n

\/\*(([^\*\n])|("\*"[^\/\n]))*\*\/

\/\*(([^\*])|("\*"[^\/]))*\*\/   {
                                        switch(last_token)
                                        {
                                            case tIDENTIFIER:
                                            case tINTVAL:
                                            case tFLOATVAL:
                                            case tCSTRING:
                                            case tRUNEVAL:
                                            case tBREAK:
                                            case tCONTINUE:
                                            case tFALLTHROUGH:
                                            case tRETURN:
                                            case tINCREMENT:
                                            case tDECREMENT:
                                            case ')':
                                            case ']':
                                            case '}':
                                                if(print_tokens) printf("tSEMICOLON \n");
                                                RETURN(';');
                                            default : ;
                                        }
                    
                                     }
 

 /* Handling Floats*/
[0-9]*\.[0-9]*  { if(print_tokens) printf("tFLOATVAL(%s) \n", yytext);
                              yylval.floatval = atof(yytext); 
                              RETURN(tFLOATVAL);
                             }

 /* Handling Ints */
0|[1-9]([0-9])* { 
                  if(print_tokens) printf("tINTVAL(%s) \n", yytext);
                  yylval.intval = atoi(yytext);
                  RETURN(tINTVAL);
                }

0([0-7]([0-7])*) {
                  if(print_tokens) printf("tINTVAL(%s) \n", yytext);
                  yylval.intval = oct_to_int(yytext);
                  RETURN(tINTVAL);
                 }

0([x]|[X])([0-9a-fA-F][0-9a-fA-F]*) { if(print_tokens) printf("tINTVAL(%s) \n", yytext);
                               yylval.intval = hex_to_int(yytext);
                               RETURN(tINTVAL);
                             }


[0-9]* { fprintf( stderr , "Error: [line %d] invalid integer %s \n", yylineno, yytext);
         exit(1);
       }

[a-zA-Z_][a-zA-Z0-9_]* { if(print_tokens) printf("tIDENTIFIER(%s) \n", yytext);
                         yylval.identifier = strdup(yytext);
                         RETURN(tIDENTIFIER);
                       }

 
 /* Handling Strings  */
(["]([^\n"]|([\\][\"]))*["])   { 
                                    if(print_tokens) printf("tCONSTSRING(%s) \n", yytext);
                                    yylval.const_string = strndup(yytext+1, strlen(yytext)-2);
                                    weed_string(yylval.const_string);
                                    RETURN(tCSTRING);
                                    }

([`]([^`])*[`])  { if(print_tokens) printf("tRAWSTRING(%s) \n", yytext);
                     char* raw_string= strndup(yytext+1, strlen(yytext)-2);
                     yylval.const_string = convert_rawtostring(raw_string);
                     RETURN(tCSTRING);
                   }


 /*Rune characters | ([\][abfnrtv\'])  */
([']([^'\n\\]|([\\][abfnrtv\\']))[']) { if(print_tokens) printf("tRUNEVAL(%s) \n", yytext);
                                        char* temp= strndup(yytext+1, strlen(yytext)-2);
                                        yylval.chrval = temp;
                                        RETURN(tRUNEVAL);
                                    }

<<EOF>> {
    switch(last_token)
    {
        case tIDENTIFIER:
        case tINTVAL:
        case tFLOATVAL:
        case tCSTRING:
        case tRUNEVAL:
        case tBREAK:
        case tCONTINUE:
        case tFALLTHROUGH:
        case tRETURN:
        case tINCREMENT:
        case tDECREMENT:
        case ')':
        case ']':
        case '}':
            if(print_tokens) printf("tSEMICOLON \n");
            RETURN(';');
        default : ;
    }
    yyterminate();

}
.     { fprintf(stderr, "Error: Invalid token at line %d, %s is not a valid \n", yylineno, yytext);
        exit(1);
      } 

%%
