#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#define PATH "/dev/sock.soc"
#define BUF_LEN 256

int sock;
int sock_flag = 0;  //сокет открыт - 0, закрыт - 1

void catch_ctrlc(int signum)
{
    printf("Получен сигнал Ctrl+C - остановка работы сервера\n");
    close(sock);

    //Перед выходом из программы-сервера следует удалить файл сокета, созданный в результате вызова socket()
    unlink(PATH);
    sock_flag = 1;
}

int main()
{
    struct sockaddr_un server_addr;
    char message[BUF_LEN];
    int error_flag = 0;
    int message_len = 0;
  // Создание сокета в файловом пространстве имен (домен AF_UNIX)
  //домен - накладывает определенные ограничения на формат используемых процессом адресов и их интерпретацию. 
  // адреса интерпретируются как имена файлов в UNIX.

  // Тип сокета -- SOCK_DGRAM означает датаграммный сокет сохраняющий границы сообщений 
  //В пространстве файловых имен датаграммные сокеты также надежны, как и потоковые сокеты.

  // Протокол -- 0, протокол выбирается по умолчанию
    sock = socket(AF_UNIX, SOCK_DGRAM, 0); 
    if (sock < 0)
    {
        printf("%s", strerror(errno));
        return EXIT_FAILURE;
    }

   // Укажем семейство адресов, которыми мы будем пользоваться
   // server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, PATH);

      // bind(дескриптор сокета, указатель на структуру, длина структуры)
      //дескриптор сокета привязывается к заданному адресу.

    //   struct sockaddr 
    // { 
    //        unsigned short sa_family; // Семейство адресов, AF_xxx 
    //        char sa_data[14]; // 14 байтов для хранения адреса 
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) // Связывание сокета с заданным адресом
    {
        printf("%s", strerror(errno));
        return EXIT_FAILURE;
    }

    // отслеживаем сигнал
    signal(SIGINT, catch_ctrlc);

    while (!error_flag && !sock_flag)
    {
        if ((message_len = recvfrom(sock, message, BUF_LEN, 0, NULL, NULL)) < 0) //null null не используем инфу об отправителе
        {
            printf("%s", strerror(errno));
            error_flag = 1;
        }

        if (!sock_flag)
        {
            message[message_len] = 0;
            printf("Получено сообщение: %s\n", message);
        }
    }

    close(sock);
    unlink(PATH);

    return -error_flag;
}