#include <Arduino.h>
#include <stdio.h>
#include "InnoTool.h"

#define FILTER_CHAR     64              //@

void Add_char(char * data)
{
	int n = strlen(data), i, j;
	for (i = 0; i <n;i++)
	{
		if (data[i] == 34)
		{	
			for (j= n;j >= i;j--)
			{
				data[j] = data[j-1];
			}
			data[i] = 92;
			i++;
			n++;
			data[n] = 0;
		}
	}
}

int filter_char(char *respond_data, int begin, int end, char *output, char filter_char)
{
	memset(output,0, sizeof(output));
	int count_filter = 0, lim = 0, start = 0, finish = 0,i;
	for (i = 0; i < strlen(respond_data); i++)
	{
		if ( respond_data[i] == filter_char)
		{
			count_filter ++;
			if (count_filter == begin)			start = i+1;
			if (count_filter == end)			finish = i;
		}
		
	}
	lim = finish - start;
	for (i = 0; i < lim; i++){
		output[i] = respond_data[start];
		start ++;
	}
	output[i] = 0;
	return 0;
}
