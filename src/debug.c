#include <julia.h>

int main()
{
    jl_init();
    jl_eval_string("include(\"juliawm.jl\")");
    jl_atexit_hook(0);
}
