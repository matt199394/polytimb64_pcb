#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
//#define OLED_RESET    A3 
#define SCREEN_ADDRESS 0x3C 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

int X = 0;
int X_original = 0;    
int X_original_2 = 0; 
int CO = 1;   // Antes era CO = 0;
int pushRelease = 0;					
int voice[4];   // Array 0 - 3
int tempo[4];  // Array 0 - 3    	

//int modoActual = 1;		 
int mode[6];				//02.07.2021 Array 0 - 6, we are going to use 1 - 6.	


int intervallo;
int maximo;
int minimo;
int replaced;   
int Sub = 1;						//The initial value could be 0, it doesn't matter.
int SubPin = 0;         //The initial value could be 1, it doesn't matter.   
int xx = 0;             //It is not being used anywhere.   
int top = 0;            
int split = 0;          
int toTheRight = 0;    

int cutting = 0;          		//09.08.2018 It would be clearer to call it pointcut.
int pulsedCut_1 = 0;   	//09.08.2018 So that it immediately stops reading the pulse once it has been pressed.
int pulsedCut_2 = 0;   	//02.07.2021
int pulsedCut_3 = 0;   	//02.07.2021
int invertedCut = 0;			//03.09.2018 To determine whether the keyboard halves are reversed (1) or not (0, as before).

int middleKeyboard = 0;
int keyboardSwitch = 0;
int Mode = 0;
int Gate = 0;				//06.03.2023 It seems that it is not used, it only appears in one place, and that's it.

int ledCounter = 0;		//30.06.2021

unsigned long previousMillis = 0;  		// will store last time LED was updated				
unsigned long currentMillis;					// Se usa en previousMillis = currentMillis; 	
const long Interval = 500;  					// interval at which to blink (milliseconds)	

// MIDI section

int noteON = 144;     		//144 = 10010000 Note on command 			
int noteOFF = 128;    		//128 = 10000000 Note off command 		
int ControlChange = 176; 	//176 = 10110000 Continuous controller, aka, Control Change	
int CCmode = 97;
int CCsplit = 96;
int CCswap = 66;
int setCC = 0;				
int blinks = 0;					

byte commandByte;
byte noteByte;
byte velocityByte;
byte buff;

char str[20];
// end MIDI

void setup() {
  pinMode(2, OUTPUT); 	//pin 1 C64
  pinMode(3, OUTPUT); 	//pin 2 C64
  pinMode(4, OUTPUT); 	//pin 4 C64
  pinMode(5, OUTPUT); 	//pin 8 C64
  pinMode(6, OUTPUT); 	//pin 16 C64
  pinMode(7, OUTPUT); 	//pin 32 C64
  pinMode(8, OUTPUT); 	//pin 64 C64
  pinMode(9, INPUT);  	//pin 128 C64
  
  pinMode(A1, OUTPUT);	 
  pinMode(A2, OUTPUT);	 
  pinMode(A3, OUTPUT);	 

  
  Serial.begin(31250);   

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
     //SDA A4
     //SCL A5
    for(;;); // Don't proceed, loop forever
  }
  
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  display.clearDisplay();
  display.display();

  digitalWrite(A3, HIGH);	//Mode 1  
   
  mode[0] = 1;					 
  mode[1] = 3;					 
  mode[2] = 9;					 
  mode[3] = 7;					 
  mode[4] = 9;					 
  mode[5] = 7; 					 
  mode[6] = 13;					 

  digitalWrite(8, LOW);		 

}

