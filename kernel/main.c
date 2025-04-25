volatile static int started = 0;

void main(void) 
{
	if(started == 0) {
		started = 1;
	} else {
		started = 0;
	}
}
