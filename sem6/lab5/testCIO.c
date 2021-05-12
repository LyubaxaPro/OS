//testCIO.c
#include <stdio.h>
#include <fcntl.h>

/*
On my machine, a buffer size of 20 bytes
translated into a 12-character buffer.
Apparently 8 bytes were used up by the
stdio library for bookkeeping.
 */

int main()
{
  // have kernel open connection to file alphabet.txt
 //  получение ссылки на объект файловой системы путем открытия его с флагом
 // O_RDONLY требует, чтобы вызывающая сторона имела разрешение на чтение 
 // объекта, даже когда последующая операция (например, 
 //    fchdir (2), fstat (2)) не требует чтения разрешение на объект.
  int fd = open("alphabet.txt",O_RDONLY);

  // create two a C I/O buffered streams using the above connection 
  //Функция fdopen связывает поток с существующим описателем файла fildes. 
  FILE *fs1 = fdopen(fd,"r"); 
  char buff1[20];
  //Функция изменяет буфер, используемый для операций ввода-вывода, на указанный поток.
  //fs1 - Указатель на объект типа FILE, который связан с открытым потоком.
  //buff1 - Выделенный клиентский буфер должен быть не меньше размера байта. Если вы зададите значение NULL, функция автоматически выделит буфер указанного размера.
  // _IOFBF - modeopen - Fully Buffered: К выходу данные записываются после того, как буфер заполнен. Входной буфер заполняется при открытии файла, а также, если буфер пуст.
  //20 - Размер буфера в байтах. 
  setvbuf(fs1,buff1,_IOFBF,20); 

  FILE *fs2 = fdopen(fd,"r");
  char buff2[20];
  setvbuf(fs2,buff2,_IOFBF,20); 
  
  // read a char & write it alternatingly from fs1 and fs2
  //прочитайте символ и запишите его поочередно из fs1 и fs2
  //Функция fscanf() возвращает количество аргументов, которым действительно были присвоены значения. В их число не входят пропущенные поля. Возврат EOF означает, что при чтении была сделана попытка пройти маркер конца файла.

  int flag1 = 1, flag2 = 2;
  while(flag1 == 1 || flag2 == 1)
  {
    char c;
    flag1 = fscanf(fs1,"%c",&c);
    if (flag1 == 1) {
                      fprintf(stdout,"%c",c);
                    }
    flag2 = fscanf(fs2,"%c",&c);
    if (flag2 == 1) { 
                      fprintf(stdout,"%c",c); 
                    }
  }
  return 0;

}

