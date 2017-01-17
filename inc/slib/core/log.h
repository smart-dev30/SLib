#ifndef CHECKHEADER_SLIB_CORE_LOG
#define CHECKHEADER_SLIB_CORE_LOG

#include "definition.h"

#include "string.h"
#include "object.h"
#include "list.h"
#include "variant.h"

SLIB_NAMESPACE_BEGIN

class LoggerSet;

class SLIB_EXPORT Console
{
public:
	static void print(const String& s);
	
	template <class... ARGS>
	void print(const String& format, ARGS&&... args)
	{
		String content = String::format(format, args...);
		print(content);
	}

	static void println(const String& s);
	
	template <class... ARGS>
	void println(const String& format, ARGS&&... args)
	{
		String content = String::format(format, args...);
		println(content);
	}

	static String readLine();
	
};

class SLIB_EXPORT Logger : public Object
{
public:
	virtual void log(const String& tag, const String& content) = 0;
	
	virtual void logError(const String& tag, const String& content);

public:
	static Ref<LoggerSet> global();
	
	static Ref<Logger> getConsoleLogger();
	
	static Ref<Logger> createFileLogger(const String& fileName);
	
	static void logGlobal(const String& tag, const String& content);
	
	static void logGlobalError(const String& tag, const String& content);

};

class SLIB_EXPORT FileLogger : public Logger
{
public:
	FileLogger();
	
	FileLogger(const String& fileName);

public:
	// override
	void log(const String& tag, const String& content);

public:
	SLIB_PROPERTY(AtomicString, FileName)

};

class SLIB_EXPORT LoggerSet : public Logger
{
public:
	LoggerSet();
	
	LoggerSet(const Ref<Logger>& logger, const Ref<Logger>& errorLogger);
	
public:
	void clearDefaultLogger();
	
	void addDefaultLogger(const Ref<Logger>& logger);
	
	void removeDefaultLogger(const Ref<Logger>& logger);
	
	
	void clearErrorLogger();
	
	void addErrorLogger(const Ref<Logger>& logger);
	
	void removeErrorLogger(const Ref<Logger>& logger);
	
public:
	// override
	void log(const String& tag, const String& content);
	
	// override
	void logError(const String& tag, const String& content);
	
protected:
	CList< Ref<Logger> > m_listLoggers;
	CList< Ref<Logger> > m_listErrorLoggers;

};

template <class... ARGS>
void Log(const String& tag, const String& format, ARGS&&... args)
{
	String content = String::format(format, args...);
	Logger::logGlobal(tag, content);
}

template <class... ARGS>
void LogError(const String& tag, const String& format, ARGS&&... args)
{
	String content = String::format(format, args...);
	Logger::logGlobalError(tag, content);
}

SLIB_NAMESPACE_END

#endif
