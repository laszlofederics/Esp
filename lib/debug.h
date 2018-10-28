#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

extern "C"
{
  extern int ets_uart_printf(const char *fmt, ...);
}

#ifdef ENABLE_DEBUG
  #define debug ets_uart_printf
#else
  #define debug
#endif

#define print ets_uart_printf

#define printError ets_uart_printf

#endif
