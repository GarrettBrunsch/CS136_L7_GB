*  **feedback.md is for instructor use only. DO NOT change the feedback.md**; make a copy if needed
* class definition style & standards:  first, member vars  none per line and do not forget to initialize them;  second functions: separate group of functions  by a blank line; either matching setter/getter pairs or list all setters back to back and list all getters back to back; start with a default c’tor, followed by  copy c’tor (if any), then other c’tors in the order of increased number of arguments, d’tor- right after c’tor(s), overloaded functions- list them back to back in the order of increased number of arguments;  all accessor/getter functions should be const; all functions with more than one statement should be normal (defined externally, outside of the class and below main()), no inline functions with more than one statement; initialize all member variables to appropriate default values at the time of definition; all member variables must be private; classes must provide a strong exception guarantee; must have default c’tor; implement Rule of Three when necessary
* 324   HashTable temp(other);  goes away at 333 along with  memory it allocated  -5
* 311, 316 and like:  why rethrow and give control away?   Might prematurely terminate c’tor and as a result, an object will  not be created, you’ll have the id for an object but not the object -5
* poor Id(s)  and/or inconsistent naming convention; ids should be self-documenting and as short as possible; use verbs for functions and nouns for variables; use camel-casing for variables (errorMessage) enum & const should be in upper case with words separated by “_”, e.g., MAX_TERMS, PRINT_RECORDS; pointer variables should start with p( if single)  or pp( if double pointer); flags isValid ( clearly what true would mean); if copying – e.g rhsQueue or scrQueue ; do not use IDs that are same as or very similar to C++  keywords and functions; no need to call an array “array”- it is obvious -2
```text
void HashTable::swapBoolPointers(bool*& a, bool*& b)
```
* 357 and like: what if numbers change? if it is not 1 or 0 make it const; in case of ASCII values, use chars, e.g., temp>’a’  -5
* 254 if you use delete on something that was not dynamically allocated, a compiler might throw an exception; check first, then use delete; new does not return nullptr if fails- it throws bad_alloc exception -2


