
// Multiple instructions example

// note: This example is interesting to see how multiple instructions can be interconnected.
//       You will see you can select the last line and see its graph, but you won't be able to
//       compile it since f and g are not declared in this context.

int f = pow(2, 2) - 1;
int g = sqrt(64) + 3;
int h = f + 7 * g;
