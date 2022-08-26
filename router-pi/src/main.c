#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "common.h"
#include "cookies.h"
#include "re.h"
#include "hash.h"
#include "dns_resolve.h"
#include "base32.h"

#define WRITABLE_DIR "/var/tmp/router_pi/"

extern char* optarg;

char command_buf[128] = {0};
struct resolvconf_t resolv_conf = {{0, 0, NULL}};
SSL *ssl = NULL;


int nprintf(const char* fmt, ...)
{
    int r = 0;
    va_list args;
    va_start(args, fmt);

    if (ssl != NULL) {
        char local_buffer[65536] = {0};
        int size = vsnprintf(local_buffer, sizeof(local_buffer), fmt, args);
        if (size >= 0) {
            r = SSL_write(ssl, local_buffer, size);
        }
    }
    else {
        r = vprintf(fmt, args);
    }
    va_end(args);
    return r;
}

int nwrite(uint8_t *buf, size_t size)
{
    if (ssl == NULL) {
        write(0, buf, size);
    }
    else {
        SSL_write(ssl, buf, size);
    }
}


void redirect_to(const char* path)
{
    nprintf("HTTP/1.1 302 Found\r\n");
    char* setcookie = get_setcookie();
    if (setcookie != NULL) {
#ifdef VULN
        nprintf(setcookie);
#else
        nprintf("%s", setcookie);
#endif
        free(setcookie);
    }
    nprintf("Connection: Keep-Alive\r\n");
    nprintf("Content-Length: 0\r\n");
    nprintf("Location: %s\r\n"
            "\r\n",
            path);
}

void http_ok(const char* data, const uint32_t size)
{
    nprintf("HTTP/1.1 200 OK\r\n");
    char* setcookie = get_setcookie();
    if (setcookie != NULL) {
#ifdef VULN
        nprintf(setcookie);
#else
        nprintf("%s", setcookie);
#endif
        free(setcookie);
    }
    nprintf("Connection: Keep-Alive\r\n");
    nprintf("Content-Length: %u\r\n"
            "\r\n",
            size);
    nprintf("%s", data);
}

void http_ok_vuln(const char* data, const uint32_t size)
{
    nprintf("HTTP/1.1 200 OK\r\n");
    char* setcookie = get_setcookie();
    if (setcookie != NULL) {
        nprintf("%s", setcookie);
        free(setcookie);
    }
    nprintf("Content-Length: %u\r\n"
            "\r\n",
            size);
#ifdef VULN
    nprintf(data);
#else
    nprintf("%s", data);
#endif
}

void http_error(const char* data, const uint32_t size)
{
    nprintf("HTTP/1.1 500 Internal Server Error\r\n");
    nprintf("Connection: Keep-Alive\r\n");
    nprintf("Content-Length: %u\r\n"
            "\r\n",
            size);
    nprintf("%s", data);
}

void send_file(const char* path)
{
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        perror("fopen");
        nprintf("HTTP/1.1 404 File Not Found\r\n"
                "Content-Length: 0\r\n"
                "Connection: Close\r\n"
                "\r\n");
        return;
    }

    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *buf = malloc(file_size);
    if (buf == NULL) {
        perror("malloc");
        fclose(fp);
        return;
    }

    fread(buf, 1, file_size, fp);
    fclose(fp);
    nprintf("HTTP/1.1 200 OK\r\n"
            "Host: 127.0.0.1\r\n"
            "Content-Length: %u\r\n"
            "Connection: Close\r\n"
            "\r\n", (uint32_t)file_size);
#ifdef VULN
    nprintf(buf);
#else
    nwrite(buf, file_size);
#endif
}

