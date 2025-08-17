package logging

import (
	"fmt"
	"log"
	"os"
	"sync"
	"time"
)

// LogLevel represents the severity of a log message
type LogLevel int

const (
	DebugLevel LogLevel = iota
	InfoLevel
	WarningLevel
	ErrorLevel
)

func (l LogLevel) String() string {
	switch l {
	case DebugLevel:
		return "DEBUG"
	case InfoLevel:
		return "INFO"
	case WarningLevel:
		return "WARNING"
	case ErrorLevel:
		return "ERROR"
	default:
		return "UNKNOWN"
	}
}

// Logger interface for unified logging
type Logger interface {
	Debug(msg string)
	Info(msg string)
	Warning(msg string)
	Error(msg string)
	SetLevel(level LogLevel)
	GetLevel() LogLevel
}

// StandardLogger implements Logger interface with standard output
type StandardLogger struct {
	level  LogLevel
	logger *log.Logger
	mutex  sync.RWMutex
}

// NewStandardLogger creates a new standard logger
func NewStandardLogger(level LogLevel) *StandardLogger {
	return &StandardLogger{
		level:  level,
		logger: log.New(os.Stdout, "", 0), // We'll handle our own formatting
	}
}

func (l *StandardLogger) shouldLog(level LogLevel) bool {
	l.mutex.RLock()
	defer l.mutex.RUnlock()
	return level >= l.level
}

func (l *StandardLogger) log(level LogLevel, msg string) {
	if !l.shouldLog(level) {
		return
	}
	
	timestamp := time.Now().Format("2006-01-02 15:04:05.000")
	formatted := fmt.Sprintf("%s [%s] %s", timestamp, level.String(), msg)
	l.logger.Println(formatted)
}

func (l *StandardLogger) Debug(msg string) {
	l.log(DebugLevel, msg)
}

func (l *StandardLogger) Info(msg string) {
	l.log(InfoLevel, msg)
}

func (l *StandardLogger) Warning(msg string) {
	l.log(WarningLevel, msg)
}

func (l *StandardLogger) Error(msg string) {
	l.log(ErrorLevel, msg)
}

func (l *StandardLogger) SetLevel(level LogLevel) {
	l.mutex.Lock()
	defer l.mutex.Unlock()
	l.level = level
}

func (l *StandardLogger) GetLevel() LogLevel {
	l.mutex.RLock()
	defer l.mutex.RUnlock()
	return l.level
}

// Global logger instance
var globalLogger Logger = NewStandardLogger(InfoLevel)
var globalMutex sync.RWMutex

// SetGlobalLogger sets the global logger instance
func SetGlobalLogger(logger Logger) {
	globalMutex.Lock()
	defer globalMutex.Unlock()
	globalLogger = logger
}

// GetGlobalLogger returns the global logger instance
func GetGlobalLogger() Logger {
	globalMutex.RLock()
	defer globalMutex.RUnlock()
	return globalLogger
}

// SetGlobalLogLevel sets the global log level
func SetGlobalLogLevel(level LogLevel) {
	GetGlobalLogger().SetLevel(level)
}

// Convenience functions using the global logger
func Debug(msg string) {
	GetGlobalLogger().Debug(msg)
}

func Info(msg string) {
	GetGlobalLogger().Info(msg)
}

func Warning(msg string) {
	GetGlobalLogger().Warning(msg)
}

func Error(msg string) {
	GetGlobalLogger().Error(msg)
}