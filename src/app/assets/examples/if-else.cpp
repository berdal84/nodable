
// Conditional structures example

// note: this example is interesting to see nested conditional structures.

int a = 5;
int b = 4;
string msg;
if ( a > b ) {
    msg = "a > b";
} else if ( a < b ) {
    msg = "a < b";
} else {
    msg = "a == b";
}
return(msg);