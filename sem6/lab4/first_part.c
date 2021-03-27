// создать файл my_file.txt
// выводить туда информацию  

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> // struct dirent
#include <string.h>

#define BUF_SIZE 0x1000  //16cc  4096
#define PATH_SIZE 40


char *stat_names[] = {
    "1 pid - ID процесса",
    "2 comm - Имя файла",
    "3 state - Состояние процесса",
    "4 ppid - ID родительского процесса",
    "5 pgrp - ID группы процесса",
    "6 session - ID сессии процесса",
    "7 tty_nr - управляющий терминал процесса",
    "8 tpgid - ID внешней группы процессов управляющего терминала",
    "9 flags - Флаги ядра процесса",
    "10 minflt - Количество минорных ошибок процесса (Минорные ошибки не включают ошибки загрузки страниц памяти с диска)",
    "11 cminflt - Количество минорных ошибок дочерних процессов (Минорные ошибки не включают ошибки загрузки страниц памяти с диска)",
    "12 majflt - Количество Мажоных ошибок процесса",
    "13 cmajflt - Количество Мажоных ошибок дочерних процессов процесса",
    "14 utime - Количество времени, в течение которого этот процесс был запланирован в пользовательском режиме",
    "15 stime - Количество времени, в течение которого этот процесс был запланирован в режиме ядра",
    "16 cutime - Количество времени, в течение которого ожидаемые дети этого процесса были запланированы в пользовательском режиме",
    "17 cstime - Количество времени, в течение которого ожидаемые дети этого процесса были запланированы в режиме ядра",
    "18 priority - Приоритет процесса",
    "19 nice - nice"
    "20 num_threads - Количество потоков",
    "21 itrealvalue - Время в тиках до следующего SIGALRM отправленного в процесс из-за интервального таймера",
    "22 starttiime - Время с начала загрузки системы", 
    "23 vsize - Объем виртуальной памяти в байтах",
    "24 rss - Resident Set Size: Количество страниц процесса в физической памяти",
    "25 rsslim - Текущий лимит в байтах на RSS процесса",
    "26 startcode - Адрес, над которым может работать текст программы",
    "27 endcode - Адрес, над которым может работать текст программы",
    "28 startstack - Адрес начала (т. е. дна) стека",
    "29 kstkesp - Текущее значение ESP (Stack pointer), найденное на странице стека ядра для данного процесса",
    "30 kstkeip - Текущее значение EIP (instruction pointer)",
    "31 signal - Растровое изображение отложенных сигналов, отображаемое в виде десятичного числа",
    "32 blocked - Растровое изображение заблокированных сигналов, отображаемое в виде десятичного числа",
    "33 sigignore - Растровое изображение игнорированных сигналов, отображаемое в виде десятичного числа",
    "34 sigcatch - Растровое изображение пойманных сигналов, отображаемое в виде десятичного числа",
    "35 wchan - Канал, в котором происходит ожидание процесса",
    "36 nswap - Количество страниц, поменявшихся местами",
    "37 cnswap - Накопительный своп для дочерних процессов",
    "38 exit_signal - Сигнал, который будет послан родителю, когда процесс будет завершен",
    "39 processor - Номер процессора, на котором было последнее выполнение",
    "40 rt_priority - Приоритет планирования в реальном времени- число в диапазоне от 1 до 99 для процессов, запланированных в соответствии с политикой реального времени",
    "41 policy - Политика планирования",
    "42 delayacct_blkio_tics - Общие блочные задержки ввода/вывода",
    "43 quest_time - Гостевое время процесса",
    "44 cquest_time - Гостевое время  дочерних процессов",
    "45 start_data - Адрес, над которым размещаются инициализированные и неинициализированные данные программы (BSS)",
    "46 end_data - Адрес, под которым размещаются инициализированные и неинициализированные данные программы (BSS)"
    "47 start_brk - Адрес, выше которого куча программ может быть расширена с помощью brk",
    "48 arg_start - Адрес, над которым размещаются аргументы командной строки программы (argv)",
    "49 arg_end - Адрес, под которым размещаются аргументы командной строки программы (argv)",
    "50 env_start - Адрес, над которым размещена программная среда",
    "51 env_end - Адрес, под которым размещена программная среда",
    "52 exit_code - Состояние выхода потока в форме, сообщаемой waitpid"
};

char *statm_names[] = {
    "size - общее число страниц выделенное процессу в виртуальной памяти",
    "resident - размер резидента(страницы, загруженной в физическую память)",
    "shared - количество общих рездентных страниц",
    "text",
    "lib",
    "data",
    "dt - dirty pages"
};