void loop() {

	top = 0;                         //This block is used only in combination with NewTest, which is not currently called from anywhere.
  for (int n = 1; n < 4; n++){     
   	if (voice[n] > top){        
     	top = voice[n];
     	split = top - 5;}
  }                                //end NewTest.

	if (CO == 0 && digitalRead(9) == HIGH) {

	  X = -1;		// It will be used to see if any of the conditions have not been met. It would look cleaner and more intelligible with Switch Case.
	  
	  switch (Mode) {

		case 1:
	   		X = Mode;				  	// You are going to play a note of the chord for voice 1.
	  			voice[1] = noteByte; 	//If bit 5 = 1 (32) indicates that voice 3 is going to be coupled to voice 1 to modulate it, but this is done in the C64.
	  			break;
		
	 	case 0:
	 		 	X = Mode;			  		// You are going to drop a note of the chord through voice 1.
	 			voice[1] = 0; 			// Same with bit 5
	 			break;
		
	 	case 3: 																										// This is the most "polyphonic" variant with 2 or 3 independent notes (or just 1, if necessary, review).
	 		 	if (voice[1] == 0) { 			X = 3;	voice[1] = noteByte; }		 
	 			else if (voice[2] == 0) { X = 5;	voice[2] = noteByte; }
	 			else if (voice[3] == 0) { X = 7; 	voice[3] = noteByte; }
	 			else {OldFunka(); }
	 			break;

	 	case 2:
	 		 	if (voice[1] == noteByte) {				X = 2; voice[1] = 0;}
	 			else if (voice[2] == noteByte) { 	X = 4; voice[2] = 0;}
	 			else if (voice[3] == noteByte) { 	X = 6; voice[3] = 0;}
	 			break;
		
	 	case 5:
	 			X = 5;							  // The last mode (the most complicated). Only voice[2] is available to play on one half of the keyboard. 
	 			voice[2] = noteByte; 	// In the other there is the voice[1] as chords and the voice[3] coupled by modulating it. 
	 			break;                // It works, but it's not very interesting and it makes a strange noise

	 	case 4:
	 			if (voice[2] == noteByte) { 			X = 4; voice[2] = 0; }
	 			break;
									
	 	case 7: 																													// voice[2] and voice[3] are independent in one half, in the other half there can be chords or voice[1] independent,
	 	 		if (voice[2] == 0) { 							X = 5; voice[2] = noteByte; }		// but separated in half. This last mode is one of the ones we were missing. I haven't tried it yet.
	 			else if (voice[3] == 0) { 				X = 7; voice[3] = noteByte; }
	 			else { OldFunka(); }
	 			break;

	 	case 6: 
	 			if (voice[2] == noteByte) { 			X = 4; voice[2] = 0; }
	  		else if (voice[3] == noteByte) { 	X = 6; voice[3] = 0; }
	  		break;
	 	
		case 9: 
	  		X = 7;								  // The classic mode that we had until now with Suboscillator 2 modulating to 3. Below, the sending of 2 packets
	 			voice[3] = noteByte;		// will be identified by Mode == 9 || Mode == 8.
	 			voice[2] = noteByte;
	 			break;
		
	 	case 8: 																			 
	 			if (voice[3] == noteByte) { X = 6;
	 																voice[3] = 0; 		// The 3 is placed before the 2 (also in case 9) to remind us that the osc 2 modulates the 3. // 03.05.2023
	 																voice[2] = 0; } 	// Obviously, for execution purposes it doesn't matter exactly.
	 			break;
		
	 	case 11:
	 	 		X = 3;								// 1 single note voice[1] on one half, 2 coupled notes voice[2] and voice[3] on the other half. The simplest of the two new modes.
	 			voice[1] = noteByte;
	 			break;
		
	 	case 10: 																			// No había comentarios sobre esto, ni hace nada cuando no se cumple la condición.
	 			if (voice[1] == noteByte) { X = 2; voice[1] = 0; }
	 		 	break;

	 	case 13:
	 			X = 7;								// The 3 voices coupled.
	 			voice[1] = noteByte;
	 			voice[2] = noteByte;
	 			voice[3] = noteByte;
	 			break;

	 	case 12:
	 			if (voice[1] == noteByte) { X = 6;
	 		 											 			voice[1] = 0;
	 		 											 			voice[2] = 0;
	 		 											 			voice[3] = 0; }
	 			break;
  
   	}	// end Switch(Mode)

      if (X > -1)       
        { X_original = X; 		// X can enter 1, 0, 3, 2, 5, 4, 7 or 6. I think (review) that this is worth C64 as Voice 1 with pseudoacordes, Voice 1 normal, Voice 2 or Voice 3.
          X_original_2 = X;		// Yes, two X_original are needed, because it can enter the second mode block,
															// in the third, or in both. What you should do is look for a cleaner method.

            HighLow();                	// Send the first package. 
            digitalWrite(8, HIGH);
            do{} while (digitalRead(9) == HIGH);		// He seemed somewhat unstable, but it seems like the usual offenses against the novtempo.
            X = noteByte - 36;
            HighLow();          	// Send the second packet.
            digitalWrite(8, LOW);
            do{} while (digitalRead(9) == LOW);   

            
            if (Mode == 9 || Mode == 8 || Mode == 13 || Mode == 12)   	 // ***** OSC 2 coupled to OSC 3 *****, or also the three OSCs coupled together.
              {	X = X_original - 2;   		     // X comes from being 6 or 7 and stays at 4 or 5.
               	HighLow();                	   // Send the first package.
               	digitalWrite(8, HIGH);
               	do{} while (digitalRead(9) == HIGH);		 

               	X = noteByte - 36;  
                HighLow();          	   // Send the second packet
                digitalWrite(8, LOW);
                do{} while (digitalRead(9) == LOW);			 
            	}
			

            if (Mode == 13 || Mode == 12)   // *****The three OSCs coupled together*****
              {	X = X_original_2 - 4;   		    // X comes from being 6 or 7 and stays at 2 or 3.
               	HighLow();                	    // Send the first package.
               	digitalWrite(8, HIGH);
               	do{} while (digitalRead(9) == HIGH);		// Oremos 2... 01.03.2023

                X = noteByte - 36;  
                HighLow();          	    // Send the second packet
                digitalWrite(8, LOW);
                do{} while (digitalRead(9) == LOW);			 
            	}
            
            CO = 1;
        }   // end of if (X > -1)
	}			// end of if (CO == 0 && digitalRead(9) == HIGH)  


	// Here you read the notes from the MIDI keyboard when bit 7 (128, pin 9) of 56577 indicates that the C64 has finished processing.
	if (digitalRead(9) == HIGH  && Serial.available() > 2) {readMIDI();}

	if (setCC > 0) {
			currentMillis = millis();
			if (currentMillis - previousMillis >= Interval) {
    			previousMillis = currentMillis;			// save the last time you blinked the LED
					if (blinks == 1) {
							if (setCC & 1){digitalWrite(A3, HIGH);} // else { digitalWrite(A3, LOW);} 		// digitalWrite(A3, LOW);
							if (setCC & 2){digitalWrite(A2, HIGH);} // else { digitalWrite(A2, LOW);}		// digitalWrite(A2, LOW);
							digitalWrite(A1, LOW);		// Por una cuestión de orden
							blinks = 0; }
					else {
							digitalWrite(A3, LOW);		// if (setCC & 1){digitalWrite(A3, HIGH);}		// digitalWrite(A3, HIGH);
							digitalWrite(A2, LOW);		// if (setCC & 2){digitalWrite(A2, HIGH);}		// digitalWrite(A2, HIGH);
							digitalWrite(A1, LOW);							// digitalWrite(A1, HIGH);		//Ponemos este A1, LOW por una cuestión de orden.
							blinks = 1;
					}
			} // end of if (currentMillis - previousMillis >= Interval)
	} // end of if (setCC > 0)

}	// end of  void loop()

