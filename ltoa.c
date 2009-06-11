//converts long ints to character arrays
//http://devel.phor.net/file.php?project=Chatterbox&path=ltoa.c

static char buf[12];

extern char * ultoa(unsigned long);

char * ltoa(long val)
{
     char *p;
     int flg = 0;
     if( val < 0 ) { flg++; val= -val; 
     }
     p = ultoa(val);
     if(flg) *--p = '-';
     return p;
  }

  char * ultoa(unsigned long val)
  {
     char *p;
     
     p = buf+sizeof(buf);
     *--p = '\0';
     
     do
       {
          *--p = '0' + val%10;
          val/=10;
       }
     while(val);
     return p;
  }