// файл доступный только для чтения
// если процесс зомби, в файле ничего не записано 
//  содержит полную командную строку для процесса
int read_cmdline(FILE* dest, int pid)
{

    char path[PATH_SIZE];
    snprintf(path, PATH_SIZE, "/proc/%d/cmdline", pid);

    fprintf(dest, "\n+ Информация из cmdline(полная командная строка для процесса)\n");

    FILE *f = fopen(path, "r"); // открыть на чтение 
    if (!f)
    {
        fprintf(dest, "Невозможно открыть cmdline\n");
        return -1;
    }

    char buf[BUF_SIZE]; 
    int len = fread(buf, 1, BUF_SIZE, f);   //считываем по 1 байту, возвращает количество успешно считанных элементов
    buf[len] = 0;

    fprintf(dest, "Командная строка для процесса: %s\n", buf);

    fclose(f);

    return 0;
}

//символическая ссылка которая указывает на директорию процесса

int read_cwd(FILE* dest, int pid)
{
    char path[PATH_SIZE];
    snprintf(path, PATH_SIZE, "/proc/%d/cwd", pid);

    fprintf(dest, "\n+ Информация из  cwd(символическая ссылка которая указывает на директорию процесса)\n");

    char buf[BUF_SIZE];

    int len = readlink(path, buf, BUF_SIZE);
    if (!len)
    {
        fprintf(dest, "Невозможно прочитать cwd");
        return -1;
    }
    buf[len] = 0;

    fprintf(dest, "Текущая директория процесса: %s\n", buf);
    return 0;
}

//файл окружения
int read_environ(FILE* dest, int pid)
{
    char path[PATH_SIZE];
    snprintf(path, PATH_SIZE, "/proc/%d/environ", pid);

    fprintf(dest, "\n+ Информация из environ (файл окружения)\n");

    char buf[BUF_SIZE]; // буффер
    int len;

    FILE *f = fopen(path, "r");
    if (!f)
    {
        fprintf(dest, "Невозможно открыть environ");
        return -1;
    }

    // записи разделены by null bytes('\0')
    while ((len = fread(buf, 1, BUF_SIZE, f)) > 0) 
    {
        for (int i = 0; i < len; i++)
            if (buf[i] == 0)
                buf[i] = 10;  //10 - код символа конца строки
        buf[len] = 0; 
        fprintf(dest, "%s\n", buf);
    }

    fclose(f);

    return 0;
}

// символическая ссылка, содержащиая фактический путь к выполненной команде.
int read_exe(FILE* dest, int pid)
{
    char path[PATH_SIZE];
    snprintf(path, PATH_SIZE, "/proc/%d/exe", pid);

    fprintf(dest, "\n+ Информация из exe (символическая ссылка, содержащиая фактический путь к выполненной команде)\n");


    char bufer[BUF_SIZE];

    int len = readlink(path, bufer, BUF_SIZE);
    if (!len)
    {
        fprintf(dest, "Невозможно прочитать exe\n");
        return -1;
    }
    bufer[len] = 0;

    fprintf(dest, "Путь к выполненной команде: %s\n", bufer);

    return 0;
}


//поддиректория содержит одну запись для каждого файла, который
//открыт процессом. Имя каждой такой записи соответствует номеру файлового
//дескриптора и является символьной ссылкой на реальный файл

int read_fd(FILE *dest, int pid)
{

    char path[PATH_SIZE];
    snprintf(path, PATH_SIZE, "/proc/%d/fd", pid);

    fprintf(dest, "\n+ Информация из fd (поддиректория содержит одну запись для каждого файла, который открыт процессом)\n");

    DIR *dp = opendir(path); // open directory
    if (!dp)
    {
        fprintf(dest, " Невозможно открыть fd\n");
        return -1;
    }
    
    struct dirent *dirp;
    char path_to_file[BUF_SIZE]; // путь к открытому файлу
    char link_to_file[BUF_SIZE]; // символическая ссылка на открытый файл
    int len; // чтобы строки не переполнялись 

    while((dirp = readdir(dp)) != NULL) // пока не дошли до конца содержимого 
    {   
        // пропускаем текущую и корневую директории 
        if((strcmp(dirp->d_name, ".") !=0 ) &&
           (strcmp(dirp->d_name, "..") != 0))
        {
            len = sprintf(path_to_file, "%s/%s", path, dirp->d_name);
            path_to_file[len] = 0;

            len = readlink(path_to_file, link_to_file, BUF_SIZE);
            link_to_file[len] = 0;

            fprintf(dest, "opened file: %s [inode: %d] -> symbolic link: %s\n", path_to_file, dirp->d_ino, link_to_file);
        }
    }

    closedir(dp);
    return 0;
}

