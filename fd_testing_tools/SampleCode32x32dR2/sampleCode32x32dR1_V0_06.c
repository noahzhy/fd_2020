
/************************************************************************/
/*		Version  0.02  													*/
/*		Author:	 Michael Schnorr										*/
/*		Date:	 05.09.2018												*/
/*              						*/
/************************************************************************/

#define Pixelforth 256
#define SPALTE 32
#define PAGESIZE 32
#define AdrExponent 0x0B

unsigned char data[258],data2[258];
unsigned char *p,*p2;
unsigned char giveOut,currentBuffer=0,test[2][(unsigned short)DATALength*2], currentReadBlock=0, GetBlindFrame=0, ReadBlindFrame=0;	//DataLength=1098
unsigned char SENSORREV1=1, VddSampledNow=0, VddSampledBefore=0, singleframe, firstrun, VddScalingOff, NrOfDefPix, DeadPixMask[MAXNROFDEFECTS];
unsigned short eloffstack[ELAMOUNT][StackSize];	//ELAMOUNT=256; StackSize=10 (arbitrary, should be chosen to match the data memory of the conctroller)
unsigned short gradScale,TA,Voltage[2][Pixel+PTATamount+ELAMOUNT+2],FramesBeforeEl=0;	
unsigned short SetMBITCalib,SetMBITUser,SetBPACalib,SetBPAUser,SetPUCalib,SetPUUser,Resolution, TableNumberSensor;
unsigned short SetBIASCalib,SetCLKCalib,SetBIASUser,SetCLKUser, DeadPixAdr[MAXNROFDEFECTS], epsilon;
signed short ThGrad[Pixel],VddOff[ELAMOUNT],VddGrad[ELAMOUNT];;
signed short ThOff[Pixel];
unsigned long PixC[Pixel],DevID;
unsigned short VddStack[VddStackAmount],VddStackPointer,VddRef[2],PTATThermals[2];
unsigned char VddScaling;
unsigned short GlobalGain;
char GlobalOffset;
char GetVddMeas,RefCal,pPTATStack,pVddSensorStack;
unsigned short PTATStack[STACKSIZEPTAT],VddSensorStack[STACKSIZEVDD];
volatile char FirstFrame=0, INT1occ=0;
volatile unsigned char pelStack, useStack;
volatile unsigned int VDD;
unsigned int pixsum, maskcnt, adaptedAdr;
float PTATGrad, PTATOff;

//I2C1STATbits just returns status of I2C
//I2C1RCV is receive buffer of I2C
//I2C1CONbits, I2C1RCV and I2C1STATbits are dependend on the microchip and the I2C registers
int main(void){

//now wake up the sensor
I2Creg.adr=0x1A;
I2Creg.read=0x00;
I2Creg.Config=0x01;
I2Creg.sfrdata=0x01;
WriteConf(I2Creg);

DelayMs(5);		//Give the module some time to wake up

//read the calibration data from the sensor
ReadCalibData();

InitMBITTRIM(0);

DelayMs(5);

InitBIASTRIM(0);

DelayMs(5);

InitBPATRIM(0);

DelayMs(5);

InitPUTRIM(0);

DelayMs(5);

InitCLKTRIM(0);		//Resolution (MBIT) has to be set before!!!

do{
	//Readout and start the sensor.
	ReadSensor via e.g. Timer3
	//if temperature is required do the following (all done in the StartStreaming function)
	//1. determine ambient temperature via PTAT
	//2. substract the thermal offset for each pixel
	//3. determine the object temperature from the look-up table
	StartStreaming(1,1);
}while(1);
}

/********************************************************************
 * Function:        void T3_Interrupt_ISR(void)
 *
 * Description:     This function reads the Voltage of all IR Pixel and the PTAT
 *
 * Dependencies:    
 *******************************************************************/
void __ISR(_TIMER_3_VECTOR, ipl5)T3_Interrupt_ISR(void){
unsigned short k,i,LineVoltage;
static unsigned char LastReadBlock;
TAsicSFR I2Creg;

/*	Order of Voltage[][i]:
*	0 to Pixel-1 (1023):						Pixelvoltage
*	Pixel(1024) to Pixel+255 (1279):			el. Offset
*	1280:										VDD
*	1281:										TAmb
*	PTATSTARTADSRESS (1282) to DATALength-1:	PTAT
*/

	//Wait until conversion is finished
	do{
		p=data;
		I2Creg.adr=0x1A;
		I2Creg.read=0x00;
		I2Creg.Config=0x02;
		Read_RAM(data,I2Creg,0x01);
	}while((data[0]&0x01)!=0x01);

	//Now readout the sensor either active or blind block
	I2Creg.adr=0x1A;
	I2Creg.read=0x00;
	I2Creg.Config=0x0A;
	p=data;
	Read_RAM(data,I2Creg,Pixelforth+2);

	I2Creg.adr=0x1A;
	I2Creg.read=0x00;
	I2Creg.Config=0x0B;
	p2=data2;
	Read_RAM(data2,I2Creg,Pixelforth+2);

	//and start now already the sensor again to shorten the calculation and UDPPutArray time
	LastReadBlock=currentReadBlock;
	currentReadBlock++;			//get the next block
	if(GetBlindFrame){
		if(FirstFrame){
			ReadBlindFrame=1;
			GetBlindFrame=1;
			currentReadBlock=0;
		}
		else{
			ReadBlindFrame=1;
			GetBlindFrame=0;
			currentReadBlock=0;
		}
	}

	if(currentReadBlock>3){
		currentReadBlock=0;
		FramesBeforeEl++;
		if(FramesBeforeEl>(unsigned short)GetElEveryFrameX){
			GetBlindFrame=1;		//get a blind frame now
			FramesBeforeEl=0;
		}
	}
	
	I2Creg.adr=0x1A;
	I2Creg.read=0x00;
	I2Creg.Config=0x01;

	if(!GetBlindFrame){
		if(SENSORREV1){
			if(!GetVddMeas){
				I2Creg.sfrdata=(currentReadBlock<<4)|0x09;
				VddSampledNow=0;
			}	
			else{
				I2Creg.sfrdata=(currentReadBlock<<4)|0x0D;		//start Vdd-Meas next
				VddSampledNow=1;
			}
		}
		else
			I2Creg.sfrdata=(currentReadBlock<<4)|0x09;
	}
	else{
		I2Creg.sfrdata=0x0B;			//blind frame
	}

	WriteConf(I2Creg);
	TMR3 = 0x00;					// Clear 32-bit Timer (msw)
	TMR2 = 0x00; 					// Clear 32-bit Timer (lsw)

	if(!ReadBlindFrame){
		//now push the data into the stack
		if(SENSORREV1){
			if(VddSampledBefore){
				VddSensorStack[pVddSensorStack]=(*(unsigned char*)(p++))<<8;
				VddSensorStack[pVddSensorStack]|=(*(unsigned char*)(p++)&0x00FF);
				pVddSensorStack++;
			}
			else{
				PTATStack[pPTATStack]=(*(unsigned char*)(p++))<<8;
				PTATStack[pPTATStack]|=(*(unsigned char*)(p++)&0x00FF);
				Voltage[currentBuffer][PTATSTARTADSRESS+LastReadBlock*2]=PTATStack[pPTATStack];
				pPTATStack++;
			}
		}
		else{
			Voltage[currentBuffer][PTATSTARTADSRESS+LastReadBlock*2]=(*(unsigned char*)(p++))<<8;
			Voltage[currentBuffer][PTATSTARTADSRESS+LastReadBlock*2]|=(*(unsigned char*)(p++)&0x00FF);
		}
		for(i=0;i<4;i++){
			LineVoltage=LastReadBlock*PixelEighth+i*COLUMN;
			for(k=0;k<COLUMN;k++){
				Voltage[currentBuffer][k+LineVoltage]=(*(unsigned char*)(p++))<<8;
				Voltage[currentBuffer][k+LineVoltage]|=(*(unsigned char*)(p++)&0x00FF);
			}
		}	
	
		//now get the second block
		if(SENSORREV1){
			if(VddSampledBefore){
				VddSensorStack[pVddSensorStack]=(*(unsigned char*)(p2++))<<8;
				VddSensorStack[pVddSensorStack]|=(*(unsigned char*)(p2++)&0x00FF);
				pVddSensorStack++;
				if(pVddSensorStack==STACKSIZEVDD)
					pVddSensorStack=0;
			}
			else{
				PTATStack[pPTATStack]=(*(unsigned char*)(p2++))<<8;
				PTATStack[pPTATStack]|=(*(unsigned char*)(p2++)&0x00FF);
				Voltage[currentBuffer][PTATSTARTADSRESS+LastReadBlock*2+1]=PTATStack[pPTATStack];
				pPTATStack++;
				if(pPTATStack==STACKSIZEPTAT)
					pPTATStack=0;
			}
			VddSampledBefore=VddSampledNow;
		}
		else{
			Voltage[currentBuffer][PTATSTARTADSRESS+LastReadBlock*2+1]=(*(unsigned char*)(p2++))<<8;
			Voltage[currentBuffer][PTATSTARTADSRESS+LastReadBlock*2+1]|=(*(unsigned char*)(p2++)&0x00FF);
		}
		for(i=0;i<4;i++){
			LineVoltage=(3-LastReadBlock)*PixelEighth+(3-i)*COLUMN+Pixel/2;
			for(k=0;k<COLUMN;k++){
				Voltage[currentBuffer][k+LineVoltage]=(*(unsigned char*)(p2++))<<8;
				Voltage[currentBuffer][k+LineVoltage]|=(*(unsigned char*)(p2++)&0x00FF);
			}
		}
	}
	else{
		if(FirstFrame){
			FirstFrame--;
		}
		else{
			ReadBlindFrame=0;
			currentReadBlock=0;		//not needed, is set before to 0
			//first PTAT, ignore it here
			p+=2;
			for(k=0;k<ELAMOUNTHALF;k++){
				eloffstack[k][pelStack]=(*(unsigned char*)(p++))<<8;
				eloffstack[k][pelStack]|=(*(unsigned char*)(p++)&0x00FF);	// Es gibt 128 Blind Pixel je Hälfte!!!
			}
			//first PTAT, ignore it here
			p2+=2;
			for(i=0;i<4;i++){
				LineVoltage=(3-i)*COLUMN+ELAMOUNTHALF;
				for(k=0;k<COLUMN;k++){
					eloffstack[k+LineVoltage][pelStack]=(*(unsigned char*)(p2++))<<8;
					eloffstack[k+LineVoltage][pelStack]|=(*(unsigned char*)(p2++)&0x00FF);	// Es gibt 128 Blind Pixel je Hälfte!!!
				}
			}
			pelStack++;
			if(pelStack==StackSize){
				pelStack=0;
				useStack=1;
			}
		}
	}

	if(LastReadBlock==3){		//now push the data out
		//GetVDD(0);
		//Voltage[currentBuffer][VDDADDRESS]=(unsigned short)VDD/DIVIDEFACTORVDD;	//scale it back to 1 mV resolution
		if(!SENSORREV1){
			//fill the buffer
			VddStackPointer++;
			if(VddStackPointer>=VddStackAmount)
				VddStackPointer=0;
			VddStack[VddStackPointer]=VDD;
		}
		currentBuffer^=0x01;
		giveOut=1;	
		if(!GetBlindFrame){
			if(SENSORREV1){		//copy this for the first tests simply into PTAT with every other frame
				GetVddMeas+=1;
				if(GetVddMeas>2)
					GetVddMeas=0;
			}
		}
		if(SingleFrame){
			SingleFrame++;
		}
	}

	IFS0bits.T3IF=0;


return;
}
/********************************************************************
 * Function:        void StartStreaming(void)
 *
 * Description:     This function starts the streaming of the sensor
 *                  main sequence can be seen in the interrupt
 *
 * Dependencies:    
 *******************************************************************/
