#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>

#define BUF_LEN 256
#define SERVER_SOCK_PORT 42317
#define MAX_COUNT 8

int sock;
int sock_flag = 0;

void catch_ctrlc(int signum)
{
    printf("Получен сигнал Ctrl+C - остановка работы сервера\n");
    sock_flag = 1;
    close(sock);
}

// прочитать сообщение от клиента
void read_message(unsigned int client_id, int *client_sockets)
{
    char message[BUF_LEN];
    memset(message, 0, BUF_LEN);

    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);

    //Getpeername возвращает  имя подключившегося сокета. Параметр namelen должен быть инициализирован в целях отображения объема памяти, который занимает name.
    //По возвращении он содержит размер памяти, занимаемый именем машины (байт). Имя не считывается, если буфер окажется слишком мал.
    //int getpeername(int s, struct sockaddr *name, socklen_t *namelen);    
    getpeername(client_sockets[client_id], (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);

    // читаем сообщение (при неудаче считаем что клиент отключился)
    int rec_BUF_LEN = recv(client_sockets[client_id], message, BUF_LEN, 0);
    if (rec_BUF_LEN == 0)
    {
        printf("Клиент отключился: адрес = %s:%d!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        client_sockets[client_id] = 0;
        close(client_sockets[client_id]);
    }
    else
    {
        message[rec_BUF_LEN] = '\0';
        printf("Сообщение: %s от клиента %s:%d\n",message, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
}

int main(void)
{
    struct sockaddr_in server_addr;

    // создать сокет,
    //AF_INET открываемый сокет должен быть сетевым
    //SOCK_STREAM сокет потоковый
    //Обеспечивает создание двусторонних, надёжных потоков байтов на основе
    //установления соединения. Может также поддерживаться механизм внепоточных данных.
    //Они не сохраняют границы записей. Потоковый сокет должен быть в состоянии соединения
    //перед тем, как из него можно будет отсылать данные или принимать их. 

    // 0 - протокол по умолчанию
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("%s", strerror(errno));
        return EXIT_FAILURE;
    }

    // инициализировать адрес сокета-сервера
// struct sockaddr_in
//  { 
//     short int sin_family; // Семейство адресов
//     unsigned short int sin_port; // Номер порта
//     struct in_addr sin_addr; // IP-адрес
//     unsigned char sin_zero[8]; // Дополнение до размера структуры sockaddr
//  };


    server_addr.sin_family = AF_INET;
    //преобразовать число из порядка хоста в сетевой
    server_addr.sin_port = htons(SERVER_SOCK_PORT); //Host To Network Short
    //Функция htons() переписывает двухбайтовое значение порта так, чтобы порядок байтов соответствовал сетевому.
    
    //программа сервер зарегистрируется на всех адресах той машины,
    //на которой она выполняется.
    server_addr.sin_addr.s_addr = INADDR_ANY; 
                                              
                                              
    // привязать сокет приложения-сервера к адресу
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("%s", strerror(errno));
        return EXIT_FAILURE;
    }

    //listen(2), которая переводит сервер в режим ожидания запроса на соединение
    if (listen(sock, MAX_COUNT) < 0)
    {
        printf("%s", strerror(errno));
        close(sock);
        return EXIT_FAILURE;
    }

    // http://ru.manpages.org/fd_set/2
    int client_sockets[MAX_COUNT] = {0};
    while (!sock_flag)
    {
        fd_set rfds;

        //очищает набор
        FD_ZERO(&rfds);

        //добавляет заданный дескриптор из набора.
        FD_SET(sock, &rfds);

        int max_fd = sock;

        //по документации
        for (int i = 0; i < MAX_COUNT; i++)
        {
            int fd = client_sockets[i];
            if (fd > 0)
                FD_SET(fd, &rfds);
            max_fd = (fd > max_fd) ? (fd) : (max_fd);
        }

        // ждем нового соединения или нового сообщения
        //  int   pselect(int   n,   fd_set   *readfds,  fd_set  *writefds,  fd_set *exceptfds, const struct timespec *timeout, sigset_t * sigmask);
        //n на единицу больше самого большого номера файловый дескриптор из всех наборов.!!! 
        // Первый параметр функции – количество проверяемых дескрипторов. Второй, третий и четвертый параметры
        // функции представляют собой наборы дескрипторов, которые следует проверять, соответственно, на готовность к чтению, записи и на наличие исключительных ситуаций.
        //ждать вечно - timeval = NULL
        int updates_cl_count = pselect(max_fd + 1, &rfds, NULL, NULL, NULL, NULL); //процесс блокируется в ожидании обновления, выходим из селекта когда в каком-либо из файлов соотв дескр появились новые символы 
        
        //после выхода из пселект в rfds остались только файлы с обновлениями

        // если новое соединение есть
        // если сокет остался в наборе, то к нему пришло новое подключение
        if (FD_ISSET(sock, &rfds))
        {
            // принять соединение с клиентом
            struct sockaddr_in client_addr;
            int client_addr_size = sizeof(client_addr);

            //используется для получения нового сокета для нового входящего соединения. 
            //которая устанавливает соединение в ответ на запрос клиента и создает копию сокета для того, 
            //чтобы исходный сокет мог продолжать прослушивание
            //неявное связывание с адресом
            int accepted_sock = accept(sock, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
            // Получив запрос на соединение, функция accept() возвращает новый сокет, открытый для обмена данными с клиентом, запросившим соединение.
            //  Сервер как бы перенаправляет запрошенное соединение на другой сокет, оставляя сокет sock свободным для прослушивания запросов на установку соединения.
            //   Второй параметр функции accept() содержит сведения об адресе клиента, запросившего соединение, 
            // а третий параметр указывает размер второго. Так же как и при вызове функции recvfom(), 
            // может быть  передано значение NULL в последнем и предпоследнем параметрах. 


            if (accepted_sock < 0)
            {
                printf("%s", strerror(errno));
                close(sock);
                return EXIT_FAILURE;
            }

            // обновляем информацию о наборе клиентов
            int added_flag = 0; // 0 - есть свободное место в таблице клиентов, 1 - нет свободного места
            for (int i = 0; i < MAX_COUNT && !added_flag; i++)
            {
                // находим свободное место в таблице клиентов
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = accepted_sock;
                    added_flag = 1;
                }
            }

            
            printf("Новое подключение: fd = %d, address = %s:%d\n", accepted_sock, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }

        // проверяем есть ли полученные сообщения
        for (int i = 0; i < MAX_COUNT; i++)
        {
            int fd = client_sockets[i];
            if ((fd > 0) && FD_ISSET(fd, &rfds))
            {
                read_message(i, client_sockets);
            }
        }
    }

    return 0;
}