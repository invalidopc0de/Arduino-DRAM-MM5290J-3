
#include <avr/io.h>
#include <avr/interrupt.h>

/* 
Code to write value to pins 
{
    int var = get_value_between_zero_and_six();
    digitalWrite(11, HIGH && (var & B00001000));
    digitalWrite(12, HIGH && (var & B00000100));
    digitalWrite(13, HIGH && (var & B00000010));
    digitalWrite(14, HIGH && (var & B00000001));
}
*/



void setAddressBus(short address){
    //digitalWrite(8, HIGH && (i & B01000000));
    PORTB |= (address & B01000000) >> 6;
    //digitalWrite(7, HIGH && (i & B00100000));
    PORTD |= (address & B00100000) << 2; 
    //digitalWrite(6, HIGH && (i & B00010000));
    PORTD |= (address & B00010000) << 2;
    //digitalWrite(5, HIGH && (i & B00001000));
    PORTD |= (address & B00001000) << 2; 
    //digitalWrite(4, HIGH && (i & B00000100));
    PORTD |= (address & B00000100) << 2; 
    //digitalWrite(3, HIGH && (i & B00000010));
    PORTD |= (address & B00000010) << 2; 
    //digitalWrite(2, HIGH && (i & B00000001));
    PORTD |= (address & B00000001) << 2; 
}



//
// Write a bit to a memory location
//
void WRmem(unsigned long address, int value)
{
	int alo,ahi;
	
	alo = address & B1111111;			// split into addressing bytes
	ahi = address >> 7;

	cli();					// turn off refresh while writing data
        //digitalWrite(13, HIGH);         // RAS high
        //digitalWrite(12, HIGH);         // CAS high
        PORTB |= B010000;   // CAS high
        PORTB |= B100000;   // RAS high
        
        setAddressBus(alo);             // low address
        //digitalWrite(13, LOW);          // RAS low
        PORTB ^= B011111;   // RAS low
        
        //digitalWrite(11, LOW);          // WE Low
        PORTB ^= B110111;   // WE low
        setAddressBus(ahi);             // high address
        digitalWrite(9, value);         // Data Input 
        //PORTB |= (value & B00000001) << 1;
        //digitalWrite(12, LOW);          // CAS low 
        PORTB |= B101111;
        
        //digitalWrite(13, HIGH);         // RAS high
        //digitalWrite(12, HIGH);         // CAS high 
        PORTB |= B010000;   // CAS high
        PORTB |= B100000;   // RAS high
        //digitalWrite(13, LOW);          // RAS low
        PORTB ^= B011111;   // RAS low        

	sei();
}

//
// Read a bit from a memory location
//
unsigned char RDmem(unsigned long address)
{
	short value;
	short alo,ahi;
	
	alo = address & B1111111;			// split into addressing bytes
	ahi = address >> 7;

	cli();					// turn off refresh while reading data
        //digitalWrite(13, HIGH);         // RAS high
        //digitalWrite(12, HIGH);         // CAS high
        PORTB |= B010000;   // CAS high
        PORTB |= B100000;   // RAS high	

	setAddressBus(alo);             // low address
	//digitalWrite(13, LOW);		// RAS lo
        PORTB ^= B011111;   // RAS low

        PORTB |= B001000;   // WE high
	setAddressBus(ahi);		// high address
        //digitalWrite(11, HIGH);         // WE high
	//digitalWrite(12, LOW);		// CAS lo
        PORTB ^= B101111;

        //value = digitalRead(10);
	value = HIGH && (PINB & B00000010);
        //digitalWrite(12, HIGH);         // RAS high
        //digitalWrite(13, HIGH);         // CAS lo
        PORTB |= B110000;

        //digitalWrite(11, LOW);          // WE lo
        //digitalWrite(13, LOW);          // RAS lo
        PORTB ^= B110111;
        PORTB ^= B011111;
        	

	sei();
	return value;
}

