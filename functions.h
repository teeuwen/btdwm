#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

void focusmon(const Arg *arg);
void focusstack(const Arg *arg);
void setlayout(const Arg *arg);
void setmfact(const Arg *arg);
void tagmon(const Arg *arg);

void spawn(const Arg *arg);
void togglebar(const Arg *arg);

void toggletag(const Arg *arg);
void viewtag(const Arg *arg);

void killclient(const Arg *arg);
void moveclient(const Arg *arg);
void movemouse(const Arg *arg);
void resizemouse(const Arg *arg);
void togglefloating(const Arg *arg);

#endif
