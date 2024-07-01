/*
proxy_parse.h -- a HTTP Request Parsing library.
written by: Matvey Arye
for: COS 518


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <ctype.h>

//ifndef PROXY_PARSE
#define PROXY_PARSE

#define DEBUG 1 

/*
PARSEDREQUEST objects are created from parsing a buffer containing a HTTP
request. The request buffer consists of a request line followed by a number
of headers. Request line fields such as method, protocol etc. are stored
explicitly. Headers such as "Content-Length" and their values are maintaining 
in a linked list. Each node in this is a ParsedHeader and contains 
key-value pair.

The buf and buflen fields are used internally to maintain the parsed required 
line.
*/

struct ParsedReqest {
    char *method;
    char *protocol;
    char *host;
    char *port;
    char *path;
    char *version;
    char *buf;
    size_t buflen;
    struct ParsedHeader *headers;
    size_t headersused;
    size_t headerslen;
    
    
};

/*
    PARSEDHEADER: any header after request line is a key-value pairwith the 
    format "key:value\r\n" and is maintained in the PARSEDHEADER linked list
    within ParsedRequest
*/

struct ParsedHeader {

};