#ifndef _RETURN_H_
#define _RETURN_H_

#define SUCCESS 0
#define INVALID_PARAMETER -1
#define INCORRECT_PARAMETER -2
#define MALLOC_FAILURE -3

#define QUEUE_FULL -4
#define QUEUE_EMPTY -5

#define INNER_FUNC_ERROR -6
#define THREAD_CREATE_ERROR -7
#define EVENT_CREATE_FAILED -8

#define PIPE_CREATE_FAILED -9
#define PIPE_WRITE_FAILED -10
#define PIPE_READ_FAILED -11
#define CONNECTION_FAILED -12

#define SUCCESSFUL_CONNECTION 100
#define MAX_CONNECTION_COUNT_REACHED 101

#define INVALID_USERNAME 201
#define INVALID_PASSWORD 202
#define PASSWORD_TOO_WEAK 203
#define REGISTER_ALREADY_EXISTS 204
#define INVALID_USERNAME_PASSWORD_COMBINATION 205
#define USERNAME_ALREADY_CONNECTED 206
#define USER_NOT_CONNECTED 207

#define FILE_CREATE_FAILED 301
#define FILE_WRITE_FAILED 302

#endif _RETURN_H_