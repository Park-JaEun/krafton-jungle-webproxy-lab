Tiny Web server
Dave O'Hallaron
Carnegie Mellon University

This is the home directory for the Tiny server, a 200-line Web
server that we use in "15-213: Intro to Computer Systems" at Carnegie
Mellon University.  Tiny uses the GET method to serve static content
(text, HTML, GIF, and JPG files) out of ./ and to serve dynamic
content by running CGI programs out of ./cgi-bin. The default 
page is home.html (rather than index.html) so that we can view
the contents of the directory from a browser.

Tiny is neither secure nor complete, but it gives students an
idea of how a real Web server works. Use for instructional purposes only.

The code compiles and runs cleanly using gcc 2.95.3 
on a Linux 2.2.20 kernel.

To install Tiny:
   Type "tar xvf tiny.tar" in a clean directory. 

To run Tiny:
   Run "tiny <port>" on the server machine, 
	e.g., "tiny 8000".
   Point your browser at Tiny: 
	static content: http://<host>:8000
	dynamic content: http://<host>:8000/cgi-bin/adder?1&2

Files:
  tiny.tar		Archive of everything in this directory
  tiny.c		The Tiny server
  Makefile		Makefile for tiny.c
  home.html		Test HTML page
  godzilla.gif		Image embedded in home.html
  README		This file	
  cgi-bin/adder.c	CGI program that adds two numbers
  cgi-bin/Makefile	Makefile for adder.c

### `adder.c` 코드 설명

1. **쿼리 스트링 가져오기**:
    - `getenv("QUERY_STRING")`를 통해 URL에 포함된 쿼리 스트링(`?1&2`)을 가져온다.
2. **파라미터 파싱**:
    - `sscanf` 함수로 `query_string`에서 두 숫자를 추출하여 각각 `a`와 `b`에 저장한다.
    - 파라미터가 잘못되었거나 누락된 경우, 오류 메시지를 HTML 형식으로 출력한다.
3. **결과 반환**:
    - 올바르게 파싱된 경우, `a + b`의 결과를 HTML로 반환하여 클라이언트에 전송한다.

### CSAPP 11.6-c 문제 
#### tiny.c의 doit 함수에서 HTTP 버전을 확인하고 출력하여 서버 응답의 버전을 클라이언트가 요청한 HTTP 버전으로 맞추도록 변경

출력 예시 : Client HTTP version: HTTP/1.1

1. **HTTP 버전 파싱**: 
   - doit 함수에서 요청 라인의 세 번째 값으로 version 변수를 파싱하여 HTTP 버전을 저장한다.
printf("Client HTTP version: %s\n", version);로 클라이언트가 사용하는 HTTP 버전을 출력한다.
2. **버전에 따른 응답**:
   - 응답을 생성할 때 HTTP/1.0과 같은 고정 값을 사용하는 대신, version 변수를 사용하여 클라이언트의 HTTP 버전에 맞춘다. 이를 통해 클라이언트의 HTTP 버전 요구에 맞는 응답을 제공한다.
3. **함수 시그니처 수정**:
   - serve_static, serve_dynamic, clienterror 함수에 version 매개변수를 추가하여 각 함수가 클라이언트의 HTTP 버전 정보를 사용할 수 있게 한다.

### CSAPP 11.7 문제
#### TINY를 확장해서 MPG 비디오 파일을 처리하도록 하시오. 실제 브라우저를 사용해서 여러분의 결과를 체크하시오.

1. **MIME 타입 추가**: 
   - get_filetype 함수에서 MPG 파일 확장자를 확인하고 맞는 MIME 타입(video/mpeg)을 설정하도록 한다.

### CSAPP 11.9 문제
#### tiny.c를 수정해서 정적 컨텐츠를 처리할 때 요청한 파일을 mmap과 rio_readn 대신에 malloc, rio_readn, rio_writen을 사용해서 연결 식별자에게 복사하도록 하시오.

요청된 파일을 메모리에 직접 매핑하는 대신, 메모리를 동적으로 할당한 후 rio_readn으로 파일 내용을 읽고 rio_writen으로 클라이언트에게 전달한다.

1. **동적 메모리 할당**:
malloc을 사용하여 요청된 파일 크기(filesize)만큼 메모리를 동적으로 할당한다.
메모리 할당에 실패한 경우 clienterror 함수를 호출하여 클라이언트에게 500 오류 메시지를 반환하고, 함수 실행을 종료한다.

2. **파일 내용 읽기**:
Rio_readn을 사용하여 열린 파일(srcfd)에서 할당한 메모리(srcp)로 파일의 내용을 읽어온다.
mmap을 사용하지 않고 파일 내용을 메모리로 읽어오기 때문에, 메모리 복사가 필요한 경우 이 방식이 유용하다.

3. **파일 내용 전송 및 메모리 해제**:
Rio_writen을 사용하여 할당된 메모리에 저장된 파일 내용을 클라이언트에게 전송한다.
전송이 완료되면 free를 호출하여 동적으로 할당된 메모리를 해제한다.

### CSAPP 11.10 문제
#### 브라우저에서 숫자 2개를 입력하면 이를 get 메소드를 사용해서 컨텐츠를 요청하도록 하고 채운 형식을 tiny에 보내고, adder가 생성한 동적 컨텐츠를 표시하는 방법으로 작업을 체크하라.

1. QUERY_STRING 파싱
getenv("QUERY_STRING")를 통해 QUERY_STRING 환경 변수를 가져온다. num1=<첫번째 숫자>&num2=<두번째 숫자> 형식으로 되어 있다.
strchr(buf, '&')로 & 기호를 찾아서 두 개의 인자를 나누고, strcpy로 각 인자를 arg1과 arg2에 복사한다.
atoi를 사용해 arg1과 arg2를 정수로 변환하고 각각 n1과 n2에 저장한다.

2. 응답 콘텐츠 작성
sprintf를 사용해 클라이언트에 보낼 HTML 콘텐츠를 작성한다. 여기서 두 숫자의 합을 계산하여 출력 문자열을 구성한다.

3. HTTP 응답 출력
HTTP 응답 헤더와 콘텐츠를 printf를 사용해 출력한다. Content-length와 Content-type 헤더를 설정하고, Content-type: text/html로 HTML 형식임을 지정한다.
마지막에 fflush(stdout);을 사용해 버퍼를 비운 후, exit(0);으로 프로그램을 종료한다.

### CSAPP 11.10 문제
#### http head 메소드를 지원하도록 하라. telnet을 웹 클라이언트로 사용해서 작업 결과를 테크하시오.

1. doit 함수에서 HEAD 메서드 처리:
HEAD 메서드를 감지하고, is_head 플래그를 설정하여 GET과 구분.
is_head 플래그는 serve_static과 serve_dynamic 함수로 전달하여 콘텐츠 본문 전송 여부를 결정.

2. serve_static 함수 수정:
is_head가 설정된 경우, 응답 헤더만 전송하고 파일 내용은 전송하지 않음.

3. serve_dynamic 함수 수정:
is_head가 설정된 경우, CGI 프로그램을 실행하지 않고 응답 헤더만 전송.