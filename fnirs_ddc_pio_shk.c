//Include necessary library
#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "pio_spi.h"
#include <time.h>
 
 
//We define our SPI Connecting between DDC and MCU
#define MOSI          3 //(SCL)  SPI0 MOSI (will not use it)
#define DDC_DOUT      20//(MI)to SPI0 MISO
#define DDC_DCLK      18 //(SCK)to SPI0 SCLK
#define DDC_CLK       25 //(D24) Digital IO/Master Clock Input
#define DDC_CONV      24 //(D25) Digital IO/CLK Confersion Control 0 = Int on Side B and 1 = Int on Side A
#define DDC_FORMAT    1 //(RX) 16 bits is used as buffer
#define DDC_DVALIDn   7 // (5) Digital IO
#define DDC_RANGE0    19 //(MO)
#define DDC_RANGE1    6 //(D4)Digital IO
#define DDC_RANGE2    8 //(D6)Digital IO
#define DDC_RESETn    9 //(D9)Digital IO
#define DDC_TEST      10//(D10)Digital IO
#define DDC_CLK_4X    0//
 
 
pio_spi_inst_t spi;
 
 
 
 
void clockActive(){
//Clock Source: CLK with source from USB (48MHz) So it can be 4MHz, we divide with 12
//Clock Source: CONV with source from USB (48MHz) So it can be 1250Hz, we divide with 38400
    clock_gpio_init(DDC_CLK, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_USB, 12);
    clock_gpio_init(DDC_CONV, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_USB,38400);
}
 
void allPinLow(){
    gpio_init(DDC_CLK);
    gpio_put(DDC_CLK,0);
    gpio_init(DDC_CONV);
    gpio_put(DDC_CONV,0);
    gpio_init(DDC_TEST);
    gpio_put(DDC_TEST,0);
    gpio_init(DDC_CLK_4X);
    gpio_put(DDC_CLK_4X,0);
    gpio_init(DDC_FORMAT);
    gpio_put(DDC_FORMAT,0);
    gpio_init(DDC_RANGE0);
    gpio_put(DDC_RANGE0,0);
    gpio_init(DDC_RANGE1);
    gpio_put(DDC_RANGE1,0);
    gpio_init(DDC_RANGE2);
    gpio_put(DDC_RANGE2,0);
    gpio_init(DDC_RESETn);
    gpio_set_dir(DDC_RESETn, GPIO_OUT);
 
}
 
void resetProgram(){
    allPinLow();
    gpio_put(DDC_RESETn, 0);
    busy_wait_ms(10);
    gpio_put(DDC_RESETn, 1);
    busy_wait_us(50);
}
 
void setDirection(){
    gpio_set_dir(DDC_TEST, GPIO_OUT);
    gpio_set_dir(DDC_CLK_4X, GPIO_OUT);
    gpio_set_dir(DDC_FORMAT, GPIO_OUT);
    gpio_set_dir(DDC_RANGE0, GPIO_OUT);
    gpio_set_dir(DDC_RANGE1, GPIO_OUT);
    gpio_set_dir(DDC_RANGE2, GPIO_OUT);
     
}
 
void calculateValue(uint8_t buffer[]){
        for(int ch=3; ch<4; ch=ch+1){      
            int val, i;
            if(ch == 0 || ch == 2){
                if(ch==0) i=0;
                if(ch==2) i=5;
                val = (buffer[i] << 12) + (buffer[i+1] << 4) + (buffer[i+2] >> 4);
            }else{
                if(ch==1) i=2;
                if(ch==3) i=7;
                val = ((buffer[i] & 0b1111) << 16) + (buffer[i+1] << 8) + buffer[i+2];
            }
            // Convert an integer to binary (in a string)
            val = val-4096;
            char binary[20];
            int n=20;
            for (int i=0;i<n;i++){
                binary[i] = (val & (int)1<<(n-i-1)) ? '1' : '0';
                }
            binary[n]='\0'; //Sign to end of string
            //printf("ch %i: %i \n", ch, val );
            printf("ch %i: %i | %s\n", ch, val, binary );
    }
}
 
 
                                         
int main() {
    stdio_init_all();
    //reset
        resetProgram();
        clockActive();
    //set direction
        setDirection();
        gpio_put(DDC_RANGE2,0);
        gpio_put(DDC_RANGE1,0);
        gpio_put(DDC_RANGE0,0);
        gpio_put(DDC_FORMAT,1);
        gpio_put(DDC_TEST,0);
   
   
    //Declare spi instance
        pio_spi_inst_t spi = {.pio = pio0, .sm = 0};
        gpio_set_dir(spi.cs_pin, GPIO_IN);
    //prog off
        uint prog_offs = pio_add_program(spi.pio, &spi_cpha0_program);
        printf("Loaded program at %d\n", prog_offs);
    //Call the Function from the spi.pio
        pio_spi_init(spi.pio, spi.sm, prog_offs, 8, 3.125f ,false,false,DDC_DCLK,
                 MOSI, DDC_DOUT); //31.25 = 1MHZ @125 clk_sys
       
 
       
 uint8_t  buffer[10];
 
 
    while(1){
//20bit
 
    //    // Readback Before Conv Toggle
    //         while(gpio_get(DDC_DVALIDn) == 1){}; //empty while schleife as long DDC_CONV 0, when change 1 goes to next step
    //         busy_wait_us(8);                 //DDC_CONV 1, busy wait us 40 for setting conv pos with dvalidn
    //         pio_spi_read8_blocking(&spi, buffer, 10);    //DDC_CONV 1
    //         //busy_wait_us(1.75);
    //         calculateValue(buffer);

  //Readback After Conv Toggle
            while(gpio_get(DDC_CONV) == 0){}; //empty while schleife as long DDC_CONV 0, when change 1 goes to next step
            busy_wait_us(8);                 //DDC_CONV 1, busy wait us 40 for setting conv pos with dvalidn
            pio_spi_read8_blocking(&spi, buffer, 10);    //DDC_CONV 1
            //busy_wait_us(1.75);
            calculateValue(buffer);
 
            while(gpio_get(DDC_CONV) == 1){};
            busy_wait_us(8);
            pio_spi_read8_blocking(&spi, buffer, 10);
            //busy_wait_us(1.75);
            calculateValue(buffer);

 
 
//16bit After CONV
    // while(gpio_get(DDC_CONV) == 0){}; //empty while schleife as long DDC_CONV 0, when change 1 goes to next step
    //     busy_wait_us(40);                 //DDC_CONV 1
    //     pio_spi_read8_blocking(&spi, buffer, 8);    //DDC_CONV 1
    //     busy_wait_us(1.75);
    //        for(int i=6;i<8;i=i+2){      //8 bcs 2,2,2,2 and i+2 bcs we want to print 2 buffer
    //             int val = (buffer[i] << 8) + buffer[i+1];
    //             printf("ch %i: %i\n", i/2, val ); // bsc for loop +2 and we want to name the channel 0,1,2,3 that's why i/2
    //     }
 
    //     while(gpio_get(DDC_CONV) == 1){};
    //     busy_wait_us(40);
    //     pio_spi_read8_blocking(&spi, buffer, 8);
    //     busy_wait_us(1.75);
    //         for(int i=6;i<8;i=i+2){                        //i=6 damit wir nur channel 3 print
    //             int val = (buffer[i] << 8) + buffer[i+1]; //shift operaton left schieben MSB
    //             printf("ch %i: %i\n", i/2, val );
    //     }
 
           
    }
   
}
 
   
   
 
 
 
 
 