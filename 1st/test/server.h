
#define BACKLOG 10
/* Make true/false word also represent 1/0 */
#define true 1
#define false 0

/* Static sizes for arrays */
#define HEADER_BUFFER_SIZE 256
#define MAX_SOCKET_READ_SIZE 256
#define MAX_HEADER_SIZE 1024
#define MAX_REQUEST_PATH_SIZE 1000
#define MAX_BUFFER_SIZE 4 * 1024 

/* Name numeric return codes */
#define FILE_OK 0
#define FILE_UNKNOWN_ERR -10
#define FILE_NOT_EXISTS -1
#define FILE_PERMISSION_DENIED -2

/* Some globally used MIME types */
#define FILE_TYPE_PLAIN "text/plain"
#define FILE_TYPE_HTML "text/html"
#define FILE_TYPE_GIF "image/gif"
#define FILE_TYPE_JPEG "image/jpeg"
#define FILE_TYPE_MP3 "audio/mpeg"
#define FILE_TYPE_PDF "application/pdf"

/* Basic HTTP Status Codes */
#define HTTP_OK_CODE 200
#define HTTP_NOT_FOUND_ERR_CODE 404
#define HTTP_SERVER_ERR_CODE 500

/* Functions defined in func.c file */
char* itoa(int i);
int mywrite(int fd, char* str);
int write_header(int fd, char* key, char* value);
int write_status(int fd, int status_code);
int write_file(int fd, int read_fd, char path[]);
