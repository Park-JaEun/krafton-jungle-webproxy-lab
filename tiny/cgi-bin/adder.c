/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include  "csapp.h"

int main(void) {
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1 = 0, n2 = 0;

    /* QUERY_STRING에서 두 개의 인자(num1과 num2)를 추출 */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
        /* "num1="으로 시작하는 부분 찾기 */
        p = strstr(buf, "num1=");
        if (p != NULL) {
            /* "num1=" 다음 값을 arg1에 복사 */
            sscanf(p, "num1=%d", &n1);
        }

        /* "num2="으로 시작하는 부분 찾기 */
        p = strstr(buf, "num2=");
        if (p != NULL) {
            /* "num2=" 다음 값을 arg2에 복사 */
            sscanf(p, "num2=%d", &n2);
        }
    }

    /* 응답 내용 작성 */
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>",
            content, n1, n2, n1 + n2);

    /* HTTP 응답 생성 */
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);

    fflush(stdout);
    exit(0);
}
/* $end adder */
