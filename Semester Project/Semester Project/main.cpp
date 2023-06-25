/*  
   NUST Shuttle Tracking System			MPS Project
	Group Members:
			1.	Abdur Rehman
			2.	Syed Muhammad Abubakar
			3.	Asad Ahmed
*/

#define F_CPU 1000000UL			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include Delay header file */
#include <avr\interrupt.h>

#define LCD_Dir  DDRB			/* Define LCD data port direction */
#define LCD_Port PORTB			/* Define LCD data port */
#define RS PB0				/* Define Register Select pin */
#define EN PB1 				/* Define Enable signal pin */

unsigned char Lat[10] = {'3','3','4','0','.','0','6','4','1','6'};	//array of characters to store the Latitude	
unsigned char Long[11] = {'0','7','3','0','6','.','4','0','3','8','8'};	//array of characters to store the Longitude 
unsigned char Latitude_in_deg[11];	//array of characters to store the Latitude
unsigned char Longitude_in_deg[11];	//array of characters to store the Longitude
unsigned char* Lat_deg = &Latitude_in_deg[0];
unsigned char* Long_deg = &Longitude_in_deg[0];
unsigned char Data[21];
unsigned char* d = &Data[0];

unsigned char received_char = 0;
int Latitude_Index = 0;
int Longitude_Index = 0;
int CommaCounter = 0;
bool IsItGGAString = false;
unsigned char GGA_CODE[3];		//array of characters to identify the GGA string


void LCD_Command( unsigned char cmnd )
{
	LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0); /* sending upper nibble */
	LCD_Port &= ~ (1<<RS);		/* RS=0, command reg. */
	LCD_Port |= (1<<EN);		/* Enable pulse */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4);  /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}

void LCD_Char( unsigned char data )
{
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0); /* sending upper nibble */
	LCD_Port |= (1<<RS);		/* RS=1, data reg. */
	LCD_Port|= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (data << 4); /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}

void LCD_Init (void)			/* LCD Initialize function */
{
	LCD_Dir = 0xFF;			/* Make LCD port direction as o/p */
	_delay_ms(20);			/* LCD Power ON delay always >15ms */
	
	LCD_Command(0x02);		/* send for 4 bit initialization of LCD  */
	LCD_Command(0x28);              /* 2 line, 5*7 matrix in 4-bit mode */
	LCD_Command(0x0c);              /* Display on cursor off*/
	LCD_Command(0x06);              /* Increment cursor (shift cursor to right)*/
	LCD_Command(0x01);              /* Clear display screen*/
	_delay_ms(2);
}

void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String (unsigned char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);	/* Command of first row and required position<16 */
	LCD_String(str);		/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);		/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);		/* Cursor at home position */
}

int to_int (char count){
	if (count=='0') return 0;
	else if (count=='1') return 1;
	else if (count=='2') return 2;
	else if (count=='3') return 3;
	else if (count=='4') return 4;
	else if (count=='5') return 5;
	else if (count=='6') return 6;
	else if (count=='7') return 7;
	else if (count=='8') return 8;
	else if (count=='9') return 9;
	else return 0;
}

void lat_to_deg(){
	long long int lat_int = 0;
	for (int i = 0;i < 10;i++){
		if (i != 4){
			lat_int = (lat_int*10) + to_int(Lat[i]);
		}
		else{
			//do nothing
		}
	}
	int deg = lat_int/10000000;
	long int min = (lat_int - (deg*10000000));
	min = min / 60;
	//putting the value into an array of string
	int temp = 0;
	for(int i = 0; i < 10;i++){
		if(i < 2){
			deg = deg*10;
			temp = deg/100;
			deg = deg % 100;
			Latitude_in_deg[i] = '0' + temp;
		}
		else if(i == 2){
			Latitude_in_deg[i] = '.';
		}
		else{
			min = min*10;
			temp = min/100000;
			min = min%100000;
			Latitude_in_deg[i] = '0' + temp;
		}
	}
}

void long_to_deg(){
	long long int long_int = 0;
	for (int i = 0;i < 11;i++){
		if (i != 5){
			long_int = (long_int*10) + to_int(Long[i]);
		}
		else{
			//do nothing
		}
	}
	int deg = long_int/10000000;
	long int min = (long_int - (deg*10000000));
	min = min / 60;
	//putting final value into array
	int temp = 0;
	for(int i = 0; i < 10;i++){
		if(i < 2){
			deg = deg*10;
			temp = deg/100;
			deg = deg % 100;
			Longitude_in_deg[i] = '0' + temp;
		}
		else if(i == 2){
			Longitude_in_deg[i] = '.';
		}
		else{
			min = min*10;
			temp = min/100000;
			min = min%100000;
			Longitude_in_deg[i] = '0' + temp;
		}
	}
}

void Display_on_LCD()
{
	LCD_Clear();
	LCD_String("Lat: ");	// Write string on 1st line of LCD
	LCD_String(Lat_deg);
	LCD_String_xy (0, 14,"'N");	
	LCD_Command(0xC0);		//Go to 2nd line
	LCD_String("Long:");	//Write string on 2nd line
	LCD_String(Long_deg);
	LCD_String_xy (1, 14,"'E");	
}

void Convert_to_Deg()
{
	lat_to_deg();
	long_to_deg();
}
 
void Make_Data_String()
{
	for(int i = 0; i < 10;i++){
		Data[i] = Latitude_in_deg[i];
	}
	Data[10] = ',';
	 int j = 0;
	for(int i = 11; i < 21; i++){
		Data[i] = Longitude_in_deg[j];
		j++;
	}
}

void USARTInit()
{
	UBRRL = 0xC;
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
}

void USARTWriteChar(char data)
{
	while(!(UCSRA & (1<<UDRE)));
	UDR=data;
}

void USART_StringTransmit(unsigned char* s)
{
	for(int i = 0; i < 21 ; i++){
		USARTWriteChar(s[i]);
	}
}
ISR(USART_RXC_vect)
{
	char received_char = UDR;
	
	if(received_char =='$'){                 // check for '$'
		Latitude_Index = 0;
		Longitude_Index = 0;
		CommaCounter = 0;
		IsItGGAString = false;
	}
	else if(IsItGGAString == true){          // if true save GGA info. into buffer
		if(received_char == ',' ){
			CommaCounter++;    // increment comma counter
		}
		if(CommaCounter == 2){
			Lat[Latitude_Index++] = received_char;
		}
		if(CommaCounter == 4){
			Long[Longitude_Index++] = received_char;
		}
	}
	else if(GGA_CODE[0] == 'G' && GGA_CODE[1] == 'G' && GGA_CODE[2] == 'A'){    // check for GGA string
		IsItGGAString = true;
		GGA_CODE[0] = 0; GGA_CODE[1] = 0; GGA_CODE[2] = 0;
	}
	else{
		GGA_CODE[0] = GGA_CODE[1];  GGA_CODE[1] = GGA_CODE[2]; GGA_CODE[2] = received_char;
	}
}

int main()
{
	LCD_Init();			//Initialization of LCD
	_delay_ms(4000);	//wait for GPS Initialization
	USARTInit();
	sei();
	while(1){
		Convert_to_Deg();
		Display_on_LCD();
		Make_Data_String();
		USART_StringTransmit(d);
		_delay_ms(1000);
	}
}