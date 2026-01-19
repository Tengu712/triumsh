# Grammar

```ebnf
lf_or_eof = "\n" | "\0" ;

letter    = "a" | ... | "z" | "A" | ... | "Z" ;
digit     = "0" | ... | "9" ;
special   = " " | "\t" | "\n" | "'" | "\"" | "\\" | "$" ;
character = ? UTF-8 character except for special ?;

whitespace  = " " | "\t" ;
escaped     = "\\" , "'" | "\\" , "\"" | "\\" , "\\" ;

comment_content = character | ? special except for "\n" ?;
comment         = "#" , { comment_content } , lf_or_eof ;

variable  = ( letter | "_" ) , { letter | digit | "_" };
expansion = "$" , variable ;

single_quoted_content = ? UTF-8 character except for "'" ?;
single_quoted         = "'" , { single_quote_content } , "'" ;

double_quoted_content = character | whitespace | expansion | escaped | "'" | "\n" ;
double_quoted         = "\"" , { double_quoted_content } , "\"" ;

token_part = single_quoted | double_quoted | expansion | escaped | character ;
token      = { token_part };

token_sep = [ "\n" ] , whitespace ;
cmdline   = token , { token_sep , [ token ] } , lf_or_eof ;

top_level = lf_or_eof | comment | cmdline ;
trish     = { top_level };
```