void StartStreaming(char Temps,char Stream){
unsigned int k,i;
unsigned char outbuffer;
TAsicSFR I2Creg;
unsigned long PTATLong;
signed long VoltageLong,PowerGradScale,NewDeltaVdd;
signed short VoltageSigned,DeltaVdd;
unsigned long aveloff,Vddlong;
signed long long VoltageLongLong;
unsigned long long DividerVdd,DividerVdd2;

firstrun=1;
currentReadBlock=0;
//Use a 32Bite Timer to synchronize with the framerate of the sensor
T3CONbits.TON = 0;				// Stop any 16-bit Timer3 operation
T2CONbits.TON = 0;				// Stop any 16/32-bit Timer2 operation
T2CONbits.T32 = 1;				// Enable 32-bit Timer mode
T2CONbits.TCKPS = 0x00;			// Select 1:1 Prescaler (=000)
T2CONbits.TCS = 0;				// Select internal instruction cycle clock
T2CONbits.TGATE = 0;			// Disable Gated Timer mode
TMR3 = 0x00;					// Clear 32-bit Timer (msw)
TMR2 = 0x00; 					// Clear 32-bit Timer (lsw)
//take a little bit lower value and then check the status flag in the int
PR3 = (unsigned int)((unsigned int)FCY*((float)tFR*0.9)/1000.0);		// Load 32-bit period value (msw)
PR2 = (unsigned int)((unsigned int)FCY*((float)tFR*0.9)/1000.0); 		// Load 32-bit period value (lsw)
// IPC3bits.T3IP = 0x04;			// Set Timer3 Interrupt Priority Level
IFS0bits.T3IF = 0;				// Clear Timer3 Interrupt Flag
IEC0bits.T3IE = 1;				// Enable Timer3 interrupt
T2CONbits.TON = 1;				// Start 32-bit Timer
giveOut=0;

//fill the PTATStack and VddSensorStack somewhere first, otherwise the first calculated data will be wrong

I2Creg.adr=0x1A;
I2Creg.read=0x00;
I2Creg.Config=0x01;
I2Creg.sfrdata=0x0B;		//always start with block 0 and el. Offset
WriteConf(I2Creg);

if(!Stream){
	SingleFrame=1;
}
else{
	SingleFrame=0;
}

do{
	if(giveOut){
		outbuffer=currentBuffer^0x01;
		//el. Buffer has to be filled in the interrupt from time to time, set GetElEveryFrameX to a good value
		#ifndef IgnoreElOffset
			for(k=0;k<PixelEighth;k++){
				if(useStack){
					aveloff=eloffstack[k][0];
					for(j=1;j<StackSize;j++){
						aveloff+=eloffstack[k][j];
					}
					Voltage[outbuffer][ELOFFSET+k]=(unsigned short)(aveloff/(unsigned long)StackSize);
				}
				else{
					aveloff=0;
					for(j=0;j<pelStack;j++){
						aveloff+=eloffstack[k][j];
					}
					Voltage[outbuffer][ELOFFSET+k]=(unsigned short)(aveloff/(unsigned long)j);
				}
			}
	
			for(k=PixelEighth;k<Pixelforth;k++){
				if(useStack){
					aveloff=eloffstack[k][0];
					for(j=1;j<StackSize;j++){
						aveloff+=eloffstack[k][j];
					}
					Voltage[outbuffer][ELOFFSET+k]=(unsigned short)(aveloff/(unsigned long)StackSize);
				}
				else{
					aveloff=0;
					for(j=0;j<pelStack;j++){
						aveloff+=eloffstack[k][j];
					}
					Voltage[outbuffer][ELOFFSET+k]=(unsigned short)(aveloff/(unsigned long)j);
				}
			}
		#endif		
		if(Temps){		//calculate Temperatures
			//first Ta
			PTATLong=0;
				for(k=0;k<(unsigned short)STACKSIZEPTAT;k++){
					PTATLong+=(unsigned long)PTATStack[k];
				}
			TA=(unsigned short)((float)(PTATLong<<(16-Resolution))/(float)STACKSIZEPTAT*PTATGrad+PTATOff+0.5);	
			PTATLong/=(unsigned long)STACKSIZEPTAT;		//is needed later for TWOVDD	
			Voltage[outbuffer][VDDADDRESS+1]=TA;
			PowerGradScale=(signed long)pow(2.0,(float)gradScale);
			//now do the average of Vdd from the sensor if the sensor can measure Vdd by itself
			if(SENSORREV1){
				Vddlong=0;
				for(k=0;k<STACKSIZEVDD;k++){
					Vddlong+=(unsigned long)VddSensorStack[k];
				}
				Vddlong/=(unsigned long)STACKSIZEVDD;
				Voltage[outbuffer][VDDADDRESS]=(unsigned short)Vddlong;	//this overwrites the Vdd of the PCB with the Vdd of the sensor
			}	
			//otherwise measure Vdd by the PCB if an old sensor is used
			//...
			//Here was a bug before which caused a wrong compensation at higher ambient temperatures
			//DeltaVdd=(signed short)((signed long)Vddlong-(signed long)VddRef);
			NewDeltaVdd=(signed long)PTATLong;
			NewDeltaVdd-=(signed long)PTATThermals[0];
			NewDeltaVdd*=((signed long)VddRef[1]-(signed long)VddRef[0]);
			NewDeltaVdd/=((signed long)PTATThermals[1]-(signed long)PTATThermals[0]);
			NewDeltaVdd=(signed long)Vddlong-(signed long)VddRef[0]-NewDeltaVdd;
			DeltaVdd=(signed short)NewDeltaVdd;			
			//finish of bug fix
			DividerVdd=(unsigned long long)(pow(2.0,(float)VddScaling));
			DividerVdd2=(unsigned long long)(pow(2.0,(float)VddScalingOff));			
			for(k=0;k<(unsigned short)Pixel/2;k++){		
				//adjust the ADC resoolution, this can be deleted if 16 Bit ADC resolution are always used!
				Voltage[outbuffer][k]=Voltage[outbuffer][k]<<(16-Resolution);
				//subtract the thermal Offsets
				VoltageLong=(signed long)((signed short)ThGrad[k]);
				VoltageLong*=(signed long)PTATLong;			//this is changed compared to older ones. Use PTAT as the reference!
				VoltageLong/=PowerGradScale;
				VoltageLong+=(signed long)ThOff[k];
				//el. Offset
				VoltageLong+=(signed long)(Voltage[outbuffer][k%ELAMOUNTHALF+ELOFFSET]<<(16-Resolution));
				//VDD-Comp
				VoltageLongLong=(signed long long)VddGrad[k%ELAMOUNTHALF];
				VoltageLongLong*=(signed long long)PTATLong;
				VoltageLongLong/=(signed long long)DividerVdd;
				VoltageLongLong+=(signed long long)VddOff[k%ELAMOUNTHALF];
				VoltageLongLong*=(signed long long)DeltaVdd;
				VoltageLongLong/=(signed long long)DividerVdd2;
				VoltageLong+=(signed long)VoltageLongLong;				
				//all 3 Offsets (Thermal, electrical and Vdd) are calculated, now substract them
				VoltageSigned=(signed short)Voltage[outbuffer][k]-(signed short)VoltageLong;
				//calc. Temperatures			
				#ifdef ReadToFromTable
					Voltage[outbuffer][k]=(unsigned short)calcTO(TA,VoltageSigned,PixC[k],k);
				#endif
			}
			for(i=16;i<32;i++){
				for(k=0;k<32;k++){	
					//adjust the ADC resoolution, this can be deleted if 16 Bit ADC resolution are always used!
					Voltage[outbuffer][k+i*SPALTE]=Voltage[outbuffer][k+i*SPALTE]<<(16-Resolution);
					//subtract the thermal Offsets
					VoltageLong=(signed long)((signed short)ThGrad[k+(47-i)*32]);
					VoltageLong*=(signed long)PTATLong;
					VoltageLong/=PowerGradScale;
					VoltageLong+=(signed long)ThOff[k+(47-i)*32];
					//el.
					VoltageLong+=(signed long)(Voltage[outbuffer][(k+i*COLUMN)%ELAMOUNTHALF+ELAMOUNTHALF+ELOFFSET]<<(16-Resolution));
					//VDD-Comp
					VoltageLongLong=(signed long long)VddGrad[k+(3-i%4)*COLUMN+ELAMOUNTHALF];
					VoltageLongLong*=(signed long long)PTATLong;
					VoltageLongLong/=(signed long long)DividerVdd;
					VoltageLongLong+=(signed long long)VddOff[k+(3-i%4)*COLUMN+ELAMOUNTHALF];
					VoltageLongLong*=(signed long long)DeltaVdd;
					VoltageLongLong/=(signed long long)DividerVdd2;
					VoltageLong+=(signed long)VoltageLongLong;					
				
					VoltageSigned=(signed short)Voltage[outbuffer][k+i*32]-(signed short)VoltageLong;
					//calc. Temperatures
					#ifdef ReadToFromTable
						Voltage[outbuffer][k+i*32]=(unsigned short)calcTO(TA,VoltageSigned,PixC[k+(47-i)*32],1);
					#endif
				}
			}	
		}
       
		if(NrOfDefPix){
			    for(i=0;i<NrOfDefPix;i++){
			        pixsum=0;
			        maskcnt=0;
					if(DeadPixAdr[i]>=Pixel/2){
						k=DeadPixAdr[i]%SPALTE;
						adaptedAdr=Pixel+Pixel/2-DeadPixAdr[i]+k*2-SPALTE;
				        for(k=0;k<8;k++){
				            if((DeadPixMask[i]>>k)&0x1){
				                switch(k){
				                    case 0: //6 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr+SPALTE];
				                        maskcnt++;
				                        break;
				                    case 1: //5 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr+SPALTE+1];
				                        maskcnt++;
				                        break;
				                     case 2: //3 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr+1];
				                        maskcnt++;
				                        break;
				                     case 3: //1 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr-SPALTE+1];
				                        maskcnt++;
				                        break;
				                     case 4: //12 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr-SPALTE];
				                        maskcnt++;
				                        break;
				                     case 5: //11 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr-SPALTE-1];
				                        maskcnt++;
				                        break;
				                     case 6: //9 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr-1];
				                        maskcnt++;
				                        break;
				                     case 7: //7 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr+SPALTE-1];
				                        maskcnt++;
				                        break;
			                    }
			                }
			            }
					}
					else{
						adaptedAdr=DeadPixAdr[i];
				        for(k=0;k<8;k++){
				            if((DeadPixMask[i]>>k)&0x1){
				                switch(k){
				                    case 0: //12 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr-SPALTE];
				                        maskcnt++;
				                        break;
				                    case 1: //1 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr-SPALTE+1];
				                        maskcnt++;
				                        break;
				                     case 2: //3 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr+1];
				                        maskcnt++;
				                        break;
				                     case 3: //5 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr+SPALTE+1];
				                        maskcnt++;
				                        break;
				                     case 4: //6 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr+SPALTE];
				                        maskcnt++;
				                        break;
				                     case 5: //7 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr+SPALTE-1];
				                        maskcnt++;
				                        break;
				                     case 6: //9 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr-1];
				                        maskcnt++;
				                        break;
				                     case 7: //11 o'clock
				                        pixsum+=Voltage[outbuffer][adaptedAdr-SPALTE-1];
				                        maskcnt++;
				                        break;
			                    }
			                }
			            }
					}
		        Voltage[outbuffer][adaptedAdr]=(unsigned short)(pixsum/maskcnt);
		        }
		    }

#ifdef DoAveraging  		//this is doing an averaging over several frames. 
                for(i=0;i<Pixel;i++){
                    if(!runs){
                        ToAverage[i]=(unsigned int)Voltage[outbuffer][i];
                    }
                    else{
                        ToAverage[i]+=(unsigned int)Voltage[outbuffer][i];
                    }                  
                }
				if(!runs){
                    ToAverage[(unsigned short)Pixel]=(unsigned int)Voltage[outbuffer][(unsigned short)PTATSTARTADSRESS];
                    ToAverage[(unsigned short)Pixel+1]=(unsigned int)Voltage[outbuffer][1089];
				}
				else{
                    ToAverage[(unsigned short)Pixel]+=(unsigned int)Voltage[outbuffer][(unsigned short)PTATSTARTADSRESS];
                    ToAverage[(unsigned short)Pixel+1]+=(unsigned int)Voltage[outbuffer][1089];
				} 
				//push out the data
				if(runs==FSteps){ 			//FSteps must be in the power of 2 minus 1, so 1,3,7,15...
                    for(i=0;i<Pixel;i++){
                        Voltage[outbuffer][i]=(unsigned short)(ToAverage[i]>>csv);  //csv=(unsigned char)(log((float)FSteps+1.0)/log(2.0)+0.5);
                    }     
                    Voltage[outbuffer][1089]=(unsigned short)(ToAverage[(unsigned short)Pixel+1]>>csv);
                    Voltage[outbuffer][PTATSTARTADSRESS]=(unsigned short)(ToAverage[(unsigned short)Pixel]>>csv);
#endif			
	    //push the data out via UDP, here is more code required!
		UDPPutArray((BYTE*)Voltage[outbuffer],DATALength+2);  
		UDPPutArray((unsigned char*)&Voltage[outbuffer][DataLengthHalf+1],DATALength-2);
		giveOut=0;
   #ifdef DoAveraging 
				}
                else{
                    runs++;
                }
    #endif		
	}
}while((!INT1occ)&&(SingleFrame<3));

return;
}

