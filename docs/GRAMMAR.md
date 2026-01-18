# Grammar

```ebnf
lf_or_eof = "\n" | "\0" ;

special   = " " | "\t" | "\n" | "'" | "\"" ;
character = ? UTF-8 character except for special ?;

escaped_single_quote = "\\" , "'" ;
escaped_double_quote = "\\" , "\"" ;
escaped              = escaped_single_quote | escaped_double_quote ;

whitespace  = " " | "\t" ;
whitespaces = { whitespace };

comment_content = character | ? special except for "\n" ?;
comment         = "#" , { comment_content } , lf_or_eof ;

token_sep_unit = [ "\n" ] , whitespaces ;
token_sep      = token_sep_unit , { token_sep_unit };

single_quoted_content = character | escaped_single_quote | ? special except for "'" ?;
single_quoted         = "'" , { single_quote_content } , "'" ;

double_quoted_content = character | escaped_double_quote | ? special except for "\"" ?;
double_quoted         = "\"" , { double_quoted_content } , "\"" ;

token_part = single_quoted | double_quoted | character | escaped ;
token      = { token_part };

cmdline = token , { token_sep , [ token ] } , lf_or_eof ;

top_level = lf_or_eof | comment | cmdline ;
trish     = { top_level };
```