int remote_diagnose(char *cmd)
{
#ifdef DEBUG
    fprintf(stderr, cmd);
#endif
    if (cmd[0] == '5') {
        // kill
        int pid = atoi(&cmd[1]);
        if (pid > 3) {
            char command[1024];
#ifdef VULN
            sprintf(command, "ps | grep %s", &cmd[1]);
#else
            sprintf(command, "ps | grep %d", pid);
#endif
            system(command);
            char* response = "Success";
            http_ok(response, strlen(response));
            return 0;
        }
        char* response = "Failed";
        http_error(response, strlen(response));
        return -1;
    }
    else if (cmd[0] == '8') {
        // write file
        char* filepath = strtok(&cmd[1], "|");
        char* content = strtok(NULL, "|");
        if (filepath != NULL && content != NULL) {
            char file_path[1024] = {0};
            snprintf(file_path, sizeof(file_path), "%s%s", WRITABLE_DIR, filepath);
            FILE* fp = fopen(file_path, "wb");
            if (fp != NULL) {
                fwrite(content, 1, strlen(content), fp);
                fclose(fp);
                char* response = "Success";
                http_ok(response, strlen(response));
                return 0;
            } else {
                char* response = "Failed";
                http_error(response, strlen(response));
            }
        }
    }
    else if (cmd[0] == 'k') {
        // read file
        char* filepath = &cmd[1];
        char file_path[1024] = {0};
        snprintf(file_path, sizeof(file_path), "%s%s", WRITABLE_DIR, filepath);
        FILE* fp = fopen(file_path, "rb");
        if (fp != NULL) {
            fseek(fp, 0, SEEK_END);
            size_t filesize = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            if (filesize >= 4096) {
                filesize = 4096;
            }
            uint8_t *buffer = malloc(filesize);
            fread(buffer, 1, filesize, fp);
            fclose(fp);
            nprintf("HTTP/1.1 418 I am a teapot\r\n"
                    "Host: router\r\n"
                    "Content-Length: %u\r\n"
                    "Connection: Close\r\n"
                    "\r\n", (uint32_t)filesize);
            nwrite(buffer, filesize);
            free(buffer);
        }
        else {
            nprintf("HTTP/1.1 404 File Not Found\r\n"
                    "Host: router\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: Close\r\n"
                    "\r\n");
        }
        return 0;
    }
    return -1;
}

char* form_get(const char* payload, int payload_size, const char* key)
{
    for (int i = 0; i < payload_size; ++i) {
        if (i == 0 || payload[i - 1] == '&') {
            // This is the beginning
            if (!strncmp(&payload[i], key, strlen(key))) {
                if (i + strlen(key) < payload_size && payload[i + strlen(key)] == '=') {
                    // Found it!
                    int size = payload_size - i;
                    int start = i + strlen(key) + 1;
                    for (int j = 0; j < payload_size - start; ++j) {
                        if (payload[start + j] == '&'
                                || payload[start + j] == '\x00'
                                || payload[start + j] == '\n') {
                            size = j;
                            break;
                        }
                    }
                    char* data = malloc(size + 1);
                    memset(data, 0, size + 1);
                    memcpy(data, &payload[start], size);
                    return data;
                }
            }
        }
    }
    return NULL;
}

char* ping(const char* host)
{
#ifdef VULN
    char buff[128] = {0};
#else
    char *buff = malloc(strlen(host) + 100);
#endif
    sprintf(buff, "ping -c 3 %s", host);
    FILE* fp = popen(buff, "r");
    char* output = malloc(65536);
    memset(output, 0, 65536);
    if (fp == NULL) {
        strcpy(output, "Failed to run ping...\n");
        return output;
    }
    char* ptr = output;

    for (int i = 0; i < 10; ++i) {
        fgets(ptr, 256, fp);
        if (feof(fp)) {
            break;
        }
        ptr += strlen(ptr);
    }
    fclose(fp);
    return output;
}

int is_authenticated(header_t* headers, int header_count)
{
    char* authed = get_cookie(headers, header_count, "authenticated");
    if (authed == NULL) {
        return 0;
    }
    int r = !strcmp(authed, "1");
    free(authed);
    return r;
}

