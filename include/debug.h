#pragma once

// #define DEBUG

#ifdef DEBUG
#define DEBUG_PRINTF(...)       \
  do                            \
  {                             \
    Serial.printf(__VA_ARGS__); \
  } while (0)
#define DEBUG_PRINTLN(...)       \
  do                             \
  {                              \
    Serial.println(__VA_ARGS__); \
  } while (0)
#else
#define DEBUG_PRINTF(...) \
  do                      \
  {                       \
  } while (0)
#define DEBUG_PRINTLN(...) \
  do                  \
  {                   \
  } while (0)
#endif