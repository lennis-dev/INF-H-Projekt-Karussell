#include "LCD.h"

lcd::lcd(void)
    {
        i2c=new SoftwareI2C(PA_12,PA_11);
        //po=new PortOut(PortC,0xFF);
        //t=new DigitalIn(PA_1,PullDown);
        init();
        
    };
    
void lcd::clear(void)
{

    sendeByte(0x01,0,0);
    cursorpos(0);
    //sleep_for(20);

};

void lcd::locate(int column, int row)
{
    cursorpos(column+row*0x40);
}

void lcd::putc(int c)
{
    sendeByte(c,0,1);   
}

void lcd::cls()
{
    clear();
}

/*
void lcd::warte(void)
{
    *po=wert;
    while(*t==0);
    wait_ms(20);
    while(*t==1);
    wait_ms(20);
};*/

void lcd::warte()
{   
    asm("push   {R0,R1}          \n"
        "ldr R0,=32000000/5/1000     \n"
        "mov R1,#1              \n"
        "1:                     \n"  
        "subs  R0,R1            \n"
        "bne   1b               \n"
        "pop   {R0,R1}          \n"
        );    
}
void lcd::sendeByte(char b,uint8_t rw, uint8_t rs )
{
    
    wert=(b&0xF0)+0x08+((rw&0x01)<<1)+(rs&0x01);
    i2c->write(Adresse,wert);
    warte();
    wert=(b&0xF0)+0xC+((rw&0x01)<<1)+(rs&0x01);
    i2c->write(Adresse,wert);
    warte();
    wert=(b&0xF0)+0x8+((rw&0x01)<<1)+(rs&0x01);
    i2c->write(Adresse,wert);
    warte();
    wert=((b&0xF)<<4)+0x8+((rw&0x01)<<1)+(rs&0x01);
    i2c->write(Adresse,wert);
    warte();
    wert=((b&0xF)<<4)+0xC+((rw&0x01)<<1)+(rs&0x01);
    i2c->write(Adresse,wert);
    warte();
    wert=((b&0xF)<<4)+0x8+((rw&0x01)<<1)+(rs&0x01);
    i2c->write(Adresse,wert);
    warte();
}

void lcd::sendeNippel(char b,uint8_t rw, uint8_t rs )
{
    wert=((b&0xF)<<4)+0x0+((rw&0x01)<<1)+(rs&0x01);
    i2c->write(Adresse,wert);
    warte();
    wert=((b&0xF)<<4)+0x4+((rw&0x01)<<1)+(rs&0x01);
    i2c->write(Adresse,wert);
    warte();
    wert=((b&0xF)<<4)+0x0+((rw&0x01)<<1)+(rs&0x01);
    i2c->write(Adresse,wert);
    warte();
}
void lcd::cursorpos(uint8_t pos)
{
    sendeByte(0x80+pos,0,0);
}
void lcd::init(void)
{
    //Adresse=pAdresse<<1;
    uint8_t data[1];
    for (Adresse=0;Adresse<255&&data[0]!=0x55;Adresse++)
    {
        i2c->write(Adresse,0x55);
        i2c->read(Adresse,data,1);
    }
    
    wait_us(20000);
    sendeNippel(0b0011,0,0);
    wait_us(5000);
    sendeNippel(0b0011,0,0);
    wait_us(1000);
    sendeNippel(0b0011,0,0);

    sendeNippel(0b0010,0,0);

    sendeByte(0b00101000,0,0);  //4Bit 2 Zeilen


    sendeByte(0b00000001,0,0);  //display clear

    sendeByte(0b00000110,0,0);  //Increment Cursor*/

    sendeByte(0b10000000,0,0);  //Home

    sendeByte(0b00001110,0,0);  //Display On    


    cursorpos(0x0);
    
    printf("GSOE V%0.1f",1.1);

    
    
}
int lcd::printf(const char *format, ...)
    {
    char buf[20];
    va_list args;
    va_start(args, format);
    vsprintf(buf,format,args);
    va_end(args);
    //LCD_i2c_textaus(buf);
    for (int i=0;i<16 && buf[i]!=0;i++)
            sendeByte(buf[i],0,1);
    return 0;
    }