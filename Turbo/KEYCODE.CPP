#include <iostream.h>
#include <bios.h>

void main()
{
	int key=-1;
	do
	{
		if(bioskey(1))
		{
			key=bioskey(0);
			cout<<key<<endl;
		}
		else
			key=-1;
	}while(key!=283);
}