/********************************************************************
 * Function:        void InitCLKTRIM(unsigned char user)
 *
 * Description:     
 *
 * Dependencies:    user:	0-> Setting during Calibration
 *							1-> Setting from user
 *******************************************************************/
void InitCLKTRIM(unsigned char user){
	TAsicSFR I2Creg;

	I2Creg.adr=0x1A;	
	I2Creg.read=0x00;
	I2Creg.Config=0x06;		//CLK_TRIM
	if(user){
		I2Creg.sfrdata=SetCLKUser;
		MCLK=(unsigned long)(190476*(unsigned long)SetCLKUser+1000000);		//0=1MHz, 63=13MHz
	}
	else{	
		I2Creg.sfrdata=SetCLKCalib;
		MCLK=(unsigned long)(190476*(unsigned long)SetCLKCalib+1000000);		//0=1MHz, 63=13MHz
	}
	WriteConf(I2Creg);
	tFR=(unsigned int)((pow(2.0,(float)Resolution-4.0)+4.0)*32.0*1000.0/(float)MCLK);					//Time for one frame in ms
	return;
}

/********************************************************************
 * Function:        void InitBIASTRIM(unsigned char user)
 *
 * Description:     
 *
 * Dependencies:    user:	0-> Setting during Calibration
 *							1-> Setting from user
 *******************************************************************/
