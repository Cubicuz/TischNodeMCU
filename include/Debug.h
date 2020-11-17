#ifndef DEBUG_HEADER_GUARD
#define DEBUG_HEADER_GUARD

#define debugHomeassistant
#ifdef debugHomeassistant
#define debugPrintf(...) Serial.printf(__VA_ARGS__)
#define debugPrintLn(x) Serial.println(x)
#else
#define debugPrintf(...)
#define debugPrintLn(x)
#endif

#endif // DEBUG_HEADER_GUARD
