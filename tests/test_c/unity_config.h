// unity_config.h - положите в папку с тестами
#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

// ВКЛЮЧИТЬ поддержку double
#define UNITY_EXCLUDE_DOUBLE 0
#define UNITY_INCLUDE_DOUBLE 1

// Также включите float, если нужно
#define UNITY_EXCLUDE_FLOAT 0

// Если нужна поддержка 64-битных целых
#define UNITY_EXCLUDE_64BIT 0

// Дополнительные настройки
#define UNITY_INCLUDE_CONFIG_H  // Важно: сообщите Unity использовать этот файл

#endif
