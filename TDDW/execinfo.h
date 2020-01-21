#include "Windows.h"

#define backtrace(X,Y)                0
#define backtrace_symbols(X,Y)        0
#define signal(X,Y)                   
#define backtrace_symbols_fd     
#define feenableexcept
#define sleep(X)                      Sleep(X)
