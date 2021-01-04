#ifndef DEF_CLIENT
#define DEF_CLIENT

void ConnectedToServer(int);
void clearBuffer();
int promptString(char *, int);
int promptInt(char *, int, int, int);

void reserveTicket(int, stream_t *, char *, char *);
void cancelTicket(int, stream_t *, char *, char *);
int promptConfirmation(const char *, ...);

#endif
