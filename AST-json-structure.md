## Module
```json
{
    "type": "Module",
    "statements": [] // These can be any node specified below
}
```


## CodeBlock
```json
{
    "type": "CodeBlock",
    "statements": [] // These can be any node specified below
}
```

## Function
```json
{
    "type": "Function",
    "parameters": ["DataType", "DataType", ...],
    "ret-type": "DataType",
    "code-block": {"type": "CodeBlock", ...}
}
```

## IfBlock
```json
{
    "type": "IfBlock",
    "statements": [
        {
            "condition": "...",
            "code-block": {"type": "CodeBlock", ...},
        }
    ],
    "default": {"type": "CodeBlock", ...} 
}
```

## Loop
```json
{
    "type": "Loop",
    "condition": "...",
    "code-block": {"type": "CodeBlock", ...} 
}
```


## DeclarationStatement
```json
{
    "type": "DeclarationStatement",
    "dst": "some variable",
    "src": "...",
    "dtype": "DataType"
}
```

## AssignmentStatement
```json
{
    "type": "AssignmentStatement",
    "dst": "some variable",
    "src": "..."
}
```

## ReturnStatement
```json
{
    "type": "ReturnStatement",
    "value": "...",
    "dtype": "DataType"
}
```

## Literal
```json
{
    "type": "Literal",
    "value": "...",
    "dtype": "DataType"
}
```

## Variable
```json
{
    "type": "Variable",
    "name": "...",
    "dtype": "DataType"
}
```

## Expression
```json
{
    "type": "Expression",
    "left-operand": "...",
    "operator": "...",
    "right-operand": "...",
    "dtype": "DataType"
}
```

## FunctionCall
```json
{
    "type": "FunctionCall",
    "function-name": "...",
    "parameters": ["..."],
    "dtype": "DataType"
}
```