void InitBIASTRIM(unsigned char user){
	TAsicSFR I2Creg;

	if(user)
		I2Creg.sfrdata=SetBIASUser;
	else	
		I2Creg.sfrdata=SetBIASCalib;
	I2Creg.adr=0x1A;	
	I2Creg.read=0x00;
	I2Creg.Config=0x04;		//BIAS_TRIM_LEFT
	WriteConf(I2Creg);
	
	DelayMs(5);
	
	I2Creg.adr=0x1A;	
	I2Creg.read=0x00;
	I2Creg.Config=0x05;		//BIAS_TRIM_RIGHT
	WriteConf(I2Creg);
	return;
}

/********************************************************************
 * Function:        void InitMBITTRIM(unsigned char user)
 *
 * Description:     
 *
 * Dependencies:    user:	0-> Setting during Calibration
 *							1-> Setting from user
 *******************************************************************/
void InitMBITTRIM(unsigned char user){
	TAsicSFR I2Creg;

	I2Creg.adr=0x1A;	
	I2Creg.read=0x00;
	I2Creg.Config=0x03;		//MBIT_TRIM
	if(user){
		I2Creg.sfrdata=SetMBITUser;
		Resolution=(SetMBITUser&0x0F)+4;
	}
	else{	
		I2Creg.sfrdata=SetMBITCalib;
		Resolution=(SetMBITCalib&0x0F)+4;
	}
	WriteConf(I2Creg);
	return;
}

/********************************************************************
 * Function:        void InitBPATRIM(unsigned char user)
 *
 * Description:     
 *
 * Dependencies:    user:	0-> Setting during Calibration
 *							1-> Setting from user
 *******************************************************************/
void InitBPATRIM(unsigned char user){
	TAsicSFR I2Creg;

	if(user)
		I2Creg.sfrdata=SetBPAUser;
	else	
		I2Creg.sfrdata=SetBPACalib;
	I2Creg.adr=0x1A;	
	I2Creg.read=0x00;
	I2Creg.Config=0x07;		//BIAS_TRIM_LEFT
	WriteConf(I2Creg);
	
	DelayMs(5);
	
	I2Creg.adr=0x1A;	
	I2Creg.read=0x00;
	I2Creg.Config=0x08;		//BIAS_TRIM_RIGHT
	WriteConf(I2Creg);
	return;
}

/********************************************************************
 * Function:        void InitPUTRIM(unsigned char user)
 *
 * Description:     
 *
 * Dependencies:    user:	0-> Setting during Calibration
 *							1-> Setting from user
 *******************************************************************/
void InitPUTRIM(unsigned char user){
	TAsicSFR I2Creg;

	I2Creg.adr=0x1A;	
	I2Creg.read=0x00;
	I2Creg.Config=0x09;		//PU_TRIM SDA & SCL
	if(user)
		I2Creg.sfrdata=SetPUUser;
	else	
		I2Creg.sfrdata=SetPUCalib;
	WriteConf(I2Creg);
	return;
}


unsigned int WriteConf(TAsicSFR reg){
	unsigned short ErrorCode;
	IdleI2C();						//Ensure Module is Idle
	StartI2C();						//Generate Start Condition
	WriteI2C((reg.adr<<1)|reg.read);	//Write device addr R/NW=0
	IdleI2C();
	ErrorCode = ACKStatus();		//Return ACK Status

	WriteI2C(reg.Config);			//Write Config addr
	IdleI2C();
	ErrorCode = ACKStatus();		//Return ACK Status

	WriteI2C(reg.sfrdata);
	IdleI2C();

	ErrorCode = ACKStatus();		//Return ACK Status
	return(ErrorCode);
}

/*********************************************************************
* Function:        StartI2C()
*
* Input:		None.
*
* Output:		None.
*
* Overview:		Generates an I2C Start Condition
*
* Note:			None
********************************************************************/
unsigned int StartI2C(void)
{
	//This function generates an I2C start condition and returns status 
	//of the Start.
#ifdef BITBANGING
    bstartSensor();
    return 1;
#else
	I2C1CONbits.SEN = 1;		//Generate Start COndition
	while (I2C1CONbits.SEN);	//Wait for Start COndition
	return(I2C1STATbits.S);	//Optionally return status
#endif
}


/*********************************************************************
* Function:        RestartI2C()
*
* Input:		None.
*
* Output:		None.
*
* Overview:		Generates a restart condition and optionally returns status
*
* Note:			None
********************************************************************/
unsigned int RestartI2C(void)
{
	//This function generates an I2C Restart condition and returns status 
	//of the Restart.
#ifdef BITBANGING
    bstartSensor();
    return 1;
#else
	I2C1CONbits.RSEN = 1;		//Generate Restart		
	while (I2C1CONbits.RSEN);	//Wait for restart	
	return(I2C1STATbits.S);	//Optional - return status
#endif
}


/*********************************************************************
* Function:        StopI2C()
*
* Input:		None.
*
* Output:		None.
*
* Overview:		Generates a bus stop condition
*
* Note:			None
********************************************************************/
unsigned int StopI2C(void)
{
	//This function generates an I2C stop condition and returns status 
	//of the Stop.
#ifdef BITBANGING
    bstopSensor();
    return 1;
#else
	I2C1CONbits.PEN = 1;		//Generate Stop Condition
	while (I2C1CONbits.PEN);	//Wait for Stop
	return(I2C1STATbits.P);		//Optional - return status
#endif
}


/*********************************************************************
* Function:        WriteI2C()
*
* Input:		Byte to write.
*
* Output:		None.
*
* Overview:		Writes a byte out to the bus
*
* Note:			None
********************************************************************/
unsigned int WriteI2C(unsigned char byte)
{
	//This function transmits the byte passed to the function
	//while (I2C1STATbits.TRSTAT);	//Wait for bus to be idle
#ifdef BITBANGING
    byte_outSensor(byte);    
#else
	I2C1TRN = byte;					//Load byte to I2C1 Transmit buffer
	while (I2C1STATbits.TBF);		//wait for data transmission
#endif
	return 0;
}


/*********************************************************************
* Function:        IdleI2C()
*
* Input:		None.
*
* Output:		None.
*
* Overview:		Waits for bus to become Idle
*
* Note:			None
********************************************************************/
unsigned int IdleI2C(void)
{
#ifndef BITBANGING
	while (I2C1STATbits.TRSTAT);		//Wait for bus Idle
#endif
	return 1;
}

/*********************************************************************
* Function:        LDByteWriteI2C()
*
* Input:		Control Byte, 8 - bit address, data.
*
* Output:		None.
*
* Overview:		Write a byte to low density device at address LowAdd
*
* Note:			None
********************************************************************/
unsigned int LDByteWriteI2C(unsigned char ControlByte, unsigned char LowAdd, unsigned char data)
{
	unsigned short ErrorCode;

	IdleI2C();						//Ensure Module is Idle
	StartI2C();						//Generate Start COndition
	WriteI2C(ControlByte);			//Write Control byte
	IdleI2C();
    
	ErrorCode = ACKStatus();		//Return ACK Status
	WriteI2C(LowAdd);				//Write Low Address
	IdleI2C();

	ErrorCode = ACKStatus();		//Return ACK Status
	
	WriteI2C(data);					//Write Data
	IdleI2C();
	ErrorCode = ACKStatus();		//Return ACK Status
	StopI2C();						//Initiate Stop Condition
//	EEAckPolling(ControlByte);		//Perform ACK polling
	return(ErrorCode);
}


