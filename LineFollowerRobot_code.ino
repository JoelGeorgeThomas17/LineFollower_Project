#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

uint8_t Sensor1 = 0;
uint8_t Sensor2 = 1;
uint8_t POT = 7;
uint8_t BatteryLevel = 6;
uint16_t EEMEM stored_threshold;
uint16_t threshold = 500;
float kKp = 0.004;  // Reduced gain

ISR(INT0_vect) {
    _delay_ms(5);
    if (!(PIND & (1 << PD2))) {
        uint32_t total = 0;
        for (uint8_t i = 0; i < 20; i++) {
            uint16_t left = ReadADCSingleConversion(0);
            uint16_t right = ReadADCSingleConversion(1);
            total += (left + right) / 2;
            _delay_ms(20);
        }
        uint16_t average_threshold = total / 20;
        eeprom_update_word(&stored_threshold, average_threshold);
        DDRB |= (1 << PB5);
        PORTB |= (1 << PB5);
        _delay_ms(250);
        PORTB &= ~(1 << PB5);
        _delay_ms(250);
        PORTB |= (1 << PB5);
        _delay_ms(250);
        PORTB &= ~(1 << PB5);
    }
}

void InitUSART(uint32_t baud_rate) {
    uint16_t ubrr_val = (F_CPU - (8 * baud_rate)) / (16 * baud_rate);
    UBRR0 = ubrr_val;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

void TransmitByte(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void printADC(char* str) {
    while (*str) {
        TransmitByte(*str++);
    }
}

void InitADC() {
    ADMUX |= (1 << REFS0);
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1);
    ADCSRA |= (1 << ADEN);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    uint16_t garbage = ADC;
}

uint16_t ReadADCSingleConversion(uint8_t channel) {
    uint8_t MuxMask = (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0);
    ADMUX &= ~MuxMask;
    ADMUX |= channel;
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

void InitTimer() {
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    ICR1 = 15999;
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
    OCR1A = 0;
    OCR1B = 0;
}

void setM1speed(float speed) {
    static int8_t last_dir = 0;

    if (speed > 1.0) speed = 1.0;
    if (speed < -1.0) speed = -1.0;

    int8_t current_dir = (speed > 0) ? 1 : (speed < 0 ? -1 : 0);

    if (current_dir != last_dir && current_dir != 0) {
        OCR1A = 0;
        _delay_ms(20);
    }

    if (speed > 0) {
        PORTB &= ~(1 << PB0);
        OCR1A = speed * ICR1;
    } else if (speed < 0) {
        PORTB |= (1 << PB0);
        OCR1A = -speed * ICR1;
    } else {
        OCR1A = 0;
    }

    last_dir = current_dir;
}

void setM2speed(float speed) {
    static int8_t last_dir = 0;

    if (speed > 1.0) speed = 1.0;
    if (speed < -1.0) speed = -1.0;

    int8_t current_dir = (speed > 0) ? 1 : (speed < 0 ? -1 : 0);

    if (current_dir != last_dir && current_dir != 0) {
        OCR1B = 0;
        _delay_ms(20);
    }

    if (speed > 0) {
        PORTB &= ~(1 << PB3);
        OCR1B = speed * ICR1;
    } else if (speed < 0) {
        PORTB |= (1 << PB3);
        OCR1B = -speed * ICR1;
    } else {
        OCR1B = 0;
    }

    last_dir = current_dir;
}

void LineFollow_bangbang() {
    uint16_t POT_value = ReadADCSingleConversion(7);
    float set_speed_right = ((float)POT_value / 1024);
    float set_speed_left = 0.8 * set_speed_right;

    uint16_t RightSensor_reading = ReadADCSingleConversion(0);
    uint16_t LeftSensor_reading = ReadADCSingleConversion(1);

    if (LeftSensor_reading > threshold && RightSensor_reading > threshold) {
        setM1speed(set_speed_right);
        setM2speed(set_speed_left);
    } else if (LeftSensor_reading < threshold && RightSensor_reading > threshold) {
        setM1speed(0);
        setM2speed(set_speed_left);
    } else if (LeftSensor_reading > threshold && RightSensor_reading < threshold) {
        setM1speed(set_speed_right);
        setM2speed(0);
    } else {
        setM1speed(-0.6 * set_speed_right);
        setM2speed(-0.6 * set_speed_left);
    }
}

// ---------- P Control Logic ----------
void LineFollow_PMode() {
    uint16_t POT_value = ReadADCSingleConversion(7);
    float base_speed_right = ((float)POT_value / 1024.0);
    float base_speed_left = 0.85* base_speed_right;

    uint16_t Left = ReadADCSingleConversion(0);
    uint16_t Right = ReadADCSingleConversion(1);
    int16_t error = Left - Right;
    if((Left>threshold&&Right<threshold)||(Left<threshold&&Right>threshold)){kKp = 0.05;}
    else{kKp = 0.004;}
    float offset = kKp * (float)error;

    // Clamp offset
    if (offset > 1.0) offset = 1.0;
    if (offset < -1.0) offset = -1.0;

    float right_speed = base_speed_right * (1.0 - offset);
    float left_speed = base_speed_left * (1.0 + offset);

    // Both sensors see the line - go forward
    if (Left > threshold && Right > threshold ) {
        setM1speed(base_speed_right);
        setM2speed(base_speed_left);
    }
    // Lost line - reverse gently
    else if (Left < threshold && Right < threshold ) {
        setM1speed(-0.5*  base_speed_right);
        setM2speed(- 0.5*base_speed_left);
    }
    // Sharp left turn
    else if (error > 400) {
        setM1speed(0);  // slow right
        setM2speed( base_speed_left);   // fast left
    }
    // Sharp right turn
    else if (error < -400) {
        setM1speed( base_speed_right);  // fast right
        setM2speed(0);   // slow left
    }
    // Mild correction
    else {
        setM1speed(right_speed);
        setM2speed(left_speed);
    }
}


int main(void) {
    PORTD |= (1 << PD4)  | (1 << PD2);
    InitUSART(9600);
    InitTimer();
    InitADC();

    EIMSK |= (1 << INT0);
    EICRA |= (1 << ISC01);
    sei();

    threshold = eeprom_read_word(&stored_threshold);
    uint16_t BatteryLevel = ReadADCSingleConversion(6);
    float battery = map(BatteryLevel,0,1023,0,9.0);
    while(1) {
      if(battery<6.0){
  PORTB ^= (1<<PB5);
  _delay_ms(100);
  }
  else{
        uint8_t pd4_high = (PIND & (1 << PD4));
       
        if (!pd4_high) {LineFollow_bangbang();}
        else{ LineFollow_PMode();}

        }
        _delay_ms(10);
    }

    return 0;
}
