#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PATH "/dev/sock.soc"  // так как некоторые системы разрешают создать сокет только в некоторых директориях
#define BUF_LEN 256

int main(int argc, char **argv)
{
    struct sockaddr_un server_addr;  //адрес сервера
    int c_pid; //pid клиента
    char message[BUF_LEN];
   // struct sockaddr_un {
  //   sa_family_t sun_family;                AF_UNIX 
  //   char        sun_path[108];            /* имя пути */  (размер 108 байт)
  // };


   // Создание сокета в файловом пространстве имен (домен AF_UNIX)
   // Тип сокета -- SOCK_DGRAM означает датаграммный сокет
   // Протокол -- 0, протокол выбирается по умолчанию
    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        printf("%s", strerror(errno));
        return EXIT_FAILURE;
    }

   // server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, PATH);

    // Получаем pid клиента 
    c_pid =  getpid();

    while (1)
    {
        printf("Введите сообщение клиента с pid = %d: ", c_pid);
        fgets(message, BUF_LEN, stdin);
        printf("\n");

        // отправка серверу

        // Первый параметр функции sendto() – дескриптор сокета, второй и третий параметры позволяют указать адрес буфера для передачи данных и его длину.
        // Четвертый параметр предназначен для передачи дополнительных флагов.
        //  Предпоследний и последний параметры несут информацию об адресе сервера и его длине, соответственно. 
        sendto(sock, message, strlen(message), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    return EXIT_SUCCESS;
}