void setup(){
        //------------------------------
        // Initialize ports
        //------------------------------
	pinMode(2, OUTPUT);  // A0
        pinMode(3, OUTPUT);  // A1
        pinMode(4, OUTPUT);  // A2
        pinMode(5, OUTPUT);  // A3 
        pinMode(6, OUTPUT);  // A4
        pinMode(7, OUTPUT);  // A5
        pinMode(8, OUTPUT);  // A6
       
        pinMode(9, OUTPUT);  // Data Input
        pinMode(10, INPUT);  // Data Output 
        pinMode(11, OUTPUT); // Write Enable
        pinMode(12, OUTPUT); // RAS
        pinMode(13, OUTPUT); // CAS

        //----------------------------------------------------------------------------
        // More init
        //----------------------------------------------------------------------------

	Serial.begin(115200);	// init Serial
	
        randomSeed(analogRead(0));
	// say hello
  	Serial.println("DRAM Interface 0.1"); 

	// Timer initialization
        cli();
        TCCR1A = 0;
        TCCR1B = 0;
        
        OCR1A = 30;
        // turn on CTC mode:
        TCCR1B |= (1 << WGM12);
        // Set CS10 and CS12 bits for 1024 prescaler:
        TCCR1B |= (1 << CS10);
        TCCR1B |= (1 << CS12);
        // enable timer compare interrupt:
        TIMSK1 |= (1 << OCIE1A);
        sei();			// interrupts are also enabled here, starting refresh	
	
        //outp(0, TCCR1A);
	//outp(5, TCCR1B);	// prescaler fClk/1024  tPeriod = 128 uS
	//outp(TI1_H, TCNT1H);	// load counter register
	//outp(TI1_L, TCNT1L);	// load counter register
	//sbi(TIMSK, TOIE1);		// enable timer1 interrupt

}

void loop() 
{
 	unsigned int i;
  	unsigned int c;
	unsigned long addr;
        

       //----------------------------------------------------------------------------
       // Let things loose
       //----------------------------------------------------------------------------                        
	delay(2000);	// wait 20 mS to allow refresh to run at least once
	//
	// do a very simple memory test
	//
	addr = 0x1234;			// some address

        WRmem(addr,0);
        //WRmem(addr+1,1);
        
        delay(5);
        
        c = RDmem(addr);	// get a byte
        Serial.println(c);
        //c = RDmem(addr+1);
        //Serial.println(c);
        
        Serial.println("Memory Written");
}

ISR(TIMER1_COMPA_vect)
{
  // Refresh DRAM cells every 2mS
  int i;
  for (i=0;i<128;i++)	// 128 cycles
  {
    //digitalWrite(13, HIGH); // RAS high
    //digitalWrite(12, HIGH); // CAS high
    PORTB |= B010000;   // CAS high
    PORTB |= B100000;   // RAS high
  
    //digitalWrite(8, HIGH && (i & B01000000));
    PORTB |= (i & B01000000) >> 6;
    //digitalWrite(7, HIGH && (i & B00100000));
    PORTD |= (i & B00100000) << 2; 
    //digitalWrite(6, HIGH && (i & B00010000));
    PORTD |= (i & B00010000) << 2;
    //digitalWrite(5, HIGH && (i & B00001000));
    PORTD |= (i & B00001000) << 2; 
    //digitalWrite(4, HIGH && (i & B00000100));
    PORTD |= (i & B00000100) << 2; 
    //digitalWrite(3, HIGH && (i & B00000010));
    PORTD |= (i & B00000010) << 2; 
    //digitalWrite(2, HIGH && (i & B00000001));
    PORTD |= (i & B00000001) << 2; 
    //digitalWrite(13, LOW);  // RAS low
    //digitalWrite(13, HIGH); // RAS high
    PORTB ^= B011111;   // RAS low
    PORTB |= B100000;   // RAS high
  }
}

