//
// Created by me on 19.12.19.
//

//#ifndef MARK_STRING_H
//#define MARK_STRING_H
class String;
#define breakpoint_helper printf("\n%s:%d breakpoint_helper\n",__FILE__,__LINE__);
typedef const char *chars;

typedef const char *chars;
extern void err(chars error);
extern void error(chars error);
extern void warn(chars error);
extern void warning(chars error);
extern chars fetch(chars url);