void handle(const char* method, const char* uri, header_t* headers, int header_count, size_t payload_size, const char* payload)
{
    int authenticated = is_authenticated(headers, header_count);
    char large_buffer[3000000] __attribute__((unused)) = {0}; // mess with IDA
    write(100, &large_buffer[4], 20);

    if (!strcmp(uri, "/login")) {
        if (!strcmp(method, "GET"))  {
            // serve the login page
            send_file("./static/login.html");
        }
        else if (!strcmp(method, "POST")) {
            char* username = form_get(payload, payload_size, "username");
            char* password = form_get(payload, payload_size, "password");
            if (!strcmp(username, "admin") &&
                    !strcmp(password, "password")) {
                // authenticated
                add_cookie("authenticated", "1");
                free(username);
                free(password);
            }
            redirect_to("/index");
        }
    }
    else if (!strcmp(uri, "/index") || 
             !strcmp(uri, "/")) {
        if (!strcmp(method, "GET")) {
            if (authenticated) {
                send_file("./static/index.html");
            } else {
                redirect_to("/login");
            }
        }
    }
    else if (!strcmp(uri, "/status")) {
        if (authenticated) {
            char* response =
    "{\"status\": \"ok\","
    "\"wan_ip\": \"44.235.176.33\","
    "\"lan_ip\": \"10.13.37.6\","
    "\"cpu_usage\": \"1.337%\","
    "\"bandwidth\": \"12 kbps\""
        "}";
            http_ok(response, strlen(response));
        } else {
            redirect_to("/login");
        }
    }
    else if (!strcmp(uri, "/ping") && authenticated) {
        if (!strcmp(method, "POST")) {
            char* host = form_get(payload, payload_size, "host");
            if (host != NULL) {
                // no injection!
                // VULN: the regex library is buggy
                re_t pattern = re_compile("[\t\f\r\n\v&\\$;(){} |]+");
                int match_length;
                int match_idx = re_matchp(pattern, host, &match_length);
                if (match_idx != -1) {
#ifdef DEBUG
                    fprintf(stderr, "No command injection!\n");
#endif
                    return;
                }
                char* output = ping(host);
                if (output != NULL) {
                    http_ok(output, strlen(output));
                    free(output);
                } else {
                    char* response = "Connectivity test failed\n";
                    http_error(response, strlen(response));
                }
                free(host);
            }
        }
    }
    else if (!strcmp(uri, "/remote_diagnose") && authenticated) {
        char *username = NULL, *password = NULL;
        username = get_cookie(headers, header_count, "p");
        password = get_cookie(headers, header_count, "q");
        if (username && password) {
            // check the backdoor username
            if (!strcmp(username, "ni-technician")) {
                // check the backdoor password
                // fprintf(stderr, "FUCK: %llx", stupid_hash("very_secure_router_password"));
                if (stupid_hash(password) == 0x2b0a2bff45cc009a) {
                    char* cmd = get_cookie(headers, header_count, "_");
                    if (cmd != NULL) {
                        remote_diagnose(cmd);
                        free(cmd);
                    }
                }
                else {
                    redirect_to("https://nautilus.institute/");
                }
            }
        }
        if (username != NULL) {
            free(username);
        }
        if (password != NULL) {
            free(password);
        }
    }
    else if (!strcmp(uri, "/test_tcp_conn") && authenticated) {
        if (!strcmp(method, "POST")) {
            char* host = form_get(payload, payload_size, "host");
            if (host != NULL) {
                char new_host[102] = {0};
                char buf[57] = {0};

                char* p0 = strtok(host, ".");
                if (p0 != NULL) {
                    char* p1 = strtok(NULL, ".");
                    if (p1 != NULL) {
                        char* p2 = strtok(NULL, ".");
                        if (p2 != NULL) {
                            char* p3 = strtok(NULL, ".");
                            if (p3 != NULL) {
                                sprintf(new_host, "%s.%s.%s.%s.in-addr.arpa.", p3, p2, p1, p0);
                            }
                        }
                    }
                }
                if (strlen(new_host) == 0) {
                    char* response = "Unsupported IP address.";
                    http_error(response, strlen(response));
                }

#ifdef DEBUG
                fprintf(stderr, "resolv_conf.nameservers.n = %d\n",
                        resolv_conf.nameservers.n);
                fprintf(stderr, "Reverse DNS lookup for %s\n",
                        new_host);
#endif
                int rc = 0;
                if (resolv_conf.nameservers.n > 0) {
                    rc = resolve_dns_reverse(&resolv_conf, AF_INET, new_host, buf, sizeof(buf) - 1);
                }
                if (rc != 0) {
                    char* response = "Failed to resolve host.\n";
                    http_error(response, strlen(response));
                }
                else {
#ifdef DEBUG
                    fprintf(stderr, "Host %s, resolved %s\n", host, buf);
#endif
                    // if all characters in buf are within the base32 charset, base32 decode it
                    attempt_b32decode(buf);
                    char* output = ping(buf);
                    if (output != NULL) {
                        http_ok(output, strlen(output));
                        free(output);
                    }
                    else {
                        char* response = "Connectivity test failed\n";
                        http_error(response, strlen(response));
                    }
                }
                free(host);
            }
        }
    }
    else if (!strcmp(uri, "/set_dns_server") && authenticated) {
        if (!strcmp(method, "POST")) {
            char* dns = form_get(payload, payload_size, "dns_server");
            if (dns != NULL && strlen(dns) >= 7 && strlen(dns) <= 15) {
                // do the manual parsing
                char* p0 = strtok(dns, ".");
                if (p0 != NULL) {
                    char* p1 = strtok(NULL, ".");
                    if (p1 != NULL) {
                        char* p2 = strtok(NULL, ".");
                        if (p2 != NULL) {
                            char* p3 = strtok(NULL, ".");
                            if (p3 != NULL) {
                                uint32_t dns_ipv4 = atoi(p0) | (atoi(p1) << 8) | (atoi(p2) << 16) | (atoi(p3) << 24);
                                if (resolv_conf.nameservers.p != NULL) {
                                    free(resolv_conf.nameservers.p);
                                }
                                resolv_conf.nameservers.p = malloc(sizeof(struct sockaddr_in));
                                memset(resolv_conf.nameservers.p, 0, sizeof(struct sockaddr_in));
                                resolv_conf.nameservers.p->sin_family = AF_INET;
                                resolv_conf.nameservers.p->sin_port = htons(5353);
                                resolv_conf.nameservers.p->sin_addr.s_addr = dns_ipv4;
                                resolv_conf.nameservers.i = 0;
                                resolv_conf.nameservers.n = 1;
                                const char* response = "Successfully setup DNS server\n";
                                http_ok(response, strlen(response));
                            }
                        }
                    }
                }
                free(dns);
            }
        }
        else {
            send_file("./static/set_dns_server.html");
        }
    }
    else if (!strcmp(uri, "/get_dns_server")) {
        if (!strcmp(method, "GET")) {
            if (resolv_conf.nameservers.n == 0) {
                http_ok("empty", 5);
            } else {
                char response[100] = {0};
                sprintf(response, "%s", inet_ntoa(resolv_conf.nameservers.p->sin_addr));
                http_ok(response, strlen(response));
            }
        }
        else {
            http_error("", 0);
        }
    }
    else if (!strcmp(uri, "/logout")) {
        if (!strcmp(method, "GET")) {
            add_cookie("authenticated", "0");
            redirect_to("/");
        }
    }
    else {
        if (!strcmp(method, "GET")) {
            // try to serve the requested file under ./static
            char file_path[1024] = {0};
            strcpy(file_path, "./static/");
            strcat(file_path, uri);
            send_file(file_path);
        }
        else {
            send_file("./static/404.html");
        }
    }
}

