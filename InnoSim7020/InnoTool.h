#ifndef _INNOTOOL_H_
#define _INNOTOOL_H_
#include <stdio.h>
#define FLASH_MEMORY_SIZE 40
#define END_CHAR          63

void Add_char(char * data);
int  filter_char(char *respond_data, int begin, int end, char *output, char filter_char);

#endif /* _UART_H_ */
