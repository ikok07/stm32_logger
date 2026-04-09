//
// Created by Kok on 1/29/26.
//

#include "log.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"

static LOGGER_TypeDef gHLogger;

uint8_t default_formatter(LOGGER_EventTypeDef *Event, char *Buffer, uint16_t Len);

/**
 * @brief Links required logger callback functions
 * @note This method MUST be called before any other!
 * @param Callbacks Logger callback functions
 */
void LOGGER_RegisterCB(LOGGER_CallbacksTypeDef *Callbacks) {
    gHLogger.Callbacks = *Callbacks;
}

/**
 * @brief This method should setup basic peripherals that are most likely to not fail during initialization.
 *        This ensures that there will be some sign of error even if more complex peripherals are still not running.
 */
void LOGGER_InitBasic() {
    gHLogger.Initialized = 1;
    if (gHLogger.Callbacks.on_init_basic != NULL) {
        gHLogger.Callbacks.on_init_basic();
    }
}

/**
 * @brief This method calls ONLY the LOGGER_LogBasicCB() which should never return error.
 *        This ensures that there will be some sign of error even if more complex peripherals are still not running.
 */
void LOGGER_LogBasic() {
    if (gHLogger.Callbacks.on_log_basic != NULL) {
        gHLogger.Callbacks.on_log_basic();
    }
}

/**
 * @brief This method initializes the logger with all the required peripherals even if they can return error.
 * @note LOGGER_LogInitBasic() is called inside this method
 */
LOGGER_ErrorTypeDef LOGGER_Init() {
    gHLogger.Initialized = 1;
    gHLogger.Enabled = 0;
    gHLogger.FatalOccurred = 0;

    LOGGER_InitBasic();

    if (gHLogger.Callbacks.on_init != NULL && gHLogger.Callbacks.on_init() != 0) return LOGGER_ERROR_IMPLEMENTATION;
    return LOGGER_ERROR_OK;
}

LOGGER_ErrorTypeDef LOGGER_DeInit() {
    gHLogger = (LOGGER_TypeDef) {0};
    if (gHLogger.Callbacks.on_de_init != NULL && gHLogger.Callbacks.on_de_init() != 0) return LOGGER_ERROR_IMPLEMENTATION;
    return LOGGER_ERROR_OK;
}

LOGGER_ErrorTypeDef LOGGER_ReInit() {
    LOGGER_ErrorTypeDef error = LOGGER_DeInit();
    if (error != LOGGER_ERROR_OK) return error;
    return LOGGER_Init();
}

LOGGER_ErrorTypeDef LOGGER_Log(LOGGER_LevelTypeDef Level, char *Msg) {
    if (!gHLogger.Initialized) return LOGGER_ERROR_UNINITIALIZED;
    if (!gHLogger.Enabled) return LOGGER_ERROR_DISABLED;

    if (gHLogger.ActiveLevel > Level) {
        // Logger's active level is higher than the provided one.
        return LOGGER_ERROR_OK;
    }

    if (strlen(Msg) > LOGGER_MSG_MAX_LEN) return LOGGER_ERROR_MESSAGE_LEN;

    char formatted_msg[LOGGER_MSG_MAX_LEN];
    LOGGER_EventTypeDef event = {
        .Level = Level,
        .msg = Msg
    };
    if (gHLogger.Callbacks.optional_on_format == NULL) {
        if (default_formatter(&event, formatted_msg, sizeof(formatted_msg)) != 0) return LOGGER_ERROR_IMPLEMENTATION;
    } else {
        if (gHLogger.Callbacks.optional_on_format(&event, formatted_msg, sizeof(formatted_msg)) != 0) return LOGGER_ERROR_IMPLEMENTATION;
    }

    event.msg = formatted_msg;

    if (Level == LOGGER_LEVEL_FATAL) {
        if (gHLogger.Callbacks.on_fatal_err != NULL && gHLogger.Callbacks.on_fatal_err(&event) != 0) return LOGGER_ERROR_IMPLEMENTATION;
    } else {
        if (gHLogger.Callbacks.on_log != NULL && gHLogger.Callbacks.on_log(&event) != 0) return LOGGER_ERROR_IMPLEMENTATION;
    }

    return LOGGER_ERROR_OK;
}

LOGGER_ErrorTypeDef LOGGER_LogF(LOGGER_LevelTypeDef Level, char *Fmt, ...) {
    char buffer[LOGGER_MSG_MAX_LEN];
    va_list args;

    va_start(args, Fmt);
    vsnprintf(buffer, sizeof(buffer), Fmt, args);
    va_end(args);

    return LOGGER_Log(Level, buffer);
}

LOGGER_ErrorTypeDef LOGGER_Enable() {
    if (!gHLogger.Initialized) return LOGGER_ERROR_UNINITIALIZED;
    gHLogger.Enabled = 1;
    return LOGGER_ERROR_OK;
}

LOGGER_ErrorTypeDef LOGGER_Disable() {
    if (!gHLogger.Initialized) return LOGGER_ERROR_UNINITIALIZED;
    gHLogger.Enabled = 0;
    return LOGGER_ERROR_OK;
}

/**
 * @brief Configures the level of the messages which the logger will output
 * @param Level Logger level
 */
LOGGER_ErrorTypeDef LOGGER_SetLevel(LOGGER_LevelTypeDef Level) {
    if (!gHLogger.Initialized) return LOGGER_ERROR_UNINITIALIZED;
    gHLogger.ActiveLevel = Level;
    return LOGGER_ERROR_OK;
}

char *LOGGER_GetLevelLabel(LOGGER_LevelTypeDef Level) {
    switch (Level) {
        case LOGGER_LEVEL_DEBUG: return "DEBUG";
        case LOGGER_LEVEL_INFO: return "INFO";
        case LOGGER_LEVEL_WARNING: return "WARNING";
        case LOGGER_LEVEL_ERROR: return "ERROR";
        case LOGGER_LEVEL_FATAL: return "FATAL";
        default: return "UNKNOWN";
    };
}

uint8_t default_formatter(LOGGER_EventTypeDef *Event, char *Buffer, uint16_t Len) {
    return snprintf(Buffer, Len, "%s - %s\n", LOGGER_GetLevelLabel(Event->Level), Event->msg) >= Len;
}