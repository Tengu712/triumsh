# Grammar

```ebnf
lf_or_eof = "\n" | "\0" ;

special    = " " | "\t" | "\n" | "'" | "\"" | "\\" | "$" | "{" | "}" | "(" | ")" | ">" | "|" ;
whitespace = " " | "\t" ;
escaped    = "\\" , "'"
           | "\\" , "\""
           | "\\" , "\\"
           | "\\" , "$"
           | "\\" , "{"
           | "\\" , "}"
           | "\\" , "("
           | "\\" , ")"
           | "\\" , ">"
           | "\\" , "|" ;
character  = ? UTF-8 character except for special ?;

simple_variable = { character | escaped };
variable        = { character | escaped | whitespace };

comment_content = ? UTF-8 character except for lf_or_eof ?;
comment         = "#" , { comment_content } , lf_or_eof ;

expansion_variable = "{" , variable , "}" | simple_variable ;
expansion_command  = "(" , cmdline , ")" ;
expansion          = "$" , ( expansion_variable | expansion_command );

single_quoted_content = ? UTF-8 character except for non-escaped "'" and "\0" ?;
single_quoted         = "'" , { single_quote_content } , "'" ;

double_quoted_content = character | whitespace | escaped | expansion
                      | "\n" | "'" | "{" | "}" | "(" | ")" | ">" | "|" ;
double_quoted         = "\"" , { double_quoted_content } , "\"" ;

token = { character | escaped | single_quoted | double_quoted | expansion };

redirect  = ">" , [ whitespace ] , token ;
pipe      = "|" , [ whitespace ] , cmdline ;
cmdline   = token , { "\n" , whitespace | whitespace | token } , [ redirect | pipe | lf_or_eof ] ;

top_level = lf_or_eof | comment | cmdline ;
trish     = { top_level };
```
