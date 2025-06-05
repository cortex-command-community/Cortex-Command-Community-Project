-- Logger.lua - Conditional logging system for Grapple debugging

local Logger = {}

-- Global debug flag - set this to true/false to enable/disable all logging
Logger.debugEnabled = false  -- Change to false to disable all print statements

-- Different log levels
Logger.LOG_LEVELS = {
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4
}

-- Current log level (only logs at or above this level will be printed)
Logger.currentLogLevel = Logger.LOG_LEVELS.DEBUG

-- Main logging function
function Logger.log(level, message, ...)
    if not Logger.debugEnabled then
        return
    end
    
    if level < Logger.currentLogLevel then
        return
    end
    
    local levelNames = {"DEBUG", "INFO", "WARN", "ERROR"}
    local levelName = levelNames[level] or "UNKNOWN"
    
    -- Format the message with any additional arguments
    local formattedMessage = string.format(message, ...)
    
    -- Print with level prefix
    print("[" .. levelName .. "] " .. formattedMessage)
end

-- Convenience functions for different log levels
function Logger.debug(message, ...)
    Logger.log(Logger.LOG_LEVELS.DEBUG, message, ...)
end

function Logger.info(message, ...)
    Logger.log(Logger.LOG_LEVELS.INFO, message, ...)
end

function Logger.warn(message, ...)
    Logger.log(Logger.LOG_LEVELS.WARN, message, ...)
end

function Logger.error(message, ...)
    Logger.log(Logger.LOG_LEVELS.ERROR, message, ...)
end

-- Simple boolean check function (like your original request)
function Logger.conditionalPrint(condition, message, ...)
    if condition then
        local formattedMessage = string.format(message, ...)
        print(formattedMessage)
    end
end

return Logger