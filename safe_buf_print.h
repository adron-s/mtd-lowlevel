#ifndef SAFE_BUF_PRINT_H
#define SAFE_BUF_PRINT_H

/*
  Реализация алгоритма safe_buf_print(). может быть использован для работы с proc файлами.
  Позволяет не заботиться о выделении памяти для buf в который пишут посредством sprintf.
*/

//******************************************************************
/* выполняет объявление переменных нужных для safe_buf_printf
  если prefix == buf то получим:
    buf - в нем будет зраниться строка результата
    buf_len = размер памяти выделеной под buf(после вызова safe_buf_printf
      может меняться если памяти не хватило и было произведено перевыделение)
    bufP - указатель на текущее место записи в buf. используется в safe_buf_printf
    buf_base_size - размер блоков которые выделяются под buf в случае нехватки памяти
*/
#define prepare_to_safe_buf_print(prefix, base_size)     \
  char *prefix = NULL;                                   \
  uint32_t prefix##_len = 0;                             \
  char *prefix##P = NULL;                                \
  uint32_t prefix##_base_size = base_size;               \
//-----------------------------------------------------------------

//******************************************************************
/* выделяет и перевыделяет памят(елси недостаточно) для buf */
static inline void realloc_buf_if_need(char **buf, char **bufP, uint32_t needed_len,
                                       uint32_t *buf_mem_len, uint32_t base_size){
  //если места в памяти слишком мало или же память вообще еще не выделали не разу то нужно перевыделить
  if(*buf_mem_len - (*bufP - *buf) < needed_len){
    char *new_buf;
    //выделяем новую мапять на print_to_buf_base_size больше старой по размеру
    new_buf = kmalloc(*buf_mem_len + base_size, GFP_KERNEL);
    //копируем данные из старой памяти
    if(*buf_mem_len > 0){
      memcpy(new_buf, *buf, *buf_mem_len);
    }
    //перерассчитываем bufP
    *bufP = new_buf + (*bufP - *buf);
    //вписываем новый размер памяти
    *buf_mem_len += base_size;
    //освободим старую память
    if(*buf){
      kfree(*buf);
    }
    //buf теперь указывает на новую большую область памяти
    *buf = new_buf;
  }
}//-----------------------------------------------------------------

//******************************************************************
/* sprintf-ает строку в prefix буфер при этом выделяя память
   для prefix буфера. можно много раз вызывать и набивать в
   буфер разные строки. память будет перевыделяться в случае
   необходимости. */
#define safe_buf_print(prefix, args...){                  \
  int len;                                                \
  /* измерим сколько памяти нужно (+1 для '\0' !) */      \
  len = snprintf(NULL, 0, ##args) + 1;                    \
  /* проверим на ошибку */                                \
  if(len <= 0){                                           \
    PRINTE2("snprintf error 1!\n");                       \
    goto fail;                                            \
  }                                                       \
  /* выделим память если ее мало или вообще нет */        \
  realloc_buf_if_need(&prefix, &prefix##P, len,           \
                      &prefix##_len, prefix##_base_size); \
  /* копируем */                                          \
  len = sprintf(prefix##P, ##args);                       \
  /* В len длина строки но без '\0' !*/                   \
  if(len > 0){                                            \
    prefix##P += len; /* следующая запись пойдет от */    \
             /* позиции где сейчас символ '\0'! */        \
  }else{ /* ошибка в sprintf */                           \
    PRINTE2("sprintf error 2!\n");                        \
    goto fail;                                            \
  }                                                       \
}//-----------------------------------------------------------------

#endif /* SAFE_BUF_PRINT_H */