/*********************************************************************
* Function:        LDByteReadI2C()
*
* Input:		Control Byte, Address, *Data, Length.
*
* Output:		None.
*
* Overview:		Performs a low density read of Length bytes and stores in *Data array
*				starting at Address.
*
* Note:			None
********************************************************************/
unsigned int LDByteReadI2C(unsigned char ControlByte, unsigned char Address, unsigned char *Data, unsigned char Length)
{								//note: wird nicht benötigt, da Read_EEPROM dasselbe macht
	IdleI2C();					//wait for bus Idle
	StartI2C();					//Generate Start Condition
	WriteI2C(ControlByte);		//Write Control Byte
	IdleI2C();					//wait for bus Idle
	WriteI2C(Address);			//Write start address
	IdleI2C();					//wait for bus Idle

	RestartI2C();				//Generate restart condition
	WriteI2C(ControlByte | 0x01);	//Write control byte for read
	IdleI2C();					//wait for bus Idle

	getsI2C(Data, Length);		//read Length number of bytes
	NotAckI2C();				//Send Not Ack
	StopI2C();					//Generate Stop
	return 1;
}


/*********************************************************************
* Function:        HDByteWriteI2C()
*
* Input:		ControlByte, HighAddr, LowAddr, Data.
*
* Output:		None.
*
* Overview:		perform a high density byte write of data byte, Data.
*				starting at the address formed from HighAdd and LowAdd
*
* Note:			None
********************************************************************/
unsigned int HDByteWriteI2C(unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char data)
{
	unsigned short ErrorCode;

	IdleI2C();						//Ensure Module is Idle
	StartI2C();						//Generate Start COndition
	WriteI2C(ControlByte);			//Write Control byte
	IdleI2C();

	ErrorCode = ACKStatus();		//Return ACK Status
	
	WriteI2C(HighAdd);
	IdleI2C();						//Write High Address
	WriteI2C(LowAdd);				//Write Low Address
	IdleI2C();

	ErrorCode = ACKStatus();		//Return ACK Status

	WriteI2C(data);					//Write Data
	IdleI2C();
	StopI2C();						//Initiate Stop Condition
	EEAckPolling(ControlByte);		//perform Ack Polling
	return(ErrorCode);
}


/*********************************************************************
* Function:        HDByteReadI2C()
*
* Input:		Control Byte, HighAdd, LowAdd, *Data, Length.
*
* Output:		None.
*
* Overview:		Performs a low density read of Length bytes and stores in *Data array
*				starting at Address formed from HighAdd and LowAdd.
*
* Note:			None
********************************************************************/
unsigned int HDByteReadI2C(unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char *Data, unsigned char Length)
{
	IdleI2C();						//Wait for bus Idle
	StartI2C();						//Generate Start condition
	WriteI2C(ControlByte);			//send control byte for write
	IdleI2C();						//Wait for bus Idle

	WriteI2C(HighAdd);				//Send High Address
	IdleI2C();						//Wait for bus Idle
	WriteI2C(LowAdd);				//Send Low Address
	IdleI2C();						//Wait for bus Idle

	RestartI2C();					//Generate Restart
	WriteI2C(ControlByte | 0x01);	//send control byte for Read
	IdleI2C();						//Wait for bus Idle

	getsI2C(Data, Length);			//Read Length number of bytes to Data
	NotAckI2C();					//send Not Ack
	StopI2C();						//Send Stop Condition
	return(0);
}


/*********************************************************************
* Function:        LDPageWriteI2C()
*
* Input:		ControlByte, LowAdd, *wrptr.
*
* Output:		None.
*
* Overview:		Write a page of data from array pointed to be wrptr
*				starting at LowAdd
*
* Note:			LowAdd must start on a page boundary
********************************************************************/
unsigned int LDPageWriteI2C(unsigned char ControlByte, unsigned char LowAdd, unsigned char *wrptr)
{
	IdleI2C();					//wait for bus Idle
	StartI2C();					//Generate Start condition
	WriteI2C(ControlByte);		//send controlbyte for a write
	IdleI2C();					//wait for bus Idle
	WriteI2C(LowAdd);			//send low address
	IdleI2C();					//wait for bus Idle
	putstringI2C(wrptr);		//send data
	IdleI2C();					//wait for bus Idle
	StopI2C();					//Generate Stop
	return(0);
}


/*********************************************************************
* Function:        HDPageWriteI2C()
*
* Input:		ControlByte, HighAdd, LowAdd, *wrptr.
*
* Output:		None.
*
* Overview:		Write a page of data from array pointed to be wrptr
*				starting at address from HighAdd and LowAdd
*
* Note:			Address must start on a page boundary
********************************************************************/
unsigned int HDPageWriteI2C(unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char *wrptr)
{
	IdleI2C();				//wait for bus Idle
	StartI2C();				//Generate Start condition
	WriteI2C(ControlByte);	//send controlbyte for a write
	IdleI2C();				//wait for bus Idle
	WriteI2C(HighAdd);		//send High Address
	IdleI2C();				//wait for bus Idle
	WriteI2C(LowAdd);		//send Low Address
	IdleI2C();				//wait for bus Idle
	putstringI2C(wrptr);	//Send data
	IdleI2C();				//wait for bus Idle
	StopI2C();				//Generate a stop
	return(0);
}


/*********************************************************************
* Function:        LDSequentialReadI2C()
*
* Input:		ControlByte, address, *rdptr, length.
*
* Output:		None.
*
* Overview:		Performs a sequential read of length bytes starting at address
*				and places data in array pointed to by *rdptr
*
* Note:			None
********************************************************************/
unsigned int LDSequentialReadI2C(unsigned char ControlByte, unsigned char address, unsigned char *rdptr, unsigned char length)
{
	IdleI2C();						//Ensure Module is Idle
	StartI2C();						//Initiate start condition
	WriteI2C(ControlByte);			//write 1 byte
	IdleI2C();						//Ensure module is Idle
	WriteI2C(address);				//Write word address
	IdleI2C();						//Ensure module is idle
	RestartI2C();					//Generate I2C Restart Condition
	WriteI2C(ControlByte | 0x01);	//Write 1 byte - R/W bit should be 1 for read
	IdleI2C();						//Ensure bus is idle
	getsI2C(rdptr, length);			//Read in multiple bytes
	NotAckI2C();					//Send Not Ack
	StopI2C();						//Send stop condition
	return(0);
}


/*********************************************************************
* Function:        HDSequentialReadI2C()
*
* Input:		ControlByte, HighAdd, LowAdd, *rdptr, length.
*
* Output:		None.
*
* Overview:		Performs a sequential read of length bytes starting at address
*				and places data in array pointed to by *rdptr
*
* Note:			None
********************************************************************/
unsigned int HDSequentialReadI2C(unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char *rdptr, unsigned char length)
{
	IdleI2C();						//Ensure Module is Idle
	StartI2C();						//Initiate start condition
	WriteI2C(ControlByte);			//write 1 byte
	IdleI2C();						//Ensure module is Idle
	WriteI2C(HighAdd);				//Write High word address
	IdleI2C();						//Ensure module is idle
	WriteI2C(LowAdd);				//Write Low word address
	IdleI2C();						//Ensure module is idle
	RestartI2C();					//Generate I2C Restart Condition
	WriteI2C(ControlByte | 0x01);	//Write 1 byte - R/W bit should be 1 for read
	IdleI2C();						//Ensure bus is idle
	getsI2C(rdptr, length);			//Read in multiple bytes
	NotAckI2C();					//Send Not Ack
	StopI2C();						//Send stop condition
	return(0);
}


/*********************************************************************
* Function:        ACKStatus()
*
* Input:		None.
*
* Output:		Acknowledge Status.
*
* Overview:		Return the Acknowledge status on the bus
*
* Note:			None
********************************************************************/
unsigned int ACKStatus(void)
{
#ifdef BITBANGING
    //unsigned char ack;
    //bit_in(&ack);
    //return (!ack);
    return 0;
#else
	return (!I2C1STATbits.ACKSTAT);		//Return Ack Status
#endif
}


/*********************************************************************
* Function:        NotAckI2C()
*
* Input:		None.
*
* Output:		None.
*
* Overview:		Generates a NO Acknowledge on the Bus
*
* Note:			None
********************************************************************/
unsigned int NotAckI2C(void)
{
#ifdef BITBANGING
    bit_outSensor(0x1);
#else
	I2C1CONbits.ACKDT = 1;			//Set for NotACk
	I2C1CONbits.ACKEN = 1;
	while(I2C1CONbits.ACKEN);		//wait for ACK to complete
	I2C1CONbits.ACKDT = 0;			//Set for NotACk
#endif
}


