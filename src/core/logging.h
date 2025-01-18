#ifndef LOGGING_H
#define LOGGING_H

/* sets up logging to the specified file */
void log_start(const char *file);

/* clears the log file */
void log_clear(void);

/* logs a debugging message to the global log file */
#define LOG(fmt, ...) { internal_log(__FILE__, __LINE__, fmt, ##__VA_ARGS__); }

void internal_log(const char *file, const int line, const char *format, ...);


#endif //LOGGING_H