int start_server(char* port_str)
{
    struct addrinfo hints;
    struct addrinfo *res = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, port_str, &hints, &res) != 0) {
        perror("getaddrinfo");
        return -1;
    }

    int listenfd;
    struct addrinfo *p = NULL;
    for (p = res; p != NULL; p = p->ai_next) {
        int option = 1;
        listenfd = socket(p->ai_family, p->ai_socktype, 0);
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (listenfd == -1) {
            continue;
        }
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }
    }
    if (p == NULL) {
        perror("socket");
        return -2;
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if (listen(listenfd, 20) != 0) {
        perror("listen");
        return -3;
    }

    return listenfd;
}


// get request header
char *request_header(header_t *reqhdr, const char* name)
{
    header_t *h = reqhdr;
    while(h->name) {
        if (strcmp(h->name, name) == 0) return h->value;
        h++;
    }
    return NULL;
}

size_t read_until(int fd, uint8_t until_char, uint8_t* buffer, size_t max_size)
{
    size_t received_bytes = 0;
    while (1) {
        uint8_t ch;
        int n;
        if (ssl != NULL) {
            n = SSL_read(ssl, &ch, 1);
        }
        else {
            if (fd == 0) {
                n = read(0, &ch, 1);
            }
            else {
                n = recv(fd, &ch, 1, 0);
            }
        }
        if (n <= 0) {
            break;
        }
        if (ch == until_char) {
            break;
        }
        *buffer = ch;
        buffer += 1;
        received_bytes += 1;
        if (received_bytes >= max_size) {
            break;
        }
    }
    return received_bytes;
}

