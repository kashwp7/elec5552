//Libraries

// Pin Definition
	// Data pins
		// D I/O 1 -> D I/O 8
const int GPIB_DATA[] = {};

	// Command pins
const int GPIB_EOI = ;
		// IFC on interupt pin
const int GPIB_IFC = ; 
const int GPIB_SRQ = ;
const int GPIB_ATN = ; 
const int GPIB_REN = ; 

const int GPIB_DAV = ;
const int GPIB_NRFD = ;
const int GPIB_NDAC = ;	
const int GPIB_CONTROL[] = {GPIB_REN, GPIB_ATN, GPIB_SRQ, GPIB_IFC, GPIB_EOI};
const int GPIB_SHAKE[] = {GPIB_NDAC, GPIB_NRFD, GPIB_DAV};

	// LCD pins
	
	// Serial pins

// GPIB mode
const int mode_idle = 0;
const int mode_listen = 1;
const int mode_talk = 2;
const int mode_secondary = 3;
int mode = mode_idle;

// delay length (in ms)
const long del_t = ;
const int itr_max = ;

// GPIB buffer setup
const int buff_size = 255;
int buff_GPIB[buff_size];
int buff_GPIB_head = 0;
int buff_GPIB_tail = 0;
int buff_GPIB_count = 0;

// Address
int add_own = ;

// setup fns
void setup_GPIB(void) {
	// pin mode setup
	listen_setup();
	interupt_setup();
	
	// empty buffers
	buff_GPIB_count = 0;
		// empty RS232 buffer	
}

void interupt_setup(void) {
	attachInterrupt(digitalPinToInterrupt(GPIB_IFC), run_IFC, FALLING);
}

void listen_setup(void) {
	// set data and command pins to input
	for (int ii = 7; ii >= 0; ii--) {
		pinMode(GPIB_DATA[ii], INPUT);
	}
	for (int ii = 4; ii >= 0; ii--) {
		pinMode(GPIB_CONTROL[ii], INPUT);
	}
	pinMode(GPIB_SRQ, OUTPUT);
	digitalWrite(GPIB_SRQ,HIGH);
	
	// setup handshakes
	pinMode(GPIB_DAV, INPUT);
	pinMode(GPIB_NRFD, OUTPUT);
	digitalWrite(GPIB_NRFD, LOW);
	pinMode(GPIB_NDAC, OUTPUT);
	digitalWrite(GPIB_NDAC, LOW);
}

void talk_setup(void) {	
	// set data pins to output
	for (int ii = 7; ii >= 0; ii--) {
		pinMode(GPIB_DATA[ii], OUTPUT);
	}
	for (int ii = 4; ii >= 0; ii--) {
		pinMode(GPIB_CONTROL[ii], INPUT);
	}
	
	// set EOI and SRQ to outputs
	pinMode(GPIB_EOI,OUTPUT);
	digitalWrite(GPIB_EOI,HIGH);
	pinMode(GPIB_SRQ,OUTPUT);
	digitalWrite(GPIB_SRQ,HIGH);
	
	// setup handshakes
	pinMode(GPIB_DAV, OUTPUT);
	digitalWrite(GPIB_DAV, HIGH);
	pinMode(GPIB_NRFD, INPUT);
	pinMode(GPIB_NDAC, INPUT);
}

void main_GPIB(void) {
	listen_GPIB();
}

void listen_GPIB(void) {
	int itr_count = 0;
	// set up pin input/output modes
	listen_setup();

	// set not ready for data to FALSE
	digitalWrite(GPIB_NRFD, HIGH);

	// wait until data is validated
	int itr_count = 0;
	dav = digitalRead(GPIB_DAV);
	while(dav == HIGH){
		dav = digitalRead(GPIB_DAV);
		delay(del_t);
		// if device times out, process GPIB buffer
		if(itr_count > itr_max) {
			digitalWrite(GPIB_NRFD, LOW);
			process_GPIB();
			itr_count = 0;
			digitalWrite(GPIB_NRFD, HIGH);			
		}
		itr_count++;
	}

	int data = read_GPIB();
	
	int atn = bitRead(data,9);
	int is_process = atn * (bitRead(data,6) | bitRead(data,5));
	
	if(atn == 1 || mode == mode_listen) {
		// data to buffer only if it is either a command or device is listening
		write_GPIB_buff(data);
	}
	
	if(is_process == 1) {
		// process GPIB buffer if any device is addressed (talk or listen) or untalk/unlisten
		process_GPIB();
	}
	
	// set ready for data to false
	digitalWrite(GPIB_NRFD, LOW);
	
	// set data accepted to true
	digitalWrite(GPIB_NDAC, HIGH);
	
	// wait until listener stops validating data
	dav = digitalRead(GPIB_DAV);
	while(dav == LOW) {
		dav = digitalRead(GPIB_DAV);
		delay(del_t);
		// timeout
			// add appropriate timeout
		itr_count++;
	}
	// set data accepted to false
	digitalWrite(GPIB_NDAC, LOW);
}

