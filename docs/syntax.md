```
program         ::= {functionDefinition | lambdaDefinition}
functionDefinition ::= "func" "(" [arguments] ")" blockStmt
lambdaDefinition  ::= "lambda" "(" [arguments] ")" "->" "(" [arguments] ")" blockStmt
arguments        ::= identifier {"," identifier}
blockStmt        ::= "{" {statement} "}"
statement        ::= letStmt
                    | whileStmt
                    | forStmt
                    | rangeBasedForStmt
                    | ifStmt
                    | ifElseStmt
                    | returnStmt
                    | continueStmt
                    | breakStmt
                    | tryCatchStmt
                    | throwStmt
                    | {expression ";"}
letStmt          ::= "let" letAssignmentPair {"," letAssignmentPair}
letAssignmentPair ::= identifier "=" expression
whileStmt        ::= "while" "(" expression ")" blockStmt
forStmt          ::= "for" "(" expression ";" expression ";" expression ")" blockStmt
rangeBasedForStmt ::= "forEach" "(" identifier "in" expression ")" blockStmt
ifStmt           ::= "if" "(" expression ")" blockStmt
ifElseStmt       ::= "if" "(" expression ")" blockStmt "else" blockStmt
returnStmt       ::= "return" expression
continueStmt     ::= "continue"
breakStmt        ::= "break"
tryCatchStmt     ::= "try" blockStmt "catch" "as" identifier blockStmt
throwStmt        ::= "throw" expression
expression       ::= uniqueExpression
                    | multiplicationExpression
                    | additionExpression
                    | binaryShiftExpression
                    | logicEqualExpression
                    | binaryExpression
                    | logicAndExpression
                    | assignmentExpression
uniqueExpression     ::= ["-" | "--" | "++" | "!"] primary
multiplicationExpression ::= primary {("*" | "/" | "%") primary}
additionExpression    ::= primary {("+" | "-") primary}
binaryShiftExpression ::= primary {("<<" | ">>") primary}
logicEqualExpression  ::= primary {("==" | "!=" | ">=" | "<=" | ">" | "<") primary}
binaryExpression      ::= primary {("|" | "&" | "^") primary}
logicAndExpression    ::= primary {("&&" | "||") primary}
assignmentExpression  ::= primary {("=" | "+=" | "-=" | "*=" | "/=" | "%=") primary}
primary              ::= basicLiterals
                        | identifier
                        | subscriptExpression
                        | invokingExpression
                        | listLiteral
                        | objectLiteral
                        | memberExpression
                        | lambdaDefinition
                        | "(" expression ")"
subscriptExpression  ::= primary "[" expression "]"
invokingExpression   ::= primary "(" [arguments] ")"
listLiteral          ::= "[" [memberPair ::= identifier ":" expression
objectLiteral        ::= "{" [memberPair {"," memberPair}] "}"
memberExpression     ::= primary "." identifier
```