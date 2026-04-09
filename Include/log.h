//
// Created by Kok on 1/29/26.
//

#ifndef LOG_H
#define LOG_H

#include <stdint.h>

#define __weak __attribute__((weak))

#define LOGGER_MSG_MAX_LEN          255

typedef enum {
    LOGGER_LEVEL_DEBUG,
    LOGGER_LEVEL_INFO,
    LOGGER_LEVEL_WARNING,
    LOGGER_LEVEL_ERROR,
    LOGGER_LEVEL_FATAL,
} LOGGER_LevelTypeDef;

typedef enum {
    LOGGER_ERROR_OK,
    LOGGER_ERROR_IMPLEMENTATION,    // An error occurred in the system-specific implementation
    LOGGER_ERROR_UNINITIALIZED,
    LOGGER_ERROR_MESSAGE_LEN,        // The passed message is longer than the allowed maximum (LOGGER_MSG_MAX_LEN)
    LOGGER_ERROR_DISABLED
} LOGGER_ErrorTypeDef;

typedef struct {
    LOGGER_LevelTypeDef Level;
    char *msg;
} LOGGER_EventTypeDef;

typedef struct {
    /**
     * @brief Setup peripherals required for logger to have basic functionality (LED, GPIOs, etc.)
     */
    void (*on_init_basic)();

    /**
     * @brief Log basic information using peripherals configured with LOGGER_InitBasicCB()
     */
    void (*on_log_basic)();

    /**
     * @brief Setup peripherals required for logger to function properly. This method is called inside LOGGER_Init()
     * @return 0 -> OK\n 1 -> ERROR
     */
    uint8_t (*on_init)();

    /**
     * @brief Clear resources assigned while using the logger
     * @return 0 -> OK\n 1 -> ERROR
     */
    uint8_t (*on_de_init)();

    /**
     * @brief Use the configured peripherals to output log message
     * @param Event Logger event
     * @return 0 -> OK\n 1 -> ERROR
     */
    uint8_t (*on_log)(LOGGER_EventTypeDef *Event);

    /**
     * @brief Handle cases where the error is fatal. This method is called when LOGGER_Log() is called with fatal log.
     * @param Event Logger event
     * @return 0 -> OK\n 1 -> ERROR
     */
    uint8_t (*on_fatal_err)(LOGGER_EventTypeDef *Event);

    /**
     * @brief Formats the logged messages.
     * @note This method is not required, there is a default function already defined
     * @param Event Logger event
     * @param Buffer Error message
     * @param Len Length of error message
     */
    uint8_t (*optional_on_format)(LOGGER_EventTypeDef *Event, char *Buffer, uint16_t Len);
} LOGGER_CallbacksTypeDef;

typedef struct {
    uint8_t Initialized;            // Used to track if the API has been initialized
    uint8_t Enabled;
    uint8_t FatalOccurred;
    LOGGER_LevelTypeDef ActiveLevel;
    LOGGER_CallbacksTypeDef Callbacks;
} LOGGER_TypeDef;

/* ------ Main methods ------ */

void LOGGER_RegisterCB(LOGGER_CallbacksTypeDef *Callbacks);
void LOGGER_InitBasic();
void LOGGER_LogBasic();

LOGGER_ErrorTypeDef LOGGER_Init();
LOGGER_ErrorTypeDef LOGGER_DeInit();
LOGGER_ErrorTypeDef LOGGER_ReInit();
LOGGER_ErrorTypeDef LOGGER_Log(LOGGER_LevelTypeDef Level, char *Msg);
LOGGER_ErrorTypeDef LOGGER_LogF(LOGGER_LevelTypeDef Level, char *Fmt, ...);

/* ------ Controls ------ */

LOGGER_ErrorTypeDef LOGGER_Enable();
LOGGER_ErrorTypeDef LOGGER_Disable();
LOGGER_ErrorTypeDef LOGGER_SetLevel(LOGGER_LevelTypeDef Level);

/* ------ Utils ------ */

char *LOGGER_GetLevelLabel(LOGGER_LevelTypeDef Level);

#endif //LOG_H