void readMIDI(){

	// // Be careful, despite the experiments done with buff and arturia + korg, if in this condition we set == 3 and touch
	// three notes simultaneously, he only recognizes one and is left hanging. It seems to go better with > 2. Pseudochords are more than 3!
	
	commandByte = Serial.read();   	//Read byte 1
	noteByte = Serial.read();      	//Read byte 2         Be careful, having the USB plugged in while playing can cause all values to be = 255.
	velocityByte = Serial.read();  	//Read byte final 		In this case, unplug the USB and use a battery for power.
	                                // For commandByte == CC, the second byte is NOT the note, and the third is NOT the speed, but we leave those names anyway.

  display.clearDisplay();
  display.setCursor(0,0);
  sprintf(str, "%d %d %d", commandByte, noteByte, velocityByte);
  display.print(str);
  display.fillRect(0,20, velocityByte, 5, SSD1306_WHITE);
  display.display();
    
	if (setCC > 0) {										// ***** Nuevo 06.03.2023 *****

		// 	currentMillis = millis();
		// 	if (currentMillis - previousMillis >= Interval) {
    // 			previousMillis = currentMillis;			// save the last time you blinked the LED
		// 		if (digitalRead(A3) == HIGH) {
		// 			if (setCC & 1){digitalWrite(A3, LOW);}		// digitalWrite(A3, LOW);
		// 			if (setCC & 2){digitalWrite(A2, LOW);}		// digitalWrite(A2, LOW);
		// 			digitalWrite(A1, LOW); }
		// 		else {
		// 			if (setCC & 1){digitalWrite(A3, HIGH);}		// digitalWrite(A3, HIGH);
		// 			if (setCC & 2){digitalWrite(A2, HIGH);}		// digitalWrite(A2, HIGH);
		// 			digitalWrite(A1, LOW);							// digitalWrite(A1, HIGH);		//Ponemos este A1, LOW por una cuestión de orden.
		// 	}
		// } // Final de if (currentMillis - previousMillis >= Interval)
    
  		if (commandByte == ControlChange) {						
				switch (setCC) {
				case 3:
					CCmode = noteByte;
					setCC = 2;
					break;
				case 2:
					CCsplit = noteByte;
					setCC = 1;
					break;
				case 1:
					CCswap = noteByte;
					setCC = 0;
					digitalWrite(A3, LOW);							// Y devuelve las luces a la normalidad del modo.
					digitalWrite(A2, LOW);
					digitalWrite(A1, LOW);
					if (mode[0] & 1){digitalWrite(A3, HIGH);}
					if (mode[0] & 2){digitalWrite(A2, HIGH);}
					if (mode[0] & 4){digitalWrite(A1, HIGH);}		// Y devuelve las luces a la normalidad del modo.
					break;
				} // end of switch
			}		// end of if (commandByte == ControlChange)
		return;
	}	// end of if (setCC > 0)							 

	
	if (commandByte == ControlChange) {
		if (noteByte == CCmode){ 									// 97 65, button B6, to test if funka.
			mode[0] = mode[0] + 1;
			//mode[0] = 2;
			//ledCounter = ledCounter;
			if (mode[0] > 6){mode[0] = 1;}
			digitalWrite(A3, LOW);
			digitalWrite(A2, LOW);
			digitalWrite(A1, LOW);
			if (mode[0] & 1){digitalWrite(A3, HIGH);}
			if (mode[0] & 2){digitalWrite(A2, HIGH);}
			if (mode[0] & 4){digitalWrite(A1, HIGH);}
		}
		else if (noteByte == CCsplit) {								// 96
			cutting = cutting + 12;
      if (cutting == 36){cutting = -24;}
		}
		else if (noteByte == CCswap) {								// 66
			if (keyboardSwitch == 1){keyboardSwitch = 0;}
      else if (keyboardSwitch == 0){keyboardSwitch = 1;}		 
		}
		else {																 
			setCC = 3;
			digitalWrite(A3, LOW);
		} 										 
			
		//}	// Final if (commandByte == ControlChange

		return; 	//https://www.arduino.cc/en/Reference.Return		// When leaving here, CO should still be = 1.
	}	// end of if (commandByte == ControlChange

    //mode[1] = 3		Then it stays as it was (actually there is no after, because it only enters the second block if Mode > 3)
    //mode[2] = 9		Then it can be 11
    //mode[3] = 7		Then it can be 1
    //mode[4] = 9		Then it can be 1
    //mode[5] = 7		Then it can be 11
    //mode[6] = 13	Then it stays as it was

    //Nowhere does it check if commandByte == noteON. It simply processes the number of the key that has been pressed (noteByte) and the Mode that is selected.
    //It considers by default to be noteON. After processing the note and the Mode, if it turns out that commandByte == noteOFF, it removes 1 from the Mode, which indicates gate off.

    // Come on, clarify in a comment the differences between Mode, mode[] and X. They are different and necessary; ActualMode seems to be on the rocks.

	CO = 0;
	// modoActual = mode[0];						//modoActual es uno de los modos para el usuario, del 1 al 6. PARECE que NO SE ESTÁ USANDO en ningún sitio.
	Mode = mode[mode[0]]; 				//Modo es uno de los modos en Arduino: 3, 9, 7, 9, 7, 13, que luego pueden cambiar (algunso) a 1 u 11.
		
		
	middleKeyboard = 0;			// También será 0 cuando NO haya que partir el teclado (Modo 1) pero lo ignorará.
														// 0 -> pasa la nota a la mitad derecha, 1 -> pasa la nota a la mitad izquierda.
	if (Mode > 3) {
		
		if (noteByte < (60 + cutting) && keyboardSwitch == 0) {middleKeyboard = 1; }
 		else if (noteByte > (59 + cutting) && keyboardSwitch == 1) {middleKeyboard = 1; }

 		if (middleKeyboard == 1) {
			if (mode[0] == 2){Mode = 11; }				// 2 - Viene de ser Modo = 9	Aquí hacen falta tres condiciones para diferenciarlo de 6.
			else if (mode[0] == 3){Mode = 1; }		// 3 - Viene de ser Modo = 7
			else if (mode[0] == 4){Mode = 1; }		// 4 - Viene de ser Modo = 9
			else if (mode[0] == 5){Mode = 11; }		// 5 - Viene de ser Modo = 7	Nuevo, funka bien, pero nos faltan las tres posiciones.
		}
	}

	//if (commandByte == noteON) 	{Gate = 1; }
	if (commandByte == noteOFF) {
		Gate = 0;
		Mode = Mode - 1;
	}
	
}

