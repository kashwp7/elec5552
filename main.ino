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

// mode
const int mode_listen = 1;
const int mode_process = 2;
int mode = mode_process;
bool skip_dav = FALSE;

// delay length (in ms)
const long del_t = ;
const int itr_max = ;

// buffer setup
const int buff_size = 16;
int buff_GPIB[buff_size];
int buff_GPIB_head = 0;
int buff_GPIB_tail = 0;
int buff_GPIB_count = 0;

// address and baud rate, + non-volatile memory locations
byte add_own;
const int add_loc = ;
long baud_own;
const int baud_size = ;
const long baud_lookup[] = {};
const int baud_loc = ;

// GPIB function definitions

const int func_table[] = {};

// Serial
void setup_serial(){
	Serial.begin(9600); 	//Debugging interface
	Serial1.begin(9600);	//MS 232 interface
}

// setup fns
void setup_Jacob(){
	idle_setup();
	interupt_setup();
	get_address();
	get_baud();
}

void main_GPIB(){
	switch (mode) {
		case mode_process:
			process();
			break;
		case mode_listen:
			listen_GPIB();
			break;
	}
	
	
}

void interupt_setup() {
	attachInterrupt(digitalPinToInterrupt(GPIB_IFC), run_IFC, FALLING);
}

void listen_setup() {
	// set data and command pins to input
	for (int ii = 7; ii >= 0; ii--) {
		pinMode(GPIB_DATA[ii], INPUT);
	}
	for (int ii = 4; ii >= 0; ii--) {
		pinMode(GPIB_CONTROL[ii], INPUT);
	}
	
	// setup handshakes
	pinMode(GPIB_DAV, INPUT);
	pinMode(GPIB_NRFD, OUTPUT);
	digitalWrite(GPIB_NRFD, LOW);
	pinMode(GPIB_NDAC, OUTPUT);
	digitalWrite(GPIB_NDAC, LOW);
}

void talk_setup() {
	
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

void idle_setup() {
	// set all pins to input
	for (int ii = 7; ii >= 0; ii--) {
		pinMode(GPIB_DATA[ii], INPUT);
	}
	for (int ii = 4; ii >= 0; ii--) {
		pinMode(GPIB_CONTROL[ii], INPUT);
	}
	pinMode(GPIB_DAV, INPUT);
	pinMode(GPIB_NRFD, INPUT);
	pinMode(GPIB_NDAC, INPUT);
}

void listen_GPIB() {
	int itr_count = 0;
	// set up pin input/output modes

	// checks if data has been prevalidated
	if(skip_dav){
		skip_dav = FALSE;		
	}
	else{
		listen_setup();
		bool dav_val = run_dav();
		if(!dav_val) {
			mode = mode_process;
			return;
		}			
	}
	
	int data = read_GPIB();
	
	//	write data to buffer
	write_GPIB_buff(data);
	
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
		itr_count++;
	}
	// set data accepted to false
	digitalWrite(GPIB_NDAC, LOW);
}

void talk_GPIB(int data){
	
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

void process_GPIB(){
	skip_dav = run_dav();
	if(skip_dav) return;
	// read buffer
	// check for commands
	// otherwise pass on directly
}

int read_GPIB(){
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

void write_GPIB(int data){
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

void write_GPIB_buff(int data){
	buff_GPIB[buff_GPIB_head] = data;
	buff_GPIB_head = (buff_GPIB_head + 1) % buff_size;
	buff_GPIB_count++;
}

int read_GPIB_buff(){
	int data = buff_GPIB[buff_GPIB_tail];
	buff_GPIB_tail = (buff_GPIB_tail + 1) % buff_size;
	buff_GPIB_count--;
	return data;
}

void run_IFC(){
	return;
}

bool out run_dav(){
	// check buffer overflow
	if(buff_GPIB_count >= buff_size){
		mode = mode_process;
		return FALSE;
	}
	// check for data
		// set not ready for data to false
	digitalWrite(GPIB_NRFD, HIGH);
	
	int itr_count = 0;
	dav = digitalRead(GPIB_DAV);
	while(dav == HIGH){
		dav = digitalRead(GPIB_DAV);
		itr_count++;
		if(itr_count > itr_max){
			mode = mode_process;
			digitalWrite(GPIB_NRFD, LOW);
			return FALSE;
		}
	}
	return TRUE;
}

void get_address(){
	add_own = EEPROM.read(add_loc);
}

void set_address(byte add_new){
	add_own = add_new;
	EEPROM.update(add_loc, add_new);	
}

void get_baud(){
	int ii = EEPROM.read(baud_loc);
	baud_own = baud_lookup[ii];
	// set RS-232 rate
}

void set_baud(byte baud_new){
	if(baud_new < baud_size){
		baud_own = baud_lookup[baud_new];
		EEPROM.update(baud_loc, baud_new);
	}
	// set RS-232 rate
}