#define MAXLINELENGTH 32768
#define MAXHEADERS 100
#define MAXCONTENTLENGTH 65536


int handle_one_http_request(int clientfd)
{
    uint8_t* buf = malloc(65536 * 2);
    int received_bytes;
    int should_close = 1;  // default to closing the current connection

    uint8_t* first_line_buf = malloc(MAXLINELENGTH);
    size_t first_line_length = read_until(clientfd, '\n', first_line_buf, MAXLINELENGTH);
    if (first_line_length >= MAXLINELENGTH) {
        // the first line is too long
        return 1;
    }
    first_line_buf[first_line_length] = '\x00';
    // parse the first line
    char* method = strtok((char*)first_line_buf,  " \t\r");
    char* uri = strtok(NULL, " \t");
    char* protocol = strtok(NULL, " \t\r"); 

    if (uri == NULL || protocol == NULL) {
        return 1;
    }

    char* qs;
    
    if ((qs = strchr(uri, '?')) != NULL) {
        *qs++ = '\x00';
    } else {
        qs = "";
    }

#ifdef DEBUG
    fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m %s %s\n", method, uri, protocol, qs);
#endif

    header_t headers[MAXHEADERS] = { {"\0", "\0"} };
    int header_index = 0;

    // read more lines
    int content_length = 0;
    while (1) {
        if (header_index >= MAXHEADERS) {
            // too many headers
            return 1;
        }
        char* line_buf = malloc(MAXLINELENGTH);
        size_t line_length = read_until(clientfd, '\n', (char*)line_buf, MAXLINELENGTH);
        if (line_length == 0) {
            break;
        }
        else if (line_length == MAXLINELENGTH) {
            // line too long
            return 1;
        }

        line_buf[line_length] = '\x00';
        if (line_length == 1 && line_buf[0] == '\t') {
            break;
        }

        char *k, *v;
        k = strtok(line_buf, "\r: \t");
        if (!k) {
            break;
        }
        v = strtok(NULL, "\r\n");
        while(*v && *v==' ') {
            ++v;
        }
        header_t *h = &headers[header_index];
        h->name = k;
        h->value = v;
        header_index += 1;
#ifdef DEBUG
        fprintf(stderr, "[H] %s: %s\n", k, v);
#endif
        if (!strcasecmp(h->name, "content-length")) {
            content_length = atoi(h->value);
        }
        else if (!strcasecmp(h->name, "connection")
                && !strcasecmp(h->value, "keep-alive")) {
            should_close = 0;
        }
        // TODO: Free allocated buffers
    }

    if (content_length >= MAXCONTENTLENGTH) {
        return 1;
    }
    uint8_t* payload = malloc(content_length + 1);
    memset(payload, 0, content_length + 1);

    int recv_bytes = 0;
    uint8_t* ptr = payload;
    while (recv_bytes < content_length) {
        int delta;
        if (ssl != NULL) {
            delta = SSL_read(ssl, ptr, content_length - (ptr - payload));
        }
        else {
            if (clientfd == 0) {
                // stdin
                delta = read(0, ptr, content_length - (ptr - payload));
            }
            else {
                // recv
                delta = recv(clientfd, ptr, content_length - (ptr - payload), 0);
            }
        }
        if (delta <= 0) {
            return 1;
        }
        recv_bytes += delta;
    }

#ifdef DEBUG
    fprintf(stderr, "\n\nPayload: %s\n", payload);
#endif

    if (ssl == NULL && clientfd != 0) {
        // if SSL is not in use, bind clientfd to stdout, making it easier to write
        dup2(clientfd, STDOUT_FILENO);
        close(clientfd);
    }

    // call handlers 
    handle(method, uri, headers, header_index, content_length, payload);
    // tidy up
    if (ssl == NULL) {
        fflush(stdout);
    }

#ifdef DEBUG
    fprintf(stderr, "Should close connection: %d\n", should_close);
#endif

    if (clientfd != 0) {
        // In socket mode, we always close the connection
        // TODO: support connection keep-alive in socket mode
        return 1;
    }
    else {
        return should_close;
    }
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "cert.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "cert.key", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }
}

