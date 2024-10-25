/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
    char *query_string = getenv("QUERY_STRING"); // URL에서 쿼리 스트링 가져오기
    int a, b;

    /* 스트링 파싱 -> 정수로 분리 */
    if (query_string == NULL || sscanf(query_string, "%d&%d", &a, &b) != 2) {
        printf("Content-type: text/html\r\n\r\n");
        printf("<html><body>");
        printf("<p>Error: Please provide two integers in the form /adder?a&b</p>");
        printf("</body></html>");
        return 1;
    }

    /* HTTP 응답 헤더 + HTML 응답 본문 출력 */
    printf("Content-type: text/html\r\n\r\n");
    printf("<html><body>");
    printf("<h1>%d + %d = %d</h1>", a, b, a + b);
    printf("</body></html>");

    return 0;
}
/* $end adder */
