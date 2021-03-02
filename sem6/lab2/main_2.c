#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

/* СТЕК */

typedef struct str_node_t
{
    void *data;  
    struct str_node_t *next;
} node_t;

typedef node_t *stack_t;

void initStack(stack_t *stack)
{
    assert(stack != NULL);
    *stack = NULL;
}

int isEmptyStack(stack_t *stack)
{
    return *stack == NULL;
}

void *popStack(stack_t *stack)
{
    assert(stack != NULL);
    stack_t p = *stack;

    if (p == NULL)
        return NULL;

    void *data = p->data;
    *stack = p->next;
    free(p);
    return data;
}

void *peekStack(stack_t *stack)
{
    assert(stack != NULL);
    return (*stack)->data;
}

void pushStack(stack_t *stack, void *data)
{
    assert(stack != NULL);
    stack_t head = (stack_t)malloc(sizeof(node_t));
    if (head != NULL)
    {
        head->data = data;
        head->next = *stack;
        *stack = head;
    }
}
 

typedef struct
{
    char *path;
    DIR *dir;  //в DIR нет полей, которые могут быть изменены приложением пользователя( opendir)
} data_t;

int print_dir(const char *path)
{
    // Создание стека
    stack_t stack; 
    initStack(&stack);

    struct stat sbuf;                                  

	// lstat возвращает информацию о файле в буфер, но в случае символьной ссылки возвращает информацию о 
    // самой ссылке, а не о файле, на который она указывает                                
    if (lstat(path, &sbuf) < 0)
    {
        printf("Ошибка в функции lstat! \n");
        return EXIT_FAILURE;
    }

	// проверка на то, является ли каталогом
    if (S_ISDIR(sbuf.st_mode) == 0)   //st_mode -  режим доступа 
    {
        printf("Путь указан не к директории! \n");
        return EXIT_FAILURE;
    }

	// Функция chdir() устанавливает в качестве текущего каталог, 
	// на который указывает параметр path. Путь может включать в себя и спецификацию диска. 
	// Каталог должен существовать. В случае успеха функция chdir() возвращает 0.
	// При неудаче возвращается значение —1
    if (chdir(path) < 0)
    {
        printf("Ошибка функции chdir! \n");
        return EXIT_FAILURE;
    }

    // Открываем директорию

    DIR *dr; 
	//Функция opendir() открывает поток каталога и возвращает указатель на структуру типа DIR, 
	//которая содержит информацию о каталоге. 
    if ((dr = opendir("./")) == NULL) 
    {
        printf("Ошибка функции opendir!\n");
        return EXIT_FAILURE;
    }

    // В стек
    data_t *data = (data_t *)malloc(sizeof(data_t)); 
    if (data == NULL)
    {
        printf("Ошибка при выделении памяти!");
        return EXIT_FAILURE;
    }
    data->path = path;
    data->dir = dr;

    pushStack(&stack, data);
    int stack_size = 1; 

    printf("%s\n", path);

    // Пока не пуст	
    struct dirent *dirp; 
    while (!isEmptyStack(&stack))
    {
        data = (data_t *)peekStack(&stack);
        if ((dirp = readdir(data->dir)) != NULL)
        {
            // для того чтобы не обрабатывать свой и родительский каталоги
            if (!(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0))
            {
                if (lstat(dirp->d_name, &sbuf) < 0)
                {
                    printf("Ошибка stat!\n");
                    return EXIT_FAILURE;
                }

                if (S_ISDIR(sbuf.st_mode) == 1)
                {
                    if (chdir(dirp->d_name) < 0)
                    {
                        printf("Ошибка chdir!\n");
                        exit(1);
                    }

                    // Открываем директорию
                    if ((dr = opendir(".")) == NULL)
                    {
                        printf("Ошибка opendir!\n");
                    }

                    // В стек
                    data = (data_t *)malloc(sizeof(data_t));
                    if (data == NULL)
                    {
                        printf("Ошибка выделения памяти! \n");
                    }
                    data->path = dirp->d_name;
                    data->dir = dr;

                    pushStack(&stack, data);
                }
                // print
                for (int i = 0; i < stack_size * 3; ++i) 
                    printf(" ");
                printf("%s%s\n", "|-- ", dirp->d_name);

                stack_size += S_ISDIR(sbuf.st_mode);  // если была директория, то стек увеличить
            }
        }
        else
        {
            data = (data_t *)popStack(&stack);
            stack_size -= 1;
            if (closedir(data->dir) < 0)
            {
                printf("Ошибка closedir!\n");
            }
            if (chdir("..") < 0)
            {
                printf("Ошибка chdir!");
                exit(1);
            }
        }
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Введите параметр с путём! \n");
        return EXIT_FAILURE;
    }

    int result = print_dir(argv[1]);

    return result;
}
