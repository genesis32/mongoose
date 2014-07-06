#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>

static const char *LOGFILE="app.log";

class Logger 
{
private:
  Logger();
  ~Logger();

  void VarPrint(const char *cat, const char *str);

  FILE *fp;
  bool printToStdout;
public:
  static Logger& InstanceOf() { static Logger logger; return logger; }
  
  void Info(const char *s, ...);
  void Warning(const char *s, ...);
  void Error(const char *s, ...);

  void SetPrintToStdout(bool print) { printToStdout = print; }
};

Logger::Logger() : fp(NULL), printToStdout(false) {
  fp = fopen(LOGFILE, "w");
}
 
Logger::~Logger() {
  if(fp) {
    fclose(fp);
  }
}
 
void Logger::VarPrint(const char *cat, const char *str) 
{
  fprintf(fp, "%s: %s\n", cat, str);
  fflush(fp);

  if(printToStdout) 
    {
      fprintf(stdout, "%s: %s\n", cat, str);
      fflush(stdout);
    }
}

void Logger::Info(const char *s, ...) 
{
  char buff[32768];
  va_list argp;
  va_start(argp, s);
  vsprintf(buff, s, argp);
  va_end(argp);

  VarPrint("info", buff);
}

void Logger::Warning(const char *s, ...) 
{
  char buff[32768];
  va_list argp;
  va_start(argp, s);
  vsprintf(buff, s, argp);
  va_end(argp);

  VarPrint("warning", buff);
}

void Logger::Error(const char *s, ...) 
{
  char buff[32768];
  va_list argp;
  va_start(argp, s);
  vsprintf(buff, s, argp);
  va_end(argp);

  VarPrint("error", buff);
}


#endif
