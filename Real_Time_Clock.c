/*
 * File:   RTC_MAIN.c
 * Author: Rayen
 *
 * Created on 2 juillet 2025, 12:40
 */


// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
#include <xc.h>
#define _XTAL_FREQ 4000000
#define RS PORTBbits.RB0
#define RW PORTBbits.RB1
#define EN PORTBbits.RB2

#define DS1307 0xD0
#define second 0x00
#define minute 0x01
#define hour 0x02
#define day 0x03
#define date 0x04
#define month 0x05
#define year 0x06
#define control_reg 0x07

void lcd_cmd(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_init();
unsigned char longueur(const unsigned char *str);
void lcd_string(const unsigned char *str);



void bcd_to_ascii(unsigned char value);
char decimal_to_bcd(unsigned char value);
void DS1307_write(char sec,char min,char hr,char day_,char date_,char month_,char year_);
void DS1307_read(char slave_addr,char reg_addr);
unsigned char sec1,min1,hour1,day1,date1,month1,year1,con1;

// lcd functions
void lcd_cmd(unsigned char cmd){
    PORTB = (cmd & 0xF0);
    EN=1;
    RW=0;
    RS=0;
    __delay_ms(2);
    EN=0;
    PORTB = ((cmd <<4) & 0xF0);
    EN=1;
    RW=0;
    RS=0;
    __delay_ms(2);
    EN=0;
}

void lcd_data(unsigned char data){
    PORTB = (data & 0xF0);
    EN=1;
    RW=0;
    RS=1;
    __delay_ms(2);
    EN=0;
    PORTB = ((data <<4) & 0xF0);
    EN=1;
    RW=0;
    RS=1;
    __delay_ms(2);
    EN=0;
}
void lcd_init(){
    lcd_cmd(0x02); //
    lcd_cmd(0x28); //4 bits mode and 2 lines 16 columns
    lcd_cmd(0x0C); // cursor off & lcd on
    lcd_cmd(0x06); //auto increment
    lcd_cmd(0x01); // clear screen
    
}
unsigned char longueur(const unsigned char *str){
    unsigned char c;
    unsigned char i=0;
    if(str !=NULL){
     c= *str;
    }
    while(c !='\0'){
        i++;
        c= *(str+i);
    }
    return i;
    
}
void lcd_string(const unsigned char *str){
    unsigned char i;
    unsigned char num= longueur(str);
    for(i=0; i< num; i++){
        lcd_data(str[i]);
    }
}

void main(void){
  TRISC= 0xFF;
  TRISD= 0x00;
  PORTD = 0x00;
  TRISB= 0x00;
  PORTB= 0x00;
  lcd_init();
  
  SSPADD = 49;
  SSPSTAT= 0x80;
  SSPCON = 0x28;
  SSPCON2 = 0x00;
  
  lcd_cmd(0x80);
  lcd_string("CLOCK:");
  lcd_cmd(0xC0);
  lcd_string("DATE:");
  DS1307_write(0,2,1,1,2,7,25);
  __delay_ms(300);
  while(1){
      __delay_ms(20);
      DS1307_read(DS1307,0);
  }
  return;
  
  
}
void bcd_to_ascii(unsigned char value){
    unsigned char bcd;
    bcd= value;
    bcd= bcd & 0xf0;
    bcd= bcd >>4;
    bcd= bcd | 0x30;
    lcd_data(bcd);
    bcd= value;
    bcd= bcd & 0x0f;
    bcd= bcd| 0x30;
    lcd_data(bcd);
}

char decimal_to_bcd(unsigned char value){
    unsigned char msb,lsb,hex;
    msb= value/10;
    lsb = value%10;
    hex= ((msb<<4)+lsb);
    return hex;
}

void DS1307_write(char second_,char minute_,char hour_,char day_,char date_,char month_,char year_){
  SSPCON2bits.SEN=1;
while(SEN);
PIR1bits.SSPIF=0;

SSPBUF = DS1307;
while(!SSPIF);
PIR1bits.SSPIF=0;
if(SSPCON2bits.ACKSTAT){
   SSPCON2bits.PEN=1;
   while(PEN);
   return;
 }

SSPBUF= second;
while(!SSPIF);
PIR1bits.SSPIF=0;
if(SSPCON2bits.ACKSTAT){
   SSPCON2bits.PEN=1;
   while(PEN);
   return;
 }
SSPBUF= decimal_to_bcd(second_);
while(!SSPIF);
PIR1bits.SSPIF=0;

SSPBUF= decimal_to_bcd(minute_);
while(!SSPIF);
PIR1bits.SSPIF=0;

SSPBUF= decimal_to_bcd(hour_);
while(!SSPIF);
PIR1bits.SSPIF=0;

SSPBUF= decimal_to_bcd(day_);
while(!SSPIF);
PIR1bits.SSPIF=0;

SSPBUF= decimal_to_bcd(date_);
while(!SSPIF);
PIR1bits.SSPIF=0;

SSPBUF= decimal_to_bcd(month_);
while(!SSPIF);
PIR1bits.SSPIF=0;

SSPBUF= decimal_to_bcd(year_);
while(!SSPIF);
PIR1bits.SSPIF=0;

SSPCON2bits.PEN=1;
while(PEN);
}

void DS1307_read(char slave_addr,char reg_addr){
    
    SSPCON2bits.SEN=1;
    while(SEN);
    PIR1bits.SSPIF=0;
    
    SSPBUF = slave_addr;
    while(!SSPIF);
    PIR1bits.SSPIF=0;
    if(SSPCON2bits.ACKSTAT){
       SSPCON2bits.PEN=1;
       while(PEN);
       return;
    }
    
    SSPBUF = reg_addr;
    while(!SSPIF);
    PIR1bits.SSPIF=0;
    if(SSPCON2bits.ACKSTAT){
       SSPCON2bits.PEN=1;
       while(PEN);
       return;
    }
    
    SSPCON2bits.RSEN= 1;
    while(RSEN);
    PIR1bits.SSPIF= 0;
    
    SSPBUF= (slave_addr +1);
    while(!SSPIF);
    PIR1bits.SSPIF= 0;
    if(SSPCON2bits.ACKSTAT){
        SSPCON2bits.PEN=1;
       while(PEN);
       return;
    }
    
    SSPCON2bits.RCEN=1;
    while(!SSPSTATbits.BF);
    sec1 =SSPBUF;
    
    SSPCON2bits.ACKDT =0;
    SSPCON2bits.ACKEN=1;
    while(ACKEN);
    
    
    SSPCON2bits.RCEN= 1;
    while(!SSPSTATbits.BF);
    min1 = SSPBUF;
    SSPCON2bits.ACKDT =0;
    SSPCON2bits.ACKEN=1;
    while(ACKEN);
    
    SSPCON2bits.RCEN= 1;
    while(!SSPSTATbits.BF);
    hour1 = SSPBUF;
    SSPCON2bits.ACKDT =0;
    SSPCON2bits.ACKEN=1;
    while(ACKEN);
    
    SSPCON2bits.RCEN= 1;
    while(!SSPSTATbits.BF);
    day1 = SSPBUF;
    SSPCON2bits.ACKDT =0;
    SSPCON2bits.ACKEN=1;
    while(ACKEN);
    
    SSPCON2bits.RCEN= 1;
    while(!SSPSTATbits.BF);
    date1 = SSPBUF;
    SSPCON2bits.ACKDT =0;
    SSPCON2bits.ACKEN=1;
    while(ACKEN);
    
    SSPCON2bits.RCEN= 1;
    while(!SSPSTATbits.BF);
    month1 = SSPBUF;
    SSPCON2bits.ACKDT =0;
    SSPCON2bits.ACKEN=1;
    while(ACKEN);
    
    SSPCON2bits.RCEN= 1;
    while(!SSPSTATbits.BF);
    year1 = SSPBUF;
    SSPCON2bits.ACKDT =0;
    SSPCON2bits.ACKEN=1;
    while(ACKEN);
    
    SSPCON2bits.RCEN= 1;
    while(!SSPSTATbits.BF);
    con1 = SSPBUF;
    SSPCON2bits.ACKDT =1;
    SSPCON2bits.ACKEN=1;
    while(ACKEN);
    
    SSPCON2bits.PEN= 1;
    while(PEN);
    
    lcd_cmd(0x88);
    bcd_to_ascii(hour1);
    lcd_data(':');
    bcd_to_ascii(min1);
    lcd_data(':');
    bcd_to_ascii(sec1);
  
    
    lcd_cmd(0xC7);
     lcd_data(' ');
    bcd_to_ascii(date1);
    lcd_data('/');
    bcd_to_ascii(month1);
    lcd_data('/');
    bcd_to_ascii(year1);
   
        
}
void delay(unsigned int d){
    while(d--);
}
