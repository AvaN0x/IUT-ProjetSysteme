#ifndef DEF_CLIENT
#define DEF_CLIENT

void connectedToServer(int);
void clearBuffer();
int promptString(char *, int);
int promptInt(char *, int, int, int);

void reserveTicket(int, stream_t *, char *, char *);
void cancelTicket(int, stream_t *, char *, char *);
void adminPanel(int, stream_t *, char *, char *, bool *);
int promptConfirmation(const char *, ...);

#endif
