/* $begin tinymain */
/*
 * tiny.c - GET 메서드를 사용하여 정적 및 동적 콘텐츠를 제공하는
 *          간단한 HTTP/1.0 웹 서버
 */
#include "csapp.h"

/* 함수 선언 */
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, 
                 char *shortmsg, char *longmsg);

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* 명령줄 인자 확인 */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    /* 지정포트로 서버 소켓 열고 연결 대기 */
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);

        /* 클라이언트 연결 요청 수락 */
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);

        /* 클라이언트 요청 처리 */
        doit(connfd);                                             //line:netp:tiny:doit

        /* 연결 종료 */
        Close(connfd);                                            //line:netp:tiny:close
    }
}

/*
 * doit - HTTP 요청/응답 트랜잭션 처리
 */
void doit(int fd) 
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* 요청 라인과 헤더를 읽음 */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE)) 
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);       

    /* 지원안하는 메서드이면 오류 반환 */
    if (strcasecmp(method, "GET")) {                     
        clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }                                                  
    read_requesthdrs(&rio);                             

    /* URI에서 파일이름과 CGI인자 추출 */
    is_static = parse_uri(uri, filename, cgiargs);     
    if (stat(filename, &sbuf) < 0) {                     
        clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn't find this file");
        return;
    }                                                   

    if (is_static) { /* 정적 */          
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) { 
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);       
    }
    else { /* 동적 */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { 
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);      
    }
}

/*
 * read_requesthdrs - HTTP 요청 헤더를 읽고 무시함
 */
void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    /* 빈 줄이 나올 때까지 헤더를 읽고 출력 */
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {         
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

/*
 * parse_uri - URI를 파일 이름과 CGI 인자로 구문 분석
 *             동적 0, 정적 1 반환
 */
int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    char *ptr;

    /* 정적 */
    if (!strstr(uri, "cgi-bin")) {
        strcpy(cgiargs, "");                   
        strcpy(filename, ".");                  
        strcat(filename, uri);                  
        if (uri[strlen(uri)-1] == '/')         
            strcat(filename, "home.html");   
        return 1;
    }
    /* 동적 */
    else {                                              
        ptr = index(uri, '?');                       
        if (ptr) {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }
        else 
            strcpy(cgiargs, "");                    
        strcpy(filename, ".");                       
        strcat(filename, uri);                     
        return 0;
    }
}

/*
 * serve_static - 파일을 클라이언트에게 전송하여 정적 콘텐츠 제공
 */
void serve_static(int fd, char *filename, int filesize) 
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
 
    /* 클라이언트에게 응답 헤더 전송 */
    get_filetype(filename, filetype);       
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));       
    printf("Response headers:\n");
    printf("%s", buf);

    /* 파일을 메모리에 매핑하여 클라이언트에게 전송 */
    srcfd = Open(filename, O_RDONLY, 0);    
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);                        
    Rio_writen(fd, srcp, filesize);        
    Munmap(srcp, filesize);                
}

/*
 * get_filetype - 파일 이름으로부터 파일 타입을 결정
 */
void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}  

/*
 * serve_dynamic - CGI 프로그램을 실행하여 동적 콘텐츠 제공
 */
void serve_dynamic(int fd, char *filename, char *cgiargs) 
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* HTTP 응답 헤더의 첫 부분 전송 */
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); 
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
  
    if (Fork() == 0) { /* 자식 프로세스 생성 */ 
        /* 실제 서버는 여기서 모든 CGI 환경 변수 설정 */
        setenv("QUERY_STRING", cgiargs, 1); 
        Dup2(fd, STDOUT_FILENO);              /* 클라이언트로 표준 출력을 리다이렉트 */ 
        Execve(filename, emptylist, environ); /* CGI 프로그램 실행 */ 
    }
    Wait(NULL); /* 부모 프로세스는 자식 프로세스 종료 대기 */ 
}

/*
 * clienterror - 클라이언트에게 오류 메시지 반환
 */
void clienterror(int fd, char *cause, char *errnum, 
                 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* HTTP 응답 본문 생성 */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* HTTP 응답 전송 */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