/*********************************************************************
* Function:        AckI2C()
*
* Input:		None.
*
* Output:		None.
*
* Overview:		Generates an Acknowledge.
*
* Note:			None
********************************************************************/
unsigned int AckI2C(void)
{
	I2C1CONbits.ACKDT = 0;			//Set for ACk
	I2C1CONbits.ACKEN = 1;
	while(I2C1CONbits.ACKEN);		//wait for ACK to complete
	return 1;
}


/*********************************************************************
* Function:       getsI2C()
*
* Input:		array pointer, Length.
*
* Output:		None.
*
* Overview:		read Length number of Bytes into array
*
* Note:			None
********************************************************************/
unsigned int getsI2C(unsigned char *rdptr, unsigned short Length)
{
#ifdef BITBANGING
	while (Length --)
	{
        if(Length){
            *rdptr++ = byte_inSensor(0);		//get a single byte with ACK
        }
        else{            
            *rdptr++ = byte_inSensor(0x80);		//get a single byte with NACK
        }
	}    
#else
	while (Length --)
	{
		*rdptr++ = getI2C();		//get a single byte
		
		if(I2C1STATbits.BCL)		//Test for Bus collision
		{
			return(-1);
		}

		if(Length)
		{
			AckI2C();				//Acknowledge until all read
		}
        else{
            NotAckI2C();            //send a NACK
        }
	}
#endif
	return(0);
}


/*********************************************************************
* Function:        getI2C()
*
* Input:		None.
*
* Output:		contents of I2C1 receive buffer.
*
* Overview:		Read a single byte from Bus
*
* Note:			None
********************************************************************/
unsigned int getI2C(void)
{
	I2C1CONbits.RCEN = 1;			//Enable Master receive
	Nop();
	while(!I2C1STATbits.RBF);		//Wait for receive bufer to be full
	return(I2C1RCV);				//Return data in buffer
}


/*********************************************************************
* Function:        EEAckPolling()
*
* Input:		Control byte.
*
* Output:		error state.
*
* Overview:		polls the bus for an Acknowledge from device
*
* Note:			None
********************************************************************/
unsigned int EEAckPolling(unsigned char control)
{
	IdleI2C();				//wait for bus Idle
	StartI2C();				//Generate Start condition
	
#ifndef BITBANGING    
	if(I2C1STATbits.BCL)
	{
		return(-1);			//Bus collision, return
	}

	else
	{
#endif
		if(WriteI2C(control))
		{
			return(-3);		//error return
		}

		IdleI2C();			//wait for bus idle
#ifndef BITBANGING          
		if(I2C1STATbits.BCL)
		{
			return(-1);		//error return
		}
#endif 
		while(ACKStatus())
		{
			RestartI2C();	//generate restart     
#ifndef BITBANGING             
			if(I2C1STATbits.BCL)
			{
				return(-1);	//error return
			}
#endif
			if(WriteI2C(control))
			{
				return(-3);
			}

			IdleI2C();
		}       
#ifndef BITBANGING         
	}
#endif
	StopI2C();				//send stop condition
#ifndef BITBANGING     
	if(I2C1STATbits.BCL)
	{
		return(-1);
	}
#endif
	return(0);
}

#ifdef BITBANGING
/********************************************************************
 * Function:        void bstart(void)
 *
 * Description:     This function generates an I2C Start condition.
 *******************************************************************/
void bstartSensor(void)
{

    SDA_TRISSensor = 1;                   // Ensure SDA is high
    SCLSensor = 1;                        // Ensure SCL is high
	asm("NOP");
    SDA_TRISSensor = 0;                   // Configure SDA as an output
    SDASensor = 0;                        // Pull SDA low
	kHz400DelaySensor();
    SCLSensor = 0;                        // Pull SCL low
  
} // end bstart(void)

/********************************************************************
 * Function:        void bstop(void)
 *
 * Description:     This function generates an I2C Stop condition.
 *******************************************************************/
void bstopSensor(void)
{
    SCLSensor = 0;                        // Ensure SCL is low
    SDA_TRISSensor = 0;                   // Configure SDA as an output
    SDASensor = 0;                        // Ensure SDA low
	kHz400DelaySensor();
    SCLSensor = 1;                        // Pull SCL high
	kHz400DelaySensor();
    SDA_TRISSensor = 1;                   // Allow SDA to be pulled high
} // end bstop(void)

/********************************************************************
 * Function:        void bit_out(unsigned char data)
 *
 * Description:     This function outputs a bit to the I2C bus.
 *******************************************************************/
static inline __attribute__((__always_inline__)) void bit_outSensor(unsigned char data)
{
    if (data & 0x80)                // Check if next bit is high
    {
        SDA_TRISSensor = 1;               // Release SDA to be pulled high
    }
    else
    {
        SDA_TRISSensor = 0;               // Configure SDA as an output
        SDASensor = 0;                    // Pull SDA low
    }
	kHz400DelaySensor();    
    SCLSensor = 1;                        // Pull SCL high to clock bit
	kHz400DelaySensor();
    SCLSensor = 0;                        // Pull SCL low for next bit
} // end bit_out(unsigned char data)

/********************************************************************
 * Function:        void bit_in(unsigned char *data)
 *
 * Description:     This function inputs a bit from the I2C bus.
 *******************************************************************/
static inline __attribute__((__always_inline__)) void bit_inSensor(unsigned char *data)
{

    SDA_TRISSensor = 1;                   // Configure SDA as an input
    SCLSensor = 1;                        // Bring SCL high to begin transfer
	kHz400DelaySensor();   
    *data &= 0xFE;                  // Assume next bit is low
    if (SDASensor)                        // Check if SDA is high
    {
        *data |= 0x01;              // If high, set next bit
    }
	kHz400DelaySensor();
    SCLSensor = 0;                        // Bring SCL low again

} // end bit_in(unsigned char *data)

/********************************************************************
 * Function:        unsigned char byte_out(unsigned char data)
 *
 * Description:     This function outputs a byte to the I2C bus.
 *                  It also receives the ACK bit and returns 0 if
 *                  successfully received, or 1 if not.
 *******************************************************************/
inline __attribute__((__always_inline__)) unsigned char byte_outSensor(unsigned char data)
{
    unsigned char i;                // Loop counter
    unsigned char ack;              // ACK bit

    ack = 0;
    for (i = 0; i < 8; i++)         // Loop through each bit
    {
        bit_outSensor(data);              // Output bit
        data = data << 1;           // Shift left for next bit
    }
    SDA_TRISSensor = 0;               // Configure SDA as an output
    SDASensor = 0;                    // Pull SDA low
	kHz400DelaySensor(); 
    bit_inSensor(&ack);                   // Input ACK bit

    return ack;
} // end byte_out(unsigned char data)

/********************************************************************
 * Function:        unsigned char byte_in(unsigned char ack)
 *
 * Description:     This function inputs a byte from the I2C bus.
 *                  Depending on the value of ack, it will also
 *                  transmit either an ACK or a NAK bit.
 *******************************************************************/
inline __attribute__((__always_inline__)) unsigned char byte_inSensor(unsigned char ack)
{
    unsigned char i;                // Loop counter
    unsigned char retval;         	// Return value

    retval = 0;
    for (i = 0; i < 8; i++)         // Loop through each bit
    {
        retval = retval << 1;       // Shift left for next bit
        bit_inSensor(&retval);            // Input bit  
    }        
    bit_outSensor(ack);                   // Output ACK/NAK bit
    return retval;
} // end byte_in(void)

/////////////////////////////////////////////////////
static inline __attribute__((__always_inline__)) void kHz400DelaySensor(void){  //delay for 400kHz-CLK of EEPROM    
    if(khz400DelayTime){
        unsigned int i;
        for(i=0;i<khz400DelayTime;i++){
            asm("NOP");
        }
    }
    return;
}

#endif


/*********************************************************************
* Function:        putstringI2C()
*
* Input:		pointer to array.
*
* Output:		None.
*
* Overview:		writes a string of data upto PAGESIZE from array
*
* Note:			None
********************************************************************/
unsigned int putstringI2C(unsigned char *wrptr)
{
	unsigned char x;

	for(x = 0; x < PAGESIZE; x++)		//Transmit Data Until Pagesize
	{	
		if(WriteI2C(*wrptr))			//Write 1 byte
		{
			return(-3);				//Return with Write Collision
		}
		IdleI2C();					//Wait for Idle bus
		if(I2C1STATbits.ACKSTAT)
		{
			return(-2);				//Bus responded with Not ACK
		}
		wrptr++;
	}
	return(0);
}