void HighLow(){
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);

  if (X & 1){digitalWrite(2, HIGH);}
  if (X & 2){digitalWrite(3, HIGH);}
  if (X & 4){digitalWrite(4, HIGH);}
  if (X & 8){digitalWrite(5, HIGH);}
  if (X & 16){digitalWrite(6, HIGH);}
  if (X & 32){digitalWrite(7, HIGH);}
}

void OldFunka(){																		//Función para sustituir una de las 3 notas tocadas, cuando se toca una cuarta.
  minimo = 100;  // 100, por poner algo menor que lo cual siempre habrá algo.

  if (Mode == 3)
  {intervallo = voice[1] - noteByte;  															// abs(a++);   // avoid this - yields incorrect results
  minimo = abs(intervallo);
  replaced = 1;}                                        
           
  intervallo = voice[2] - noteByte;
  intervallo = abs(intervallo);
  if (intervallo < minimo)
  {minimo = intervallo;
  replaced = 2;}             
           
  intervallo = voice[3] - noteByte;
  intervallo = abs(intervallo);
  if (intervallo < minimo)
  {replaced = 3;}                  

  voice[replaced] = noteByte;
  X = (replaced * 2) + 1;  //X = (replaced * 16) + 1; ***
}

void NewTest(){ 																		//Función para sustituir una de las 3 notas tocadas, cuando se toca una cuarta.

// Si ha entrado aquí, es porque están todos los canales ocupados.
// Lo primero es ver si la nota tocada está a la derecha del split.

  toTheRight = 0;
  for (int n = 1; n < 4; n++){     
    if (voice[n] > split){toTheRight = ++toTheRight;}
  }                               

  if (toTheRight > 1)
  {
    if (tempo[3] > split){replaced = tempo[3];}
    else if (tempo[2] > split){replaced = tempo[2];}
    else if (tempo[1] > split){replaced = tempo[1];}

    if (voice[1] = replaced){replaced = 1;}
    else if(voice[2] = replaced){replaced =2;}
    else if(voice[3] = replaced){replaced =3;}

    voice[replaced] = noteByte;
    X = (replaced * 2) + 1;  //X = (replaced * 16) + 1; ***
  }

  else    //Es decir, a la derecha del split hay como mucho una nota (o puede que ninguna).
  {
    OldFunka();
  }

}

//Send MIDI message. No hace falta para el C64, porque no se le envía MIDI, sino bits por los pins 2 - 7 (8 y 9).
void MIDImessage(int command, int MIDInote, int MIDIvelocity) {
  Serial.write(command);       //Envía note on o note off command        
  Serial.write(MIDInote);      //Envía pitch data                        
  Serial.write(MIDIvelocity);  //Envía velocity data
}