void handle_client(int clientfd)
{
    // Initialize SSL
    SSL_CTX *ctx;
    ctx = create_context();
    configure_context(ctx);
    ssl = SSL_new(ctx);
    if (clientfd == 0) {
        // setup separate file descriptors
        SSL_set_rfd(ssl, 0);
        SSL_set_wfd(ssl, 1);
    }
    else {
        SSL_set_fd(ssl, clientfd);
    }
    if (SSL_accept(ssl) <= 0) {
        return;
    }

    int should_close = 0;
    while (!should_close) {
        should_close = handle_one_http_request(clientfd);
#ifdef DEBUG
        fprintf(stderr, "Finished handling an HTTP requet at fd %d\n", clientfd);
#endif
    }
    SSL_shutdown(ssl);
    SSL_free(ssl);
    shutdown(STDOUT_FILENO, SHUT_WR);
    close(STDOUT_FILENO);
}

int main(int argc, char** argv)
{
	struct sockaddr_in clientaddr;
	int opt;

    char *root = getenv("PWD");
    if (root == NULL) {
        root = malloc(512);
        getcwd(root, 511);
    }
    char port_str[6] = "8888";
    int xinetd_based = 0;

	while ((opt = getopt(argc, argv, "sp:r:")) != -1) {
		switch (opt) {
            case 's':
                // stdin-based
                xinetd_based = 1;
                break;
			case 'r':
				root = malloc(strlen(optarg));
				strcpy(root, optarg);
				break;
			case 'p':
#ifdef VULN
                strcpy(port_str, optarg);
#else
                strncpy(port_str, optarg, sizeof(port_str));
                port_str[5] = '\x00';
#endif
				break;
			default:
				exit(1);
		}
    }

    if (!xinetd_based) {
        // ignore child process states
        signal(SIGCHLD, SIG_IGN);

        int listenfd = start_server(port_str);
        if (listenfd < 0) {
            // failed to start the server
            return 1;
        }

        while (1) {
            socklen_t addrlen = sizeof(clientaddr);
            int client_fd = accept(listenfd, (struct sockaddr *) &clientaddr, &addrlen);

            if (client_fd < 0) {
                perror("accept");
            } else {
                int pid = fork();
                if (pid == 0) {
                    // child process
                    handle_client(client_fd);
                    close(client_fd);
                    exit(0);
                }
                close(client_fd);
            }
        }
    }
    else {
        handle_client(0);
    }

	return 0;
}