/*********************************************************************
* Function:     Read_RAM()
*
* Input:		pointer to data, ReadAdress of RAM and amount of data to read
*
* Output:		data (changes global)
*
* Overview:		reads the RAM at the Adress ReadAdress for the amount of readnumber
*
* Note:			None
********************************************************************/

unsigned int Read_RAM(unsigned char* data,TAsicSFR reg, unsigned short readnumber){
    unsigned short ErrorCode;
	memset((unsigned short*)data,0,readnumber);

	IdleI2C();						//Ensure Module is Idle
	StartI2C();						//Generate Start Condition
	WriteI2C((reg.adr<<1)|reg.read);	//Write device addr
	IdleI2C();    
	ErrorCode = ACKStatus();		//Return ACK Status

	WriteI2C(reg.Config);			//Write Config addr
	IdleI2C();
	ErrorCode = ACKStatus();		//Return ACK Status
//	if(ErrorCode)					//maybe the slave is just in data conversion and so it does not send the ACK
//		return;

	IdleI2C();
	RestartI2C();					//Generate I2C Restart Condition

	WriteI2C((reg.adr<<1)|0x01);	//Write device addr again with R/W=1

	IdleI2C();
	getsI2C(data, readnumber);		//read Length number of bytes		
	StopI2C();						//Initiate Stop Condition
	return(ErrorCode);
}	
//instead of "getsI2C(data, readnumber);" if the readout needs to be done separatley due to memmory issues, the line could be replaced by e.g.:
//if(readnumber>1){
//	getsI2C(data, readnumber/2);		//read Length/2 number of bytes
//	AckI2C();
//	data+=readnumber/2;
//	getsI2C(data, readnumber/2);		//read Length/2 number of bytes
//}
//else{
//	getsI2C(data, readnumber);
//}
//The readout can simply be stopped by not sending any clock transitions anymore and continued by sending further clock transitions

/********************************************************************
 * Function:        void calcTO(unsigned int TAmb, signed int dig, unsigned long PiC)
 *
 * Description:     calculate the object temperature via look-up table
 *
 * Dependencies:    ambient temperature (TAmb), pixel voltage (dig), pixel sensitivity coefficients (PiC)
 *******************************************************************/
unsigned int calcTO(unsigned int TAmb, signed int dig, signed long PiC, unsigned int dontCalcTA){
	unsigned int Tobject,y;
	signed int val;
	signed long vx,vy,ydist;
	static unsigned int CurTACol;
	static signed int dTA;
	signed long long scale;


	//first check the position in x-axis of table
	if(!dontCalcTA){
		if(TAmb<XTATemps[0]){
			CurTACol=0;
		}
		else{
			for(CurTACol=0;CurTACol<NROFTAELEMENTS;CurTACol++){
				if((XTATemps[CurTACol]<=TAmb)&&(XTATemps[CurTACol+1]>TAmb))
						break;
			}
		}
		dTA=TAmb-XTATemps[CurTACol];
	}

	if((CurTACol>=NROFTAELEMENTS-1))
			return 0;


	//now scale dig to PCSCALEVAL
	scale=(signed long long)PCSCALEVAL*(signed long long)dig;
	vx=(signed long)(scale/((signed long long)PiC));


	val=vx+TABLEOFFSET;
	//now determine row
	y=val>>ADEXPBITS;
	ydist=(signed long)ADEQUIDISTANCE;
	if(y<(NROFADELEMENTS-1)){
		if(TempTable[y][CurTACol]){
			vx=((((signed long)TempTable[y][CurTACol+1]-(signed long)TempTable[y][CurTACol])*(signed long)dTA)/(signed long)TAEQUIDISTANCE)+(signed long)TempTable[y][CurTACol];
			vy=((((signed long)TempTable[y+1][CurTACol+1]-(signed long)TempTable[y+1][CurTACol])*(signed long)dTA)/(signed long)TAEQUIDISTANCE)+(signed long)TempTable[y+1][CurTACol];
			Tobject=(unsigned int)((vy-vx)*((signed long)val-(signed long)YADValues[y])/ydist+(signed long)vx);
			}
		else
			return 0;
		}
	else
		return 0;
		
#ifdef AdjustOffsetGain
	val=(signed int)Tobject;
	val+=(signed int)GlobalOffset;
	Tobject=(unsigned short)val;
#endif		

	return (unsigned short)Tobject;
}

/********************************************************************
 * Function:        void ReadCalibData(void)
 *
 * Description:     read all required calibration data
 *
 * Dependencies:    
 *******************************************************************/
void ReadCalibData(unsigned char print){
TAsicSFR I2Creg;
unsigned short i,PixCRaw;
float common[2],PixCMin,PixCMax;
char eecpy[8];

//read the whole EEPROM and put them into the storage

	HighDensSequentialRead((unsigned short)AdrPixCMin,eecpy,sizeof(float)*2);
	memcpy((char*)&common,eecpy,sizeof(float)*2);
	PixCMin=common[0];
	PixCMax=common[1];
	HighDensSequentialRead((unsigned short)AdrGradScale,eecpy,sizeof(char));
	gradScale=(unsigned short)eecpy[0];
	HighDensSequentialRead((unsigned short)AdrExponent,eecpy,sizeof(char)*3);
	TableNumberSensor=(unsigned short)eecpy[1]<<8;
	TableNumberSensor+=(unsigned short)eecpy[0];
	epsilon=(unsigned short)eecpy[2];
	HighDensSequentialRead((unsigned short)AdrMBITPixC,eecpy,sizeof(char)*5);
	SetMBITCalib=(unsigned short)eecpy[0];
	SetBIASCalib=(unsigned short)eecpy[1];
	SetCLKCalib=(unsigned short)eecpy[2];
	SetBPACalib=(unsigned short)eecpy[3];
	SetPUCalib=(unsigned short)eecpy[4];

	HighDensSequentialRead((unsigned short)AdrPTATGrad,eecpy,sizeof(float)*2);
	memcpy((char*)&common,eecpy,sizeof(float)*2);
	PTATGrad=common[0];
	PTATOff=common[1];

	HighDensSequentialRead((unsigned short)AdrDevID,eecpy,sizeof(char)*4);
	memcpy((char*)&DevID,eecpy,sizeof(long));

	//read all settings for clock etc. from the EEPROM, if not set, use the default values!
	HighDensSequentialRead((unsigned short)AdrMBITUser,eecpy,sizeof(char));
	memcpy((unsigned char*)&SetMBITUser,eecpy,sizeof(unsigned char));
	if((SetMBITUser<0x09)||(SetMBITUser>0x0C)){		//EEPROM empty or setting out of spec -> set to default
		SetMBITUser=(unsigned char)MBITTRIMDefault;
		eecpy[0]=SetMBITUser;
		HighDensPageWrite((unsigned short)AdrMBITUser,eecpy,sizeof(char));
	}
	HighDensSequentialRead((unsigned short)AdrBIASUser,eecpy,sizeof(char));
	memcpy((unsigned char*)&SetBIASUser,eecpy,sizeof(unsigned char));
	if((!SetBIASUser)||(SetBIASUser>0x1F)){		//EEPROM empty or setting out of spec -> set to default
		SetBIASUser=(unsigned char)BIAScurrentDefault;
		eecpy[0]=SetBIASUser;
		HighDensPageWrite((unsigned short)AdrBIASUser,eecpy,sizeof(char));
	}
	HighDensSequentialRead((unsigned short)AdrCLKUser,eecpy,sizeof(char));
	memcpy((unsigned char*)&SetCLKUser,eecpy,sizeof(unsigned char));
	if(SetCLKUser>0x3F){		//EEPROM empty or setting out of spec -> set to default
		SetCLKUser=(unsigned char)CLKTRIMDefault;
		eecpy[0]=SetCLKUser;
		HighDensPageWrite((unsigned short)AdrCLKUser,eecpy,sizeof(char));
	}
	HighDensSequentialRead((unsigned short)AdrBPAUser,eecpy,sizeof(char));
	memcpy((unsigned char*)&SetBPAUser,eecpy,sizeof(unsigned char));
	if((!SetBPAUser)||(SetBPAUser>0x1F)){		//EEPROM empty or setting out of spec -> set to default
		SetBPAUser=(unsigned char)BPATRIMDefault;
		eecpy[0]=SetBPAUser;
		HighDensPageWrite((unsigned short)AdrBPAUser,eecpy,sizeof(char));
	}
	HighDensSequentialRead((unsigned short)AdrPUUser,eecpy,sizeof(char));
	memcpy((unsigned char*)&SetPUUser,eecpy,sizeof(unsigned char));
	if(!((SetPUUser==0x11)||(SetPUUser==0x22)||(SetPUUser==0x44)||(SetPUUser==0x88))){		//EEPROM empty or setting out of spec -> set to default
		SetPUUser=(unsigned char)PUTRIMDefault;
		eecpy[0]=SetPUUser;
		HighDensPageWrite((unsigned short)AdrPUUser,eecpy,sizeof(char));
	}

#ifdef AdjustOffsetGain
	HighDensSequentialRead((unsigned short)AdrGlobalOffset,eecpy,sizeof(char));
	memcpy((char*)&GlobalOffset,eecpy,sizeof(char));
	if(GlobalOffset==(char)0xFF){
		GlobalOffset=0;
		eecpy[0]=GlobalOffset;
		HighDensPageWrite((unsigned short)AdrGlobalOffset,eecpy,sizeof(char));
	}
	HighDensSequentialRead((unsigned short)AdrGlobalGain,eecpy,sizeof(char)*2);
	memcpy((char*)&GlobalGain,eecpy,sizeof(short));
	//if values are not set before, do it now!
	if((GlobalGain==0x0000)||(GlobalGain==0xFFFF)){
		GlobalGain=10000;
		memcpy(eecpy,(char*)&GlobalGain,sizeof(short));
		HighDensPageWrite((unsigned short)AdrGlobalGain,eecpy,sizeof(short));
	}
#endif

	//now load the Thermal-Grad and -Offset
	for(i=0;i<(unsigned short)Pixel;i++){
		HighDensSequentialRead((unsigned short)AdrTh1+i*2,eecpy,sizeof(char)*2);
		memcpy((char*)&ThGrad[i],eecpy,sizeof(short));
		HighDensSequentialRead((unsigned short)AdrTh2+i*2,eecpy,sizeof(char)*2);
		memcpy((char*)&ThOff[i],eecpy,sizeof(short));
	}
	
	//and finally the PixC
	for(i=0;i<(unsigned short)Pixel;i++){
		HighDensSequentialRead((unsigned short)AdrPixC+i*2,eecpy,sizeof(char)*2);
		memcpy((char*)&PixCRaw,eecpy,sizeof(short));
		#ifdef AdjustOffsetGain
			PixC[i]=(unsigned long)((((float)PixCRaw/65535.0)*(PixCMax-PixCMin)+(float)PixCMin)*(float)epsilon/100.0*(float)GlobalGain/10000.0+0.5);
		#else
			PixC[i]=(unsigned long)((((float)PixCRaw/65535.0)*(PixCMax-PixCMin)+(float)PixCMin)*(float)epsilon/100.0+0.5);
		#endif
	}

	for(i=0;i<(unsigned short)ELAMOUNT;i++){
		HighDensSequentialRead((unsigned short)AdrVddCompValues2+i*2,eecpy,sizeof(char)*2);
		memcpy((char*)&VddGrad[i],eecpy,sizeof(short));
	}
	HighDensSequentialRead((unsigned short)AdrVddScaling,eecpy,sizeof(char));
	VddScaling=(unsigned short)eecpy[0];	
	HighDensSequentialRead((unsigned short)AdrVddScalingOff,eecpy,sizeof(char));
	VddScalingOff=(unsigned short)eecpy[0];
	for(i=0;i<(unsigned short)ELAMOUNT;i++){
		HighDensSequentialRead((unsigned short)AdrVddCompValues+i*2,eecpy,sizeof(char)*2);
		memcpy((char*)&VddOff[i],eecpy,sizeof(short));
	}
	if(SENSORREV1){
		//readout more data from the EEPROM as more is reuired for the bug fix
		HighDensSequentialRead((unsigned short)AdrVddMeasTh1,eecpy,sizeof(char)*4);
		memcpy((char*)&VddRef,eecpy,sizeof(short)*2);
		HighDensSequentialRead((unsigned short)AdrPTATTh1,eecpy,sizeof(char)*4);
		memcpy((char*)&PTATThermals,eecpy,sizeof(short)*2);	
	}
	else{
		HighDensSequentialRead((unsigned short)AdrVddTh1,eecpy,sizeof(char)*2);
		memcpy((char*)&VddRef,eecpy,sizeof(short));
	}
	
return;
}


