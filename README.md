# ConfParser
Conf parser is a basic object oriented interpreter used
to create object oriented scripts for other softwares.
The aim is not to create entire softwares but to allows
a lua-type scripting for other softwares but with some
advanced concepts like inheritance, overloading, operator
management, preprocessing etc...

## ConfParser capabilities :
Object oriented programming
* Intrinsic types as objects and not rvalues
    * All in-code usable objects like intrinsic types or operators
      are threated as standard types and objets which allows them
      to have the same properties as user-created objects
* Types extension
    * Class declaration can be used to create or extend a type
* Type-specific operators creation and overloading
    * Operators are threated as class methods and have standard
      functions properties but with additional data (priority, 
      type)
* Operators priority management with volatile priority rules
    * No imposed priority rules, any operator can have any
      priority and any non-alphanumeric expression can be
      considered as operator
* Intrinsic lambda linking as language functions
    * Intrinsic functions are executed through lambda without hard
      coding constraints
* Scoped block-lifetime instances
    * Instanced are freed at the end of their declaration scope. Scopes
      are everything which can contains declaration from types to global
      scope including functions
* Scope inheritance including classes and files
    * Inheritance is a scope property and not only a type property. Any
      scope can inherit from another scope including functions or simple
      unamed scopes
* Basic preprocessor directives to manipulate interpreter
    * Preprocessor commands to allow file inclusion and interpreter
      setting
* Rvalues managment with anti-wast pattern
    * RValues are threated as expression-temporary variables and instantly
      cleaned up unlinke other variables

## ConfParser specs & capabities :
* Modern C++17 with advanced moving semantic
* Fully modulable intrinsic architecture
* UNICODE compatible
* Safe-Free pattern
* Dependance free interface
* CLI Interface

## Basic example:
(file types.conf)
```
# This is a custom type
class MyType {
    int aVar = 0
    string anotherVar
}
```

(file script.conf)
```
%use "types.conf"

MyType myVar
myVar.aVar = 5*(4+9)
myVar.anotherVar = "Hello World"
```

## Coming soon :
This project is in slow developpement cycles !
* Pre,Pos,Surrounding operators support
* Possibility to declare/overload functions and operators in-code (intrinsic functions & operators already supported)
* Create inheritence in-code (intrinsic inheritence already implemented for object type)
* Modular rvalue types with (eventually) litterals overloading
* Standard library
* Operators parser optimization
* Documentation
* Better CLI interface support
* Better memory management