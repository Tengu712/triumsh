# Grammar

```ebnf
lf_or_eof = "\n" | "\0" ;

letter     = "a" | ... | "z" | "A" | ... | "Z" ;
digit      = "0" | ... | "9" ;
special    = " " | "\t" | "\n" | "'" | "\"" | "\\" | "$" | "(" | ")" ;
whitespace = " " | "\t" ;
escaped    = "\\" , "'"
           | "\\" , "\""
           | "\\" , "\\"
           | "\\" , "$"
           | "\\" , "("
           | "\\" , ")" ;
character  = ? UTF-8 character except for special ?;

comment_content = character | ? special except for "\n" ?;
comment         = "#" , { comment_content } , lf_or_eof ;

expansion_variable = ( letter | "_" ) , { letter | digit | "_" };
expansion_command  = "(" , cmdline , ")" ;
expansion          = "$" , ( expansion_variable | expansion_command );

single_quoted_content = character | escaped | ? special except for "'" ?;
single_quoted         = "'" , { single_quote_content } , "'" ;

double_quoted_content = character | whitespace | escaped | expansion | "\n" | "'" | "(" | ")" ;
double_quoted         = "\"" , { double_quoted_content } , "\"" ;

token = { character | escaped | single_quoted | double_quoted | expansion };

cmdline   = token , { "\n" , whitespace | whitespace | token } , lf_or_eof ;

top_level = lf_or_eof | comment | cmdline ;
trish     = { top_level };
```