void HighDensPageWrite(unsigned short address,unsigned char *data, unsigned short numbytes){
	unsigned char HighAdd,LowAdd;
	InitI2C(1);							//init I²C for EEPROM (max 400 kHz)
	for(i=0;i<numbytes;i++){
		LowAdd=(unsigned char)(address&0xFF);
		HighAdd=(unsigned char)((address&0xFF00)>>8);
		HDByteWriteI2C(0xA0, HighAdd, LowAdd, data[i]);
		address++;
		DelayMs(5);		//give the sensor some time to write to the EEPROM
	}
	InitI2C(0);	
return;
}

/********************************************************************
 * Function:        void HighDensByteRead(unsigned char *data)
 *
 * Description:     This function reads a single byte from a
 *                  high-density (>= 32 Kb) serial EEPROM device.
 *
 * Dependencies:    'i2ccontrol' contains the control byte
 *                  'address' contains the address word
 *******************************************************************/
void HighDensByteRead(unsigned short address, unsigned char *data)
{
    bstart();                       // Generate Start condition
    byte_out(CONTROLBYTE);              // Output control byte
    byte_out((unsigned char)(address>>8));// Output address MSB
    byte_out((unsigned char)address);// Output address LSB
    bstart();                       // Generate Start condition
    byte_out(CONTROLBYTE | 0x01);       // Output control byte
    *data = byte_in(NAKBIT);        // Input data byte
    bstop();                        // Generate Stop condition
} // end HighDensByteRead(unsigned char data)

/********************************************************************
 * Function:        void HighDensSequentialRead(unsigned char *data,
 *                                              unsigned char numbytes)
 *
 * Description:     This function reads multiple bytes from a
 *                  high-density (>= 32 Kb) serial EEPROM device.
 *
 * Dependencies:    'i2ccontrol' contains the control byte
 *                  'address' contains the address word
 *******************************************************************/
void HighDensSequentialRead(unsigned short address,unsigned char* data, unsigned short numbytes)
{
	unsigned char HighAdd,LowAdd;
	InitI2C(1);					//init I²C for EEPROM (max 400 kHz)
	LowAdd=(unsigned char)(address&0xFF);
	HighAdd=(unsigned char)((address&0xFF00)>>8);
	HDSequentialReadI2C(0xA0,HighAdd,LowAdd,data,numbytes);
	InitI2C(0);					//init I²C for Sensor (> 1000 kHz)
}
/*********************************************************************
* Function:        InitI2C(unsigned char EEPROM)
*
* Input:		None.
*
* Output:		None.
*
* Overview:		Initialises the I2C(1) peripheral
*
* Note:			Sets up Master mode, No slew rate control, 100Khz
				This function is depending on the hardware (controller) you are using.
				It should just help to see how an I2C interface is initialized by using a PIC32 controller
********************************************************************/
unsigned int InitI2C(unsigned char EEPROM){	
#ifdef BITBANGING
    if(EEPROM)
        khz400DelayTime=15;
    else
        khz400DelayTime=BITBANGING;
	I2C1CON = 0x1200;   //switch it off
#else
	unsigned short BRG_VAL;
	//const unsigned long FSCL=200000;
if(EEPROM){
	BRG_VAL=(unsigned short)((double)FCY/(400000.0*2.0)-2.0+0.5);		//24AA64 only allows a speed of 400 kHz (if VCC>2.5V)
//	BRG_VAL=(unsigned short)((double)FCY/((float)I2CSpeed*2.0)-2.0+0.5);		//24AA64 only allows a speed of 400 kHz (if VCC>2.5V)
}
else{

//	BRG_VAL=(unsigned short)((double)FCY/(1200000.0*2.0)-2.0+0.5);				//1200000 = 1 MHz
	BRG_VAL=(unsigned short)((double)FCY/((float)I2CSpeed*2.0)-2.0+0.5);			//max working is 1900000 with 1.35 MHz

}
	//This function will initialize the I2C(1) peripheral.
	//First set the I2C(1) BRG Baud Rate.

	//Consult the dSPIC Data Sheet for information on how to calculate the
	//Baud Rate.
		
	I2C1BRG = BRG_VAL;

	//Now we will initialise the I2C peripheral for Master Mode, No Slew Rate
	//Control, and leave the peripheral switched off.
	
	I2C1CON = 0x1200;
	
	I2C1RCV = 0x0000;
	I2C1TRN = 0x0000;
	//Now we can enable the peripheral
	
	I2C1CON = 0x9200;
#endif
	return 0;
}