void talk_GPIB(int data) {
	
	// set up pin input/output modes
	talk_setup();
	
	// wait until listener is ready for byte, and data not accepted
	nrfd = digitalRead(GPIB_NRFD);
	ndac = digitalRead(GPIB_NDAC);
	while(nfrd == LOW || ndac == HIGH){
		nrfd = digitalRead(GPIB_NRFD);
		ndac = digitalRead(GPIB_NDAC);
		delay(del_t);
	}
	
	// write byte & controls to data lines
	write_GPIB(data);
	
	// set data valid to true
	digitalWrite(GPIB_DAV, LOW);
	
	// wait until data is accepted and listener is not ready for data
	nrfd = digitalRead(GPIB_NRFD);
	ndac = digitalRead(GPIB_NDAC);
	while(ndac == LOW || nrfd == HIGH){
		ndac = digitalRead(GPIB_NDAC);
		delay(del_t);
	}
	
	// set data valid to false
	digitalWrite(GPIB_DAV, HIGH);
}

void process_GPIB(void) {
	// Process all the commands/data present in the GPIB buffer
	while(buff_GPIB_count > 0){
		int buff = read_GPIB_buff();
		int comms = buff / 256;
		int data = buff % 256;
		
		int atn = bitRead(comms, 1);
		if (atn == 1){
			int command_mode = ((data / 32) % 4);
			switch command_mode {
				case mode_idle:
					int data_mod = data % 128;
					if(data_mod == 20){
						// device clear
						setup_GPIB();						
					}
					break;
					
				case mode_listen:
					int add = data % 32;
					if(add == add_own){
						// MLA
						mode = mode_listen;
					}
					
					if(add == 31){
						// unlisten
						mode = mode_idle;
					}
					break;
					
				case mode_talk:
					int add = data % 32;
					if(add == add_own){
						// MTA
						process_Serial();
					}
					
					// add untalk if necesaary
					break;
					
				case mode_secondary:
					// No commands here yet
					break;
								
			} 
		} else {
			// Send data straigh thru to Serial
		}
		
	}
}

void process_Serial(void) {
	// Send all the data in the serial buffer
	// for each piece of data in the Serial buffer, use talk_GPIB(data)
	
}

int read_GPIB(void) {
	// read control pins
	int data = 0;
	for (int ii = 4; ii >= 0; ii--) {
		data = data * 2;
		data_ii = digitalRead(GPIB_CONTROL[ii]);
		if(data_ii == LOW) data++;
	}	
	
	// read data pins
	for (int ii = 7; ii >= 0; ii--) {
		data = data * 2;
		data_ii = digitalRead(GPIB_DATA[ii]);
		if(data_ii == LOW) data++;
	}
	return data;
}

void write_GPIB(int data) {
	for(int ii = 0; ii <= 7; ii++){
		int data_ii = bitRead(data, ii);
		if(data_ii == 1){
			digitalWrite(GPIB_DATA[ii], LOW);
		}
		else{
			digitalWrite(GPIB_DATA[ii], HIGH);			
		}		
	}
	
	int data_EOI = bitRead(data, 12);
	if(data_EOI == 1){
		digitalWrite(GPIB_EOI, LOW);
	}
	else{
		digitalWrite(GPIB_EOI, HIGH);
	}
	
	int data_SRQ = bitRead(data, 10);
	if(data_SRQ == 1){
		digitalWrite(GPIB_SRQ, LOW);
	}
	else{
		digitalWrite(GPIB_SRQ, HIGH);
	}
}

void write_GPIB_buff(int data) {
	buff_GPIB[buff_GPIB_head] = data;
	buff_GPIB_head = (buff_GPIB_head + 1) % buff_size;
	buff_GPIB_count++;
}

int read_GPIB_buff(void) {
	int data = buff_GPIB[buff_GPIB_tail];
	buff_GPIB_tail = (buff_GPIB_tail + 1) % buff_size;
	buff_GPIB_count--;
	return data;
}

void run_IFC(void) {
	return;
}