// currently mapped memory regions
// список выделенных участков памяти, используемых процессом
// The format of the file is:
//нач. адрес кон.адр. права смещение н.у. inode pathname
//00000000 - 00013000 r-xs 00000400 03:03 1264  /lib/
int read_maps(FILE *dest, int pid)
{
    char path[PATH_SIZE];
    snprintf(path, PATH_SIZE, "/proc/%d/maps", pid);

    fprintf(dest, "\n+ Информация из maps (файд - список выделенных участков памяти, используемых процессом)\n");

    char buf[BUF_SIZE]; 
    int len;

    FILE *f = fopen(path, "r"); 
    if (!f)
    {
        fprintf(dest, "Невозможно открыть maps\n");
        return -1;
    }

    while ((len = fread(buf, 1, BUF_SIZE, f)) > 0) 
    {
        buf[len] = 0; 
        fprintf(dest, "%s\n", buf);
    }

    fclose(f);
    return 0;
}

// символическая ссылка - указатель на корень файловой системы 
int read_root(FILE *dest, int pid)
{
    char path[PATH_SIZE];
    snprintf(path, PATH_SIZE, "/proc/%d/root", pid);

    fprintf(dest, "\n+ Информация из root (символическая ссылка - указатель на корень файловой системы)\n");

    char buf[BUF_SIZE]; 

    int len = readlink(path, buf, BUF_SIZE);
    if (!len)
    {
        fprintf(dest, "Невозможно прочитать root\n");
        return -1;
    }

    buf[len] = 0;
    fprintf(dest, "Корень фалойловой системы процесса: %s\n", buf);

    return 0;

}


// файл - Информация о состоянии процесса.
int read_stat(FILE *dest, int pid)
{
    char path[PATH_SIZE];
    snprintf(path, PATH_SIZE, "/proc/%d/stat", pid);

    fprintf(dest, "\n+ Информация из stat (файл - информация о состоянии процесса.)\n");

    char bufer[BUF_SIZE];
    int n = 0;

    FILE *f = fopen(path,"r");
    int len = fread(bufer, 1, BUF_SIZE, f);

    char* p = strtok(bufer, " ");

    while(n <= 52)
    {
        fprintf(dest, "%s: %s \n", stat_names[n], p);
        n++;
        p = strtok(NULL, " ");
        bufer[len] = 0; 
    }

    fclose(f);
    return 0;
}

// Предоставляет информацию об использовании памяти, измеряемой в страницах.
// size  общее число страниц выделенное процессу в виртуальной памяти.
// resident размер резидента(страницы, загруженной в физическую память)

int read_statm(FILE *dest, int pid)
{
    char path[PATH_SIZE];
    snprintf(path, PATH_SIZE, "/proc/%d/statm", pid);

    fprintf(dest, "\n+ Информация из statm (файл - информация об использовании памяти, измеряемой в страницах.)\n");

    char bufer[BUF_SIZE];
    int n = 0;

    FILE *f = fopen(path,"r");
    int len = fread(bufer, 1, BUF_SIZE, f);

    char* p = strtok(bufer, " ");

    while(p != NULL)
    {
        fprintf(dest, "%s: %s \n", statm_names[n], p);
        n++;
        p = strtok(NULL, " ");
        bufer[len] = 0;
    }

    fclose(f);
    return 0;
}



int main(int argc, char* argv[])
{
    int pid;

    if (argc == 2){
        pid = atoi(argv[1]);
    }

    FILE* my_file = fopen("myfile.txt", "w");

    read_cmdline(my_file, pid);
    read_stat(my_file, pid);
    read_statm(my_file, pid);
    read_cwd(my_file, pid);
    read_environ(my_file, pid);
    read_exe(my_file, pid);
    read_fd(my_file, pid);
    read_maps(my_file, pid);
    read_root(my_file, pid);


    fclose(my_file);
    return 0;
}





// 14 utime - Количество времени, в течение которого этот процесс был запланирован в пользовательском режиме: 611 
// 15 stime - Количество времени, в течение которого этот процесс был запланирован в режиме ядра: 2242 