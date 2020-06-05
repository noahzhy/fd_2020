#define COMPDEFECTIVEPIX
#define SCALETOPTAT

/********************************************************************
 * Function:        void RecalibThOffset(void)
 *
 * Description:     This function does a recalibration by looking at a smooth picture
 *
 * Dependencies:    none
 *******************************************************************/
void RecalibThOffset(void){
unsigned int i,k,c,BlockCalib,NextBlock;
unsigned long PTAT,VddLong;
unsigned long CalibData[(unsigned short)Pixel+(unsigned short)PTATamount+(unsigned short)ELAMOUNT];
unsigned char eecpy[Pixel*2],BufferRAM[2];
unsigned short PixelValue,Pixel2Value,readback;
TAsicSFR I2Creg;
unsigned short AverageAmount;
float TAMB,common[3];
unsigned char ReadBlindFrame=0,GetBlindFrame=0;
char GetVddMeas=0,VddSampledBefore=0,VddSampledNow=0;
unsigned long VddSensorLong=0;
unsigned long long DividerVdd;
signed long VoltageLong2,NewDeltaVdd;
signed short DeltaVdd;
unsigned long long DividerVdd2;
signed long long VoltageLongLong;


	IEC0bits.INT4IE=0;  //neu
	sprintf(tempstring,"Recording...\r\n");
	putstringUDPUART(tempstring);
	U2RXREG=0;
	U2STA&0;
	IFS1bits.U2RXIF = 0;
	IEC0bits.T4IE=0;
	IEC1bits.U2TXIE=0;
	IEC0bits.INT1IE = 0;
    calibsocket = UDPOpen(30333,&CamServer,30333);

	//initialize all values to 0
    memset(CalibData,0,sizeof(CalibData));
	//start the sensor
    SensorWakeup(0);
    
	I2Creg.adr=0x1A;
	I2Creg.read=0x00;
	I2Creg.Config=0x01;
	I2Creg.sfrdata=0x09;		//always start with block 0
	WriteConf(I2Creg);
    sprintf(tempstring,".");
	for(k=0;k<(unsigned short)runtoBlindBeforePixC;k++){
        for(BlockCalib=0;BlockCalib<4;BlockCalib++){
            NextBlock=BlockCalib+1;
            if(NextBlock>3){
                NextBlock=0;
                while(!UDPIsPutReady(calibsocket));
                UDPPutArray((BYTE*)Voltage[0],DATALength);
                UDPFlush();
                while(!UDPIsPutReady(calibsocket));
                UDPPutArray((unsigned char*)&Voltage[0][DataLengthHalf],DATALength2);
                UDPFlush();
                StackTask();
            }
            //wait until EOC is set
           // DelayMs(tFR);
            do{
                    p=data;
                    I2Creg.adr=0x1A;
                    I2Creg.read=0x00;
                    I2Creg.Config=0x02;
                    Read_RAM(data,I2Creg,0x01);
            }while((data[0]&0x01)!=0x01);

            //readout the whole frame for both sides
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

            //start the sensor again
            I2Creg.adr=0x1A;
            I2Creg.read=0x00;
            I2Creg.Config=0x01;
            I2Creg.sfrdata=(NextBlock<<4)|0x09;
            WriteConf(I2Creg);
        }
        if(!(k%50)){		//print from time to time to show that the module is still alive
            putstringUDPUART(tempstring);
        }
    }
    sprintf(tempstring,"Blind frames captured, continuing with real values...\r\n");
    putstringUDPUART(tempstring);

	VddLong=0;
	for(k=0;k<(unsigned short)runtoPixCs;k++){
		for(BlockCalib=0;BlockCalib<4;BlockCalib++){
			NextBlock=BlockCalib+1;
			if(NextBlock>3){
	            NextBlock=0;
				GetVDD(0);
				VddLong+=(unsigned long)VDD;
	            while(!UDPIsPutReady(calibsocket));
	            UDPPutArray((BYTE*)Voltage[0],DATALength);
	            UDPFlush();
	            while(!UDPIsPutReady(calibsocket));
	            UDPPutArray((unsigned char*)&Voltage[0][DataLengthHalf],DATALength2);
	            UDPFlush();
	            StackTask();
				if(SENSORREV1)
					GetVddMeas^=1;
				//get the el. Offsets now                        
				if(!GetBlindFrame){
				    //Start the blind frames next
				    GetBlindFrame=1;
				}
				else{		//blind frame was started before, so read it now out
				    ReadBlindFrame=1;
				    GetBlindFrame=0;
				    if(SENSORREV1)
				        GetVddMeas^=1;  //toggle this once more because it was toggled 2 times due to the recall with NextBlock>3                           
				}       
			}
			//wait until EOC is set
		//	DelayMs(tFR);
			do{
				p=data;
				I2Creg.adr=0x1A;
				I2Creg.read=0x00;
				I2Creg.Config=0x02;
				Read_RAM(data,I2Creg,0x01);
			}while((data[0]&0x01)!=0x01);

			//readout the whole frame for both sides
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

				//start the sensor again
				I2Creg.adr=0x1A;
				I2Creg.read=0x00;
				I2Creg.Config=0x01;
                if(GetBlindFrame){
                    I2Creg.sfrdata=0x0B;			//blind frame
                }
                else{
	                if(!GetVddMeas){
	                    I2Creg.sfrdata=(NextBlock<<4)|0x09;
	                    VddSampledNow=0;
	                }	
	                else{
	                    I2Creg.sfrdata=(NextBlock<<4)|0x0D;		//start Vdd-Meas next
	                    VddSampledNow=1;
	                }                     
                }
				WriteConf(I2Creg);

			//and copy the Thermals and PTAT
			//and copy the Thermals and PTAT
			BufferRAM[1]=*(unsigned char*)(p++);
			BufferRAM[0]=*(unsigned char*)(p++);
			PixelValue=(unsigned short)BufferRAM[1];
			PixelValue=PixelValue<<8;
			PixelValue+=(unsigned short)BufferRAM[0];
			PixelValue=PixelValue<<(16-Resolution);                   
			if(!ReadBlindFrame){    //ignore PTAT for el. Offset
				if(SENSORREV1){
					if(VddSampledBefore){
						//CalibData[BlockCalib+(unsigned short)PIXEL+(unsigned short)PTATamount+(unsigned short)ELAMOUNT+1+(unsigned short)PTATamount]+=(unsigned long)PixelValue;
						VddSensorLong+=(unsigned long)PixelValue;
					}
					else{
						CalibData[BlockCalib]+=(unsigned long)PixelValue;
					}
				}
				else{
					CalibData[BlockCalib]+=(unsigned long)PixelValue;
				}               
			}
			for(i=0;i<PixelEighth;i++){
				BufferRAM[1]=*(unsigned char*)(p++);
				BufferRAM[0]=*(unsigned char*)(p++);
				PixelValue=(unsigned short)BufferRAM[1];
				PixelValue=PixelValue<<8;
				PixelValue+=(unsigned short)BufferRAM[0];
				PixelValue=PixelValue<<(16-Resolution);                  
				if(ReadBlindFrame){    //ignore PTAT for el. Offset
					CalibData[i+(unsigned short)PTATamount+(unsigned short)Pixel]+=(unsigned long)PixelValue;
				}
				else                      
					CalibData[i+(unsigned short)PTATamount+BlockCalib*PixelEighth]+=(unsigned long)PixelValue;
			}

			BufferRAM[1]=*(unsigned char*)(p2++);
			BufferRAM[0]=*(unsigned char*)(p2++);
			PixelValue=(unsigned short)BufferRAM[1];
			PixelValue=PixelValue<<8;
			PixelValue+=(unsigned short)BufferRAM[0];
			PixelValue=PixelValue<<(16-Resolution);                  
			if(!ReadBlindFrame){    //ignore PTAT for el. Offset               
				if(SENSORREV1){
					if(VddSampledBefore){
						//CalibData[j][BlockCalib+(unsigned short)PIXEL+(unsigned short)PTATamount+(unsigned short)ELAMOUNT+1+(unsigned short)PTATamount/2+(unsigned short)PTATamount]+=(unsigned long)PixelValue;
						VddSensorLong+=(unsigned long)PixelValue;
					}
					else{
						CalibData[BlockCalib+(unsigned short)PTATamount/2]+=(unsigned long)PixelValue;
					}
				}
				else{
					CalibData[BlockCalib+(unsigned short)PTATamount/2]+=(unsigned long)PixelValue;
				}               
			}
			VddSampledBefore=VddSampledNow;
			for(i=0;i<PixelEighth;i++){
			   BufferRAM[1]=*(unsigned char*)(p2++);
			   BufferRAM[0]=*(unsigned char*)(p2++);
			   PixelValue=(unsigned short)BufferRAM[1];
			   PixelValue=PixelValue<<8;
			   PixelValue+=(unsigned short)BufferRAM[0];
			   PixelValue=PixelValue<<(16-Resolution);                   
				if(ReadBlindFrame){    //ignore PTAT for el. Offset
					CalibData[i+(unsigned short)PTATamount+(unsigned short)Pixel+(unsigned short)ELAMOUNTHALF]+=(unsigned long)PixelValue;
				}
				else                        
					CalibData[i+(unsigned short)PTATamount+BlockCalib*PixelEighth+(unsigned short)Pixel/2]+=(unsigned long)PixelValue;
			}                   
			ReadBlindFrame=0;
			BlockCalib-=GetBlindFrame;
		}
	}
	PTAT=0;
	for(i=0;i<(unsigned short)PTATamount;i++){		//Maybe divide the calibration into PTAT1 and PTAT2 later?
		PTAT+=CalibData[i];
		CalibData[i]=0;								//reset PTAT data to 0 for Thermals
	}
	if(SENSORREV1)
		PTAT*=2;		//only half of the data was PTAT, the other half was Vdd    
	PTAT/=(unsigned short)PTATamount;
	common[0]=(float)PTAT;
	common[0]/=(float)runtoPixCs;
    PTAT=(unsigned short)common[0]; 
	TAMB=common[0]*PTATGrad+PTATOff;
    
    if(SENSORREV1){
        VddSensorLong/=4;		//mulitply by 2 due to every 2. Frame only and divide by 8 due to 4 blocks * 2
        VddSensorLong/=runtoPixCs;
        PixelValue=(unsigned short)VddSensorLong;
        sprintf(tempstring,"VddSensor was %u\r\n",PixelValue);
        putstringUDPUART(tempstring);
        VddLong=VddSensorLong;
    }  
//	DeltaVdd=(signed short)((signed long)VddLong-(signed long)VddRef);
	NewDeltaVdd=(signed long)PTATLong;
	NewDeltaVdd-=(signed long)PTATThermals[0];
	NewDeltaVdd*=((signed long)VddRef[1]-(signed long)VddRef[0]);
	NewDeltaVdd/=((signed long)PTATThermals[1]-(signed long)PTATThermals[0]);
	NewDeltaVdd=(signed long)Vddlong-(signed long)VddRef[0]-NewDeltaVdd;
	DeltaVdd=(signed short)NewDeltaVdd;
	DividerVdd=(unsigned long long)(pow(2.0,(float)VddScaling));
	DividerVdd2=(unsigned long long)(pow(2.0,(float)VddScalingOff));   
    //determine the average value of the center Pixels and scale everything later to these Pixel
    AverageAmount=0;
    common[2]=0.0;
    for(k=(SPALTE/2-2);k<(SPALTE/2+2);k++){
        for(i=(ZEILE/2-2);i<(ZEILE/2);i++){
            AverageAmount++;
            common[0]=(float)ThGrad[k+i*SPALTE];
#ifdef SCALETOPTAT
            //scale Thermals to PTAT instead of Ta
            common[0]*=PTAT;
#else
            common[0]*=TAMB;
#endif
            common[0]/=(float)pow(2.0,(float)gradScale);
            common[0]+=(float)ThOff[k+i*SPALTE];				//Thermals
            common[0]+=(float)((float)CalibData[(k+i*SPALTE)%ELAMOUNTHALF+(unsigned short)PTATamount+(unsigned short)Pixel]/(float)runtoPixCs);            
            VoltageLongLong=(signed long long)VddGrad[(k+i*SPALTE)%ELAMOUNTHALF];
            VoltageLongLong*=(signed long long)PTAT;
            //VoltageLongLong*=(signed long long)DeltaVdd;
            VoltageLongLong/=(signed long long)DividerVdd;
            VoltageLongLong+=(signed long long)VddOff[(k+i*SPALTE)%ELAMOUNTHALF];
            VoltageLongLong*=(signed long long)DeltaVdd;
            VoltageLongLong/=(signed long long)DividerVdd2;
            VoltageLong2=(signed long)VoltageLongLong;
			common[0]+=(float)VoltageLong2;			       
            common[1]=(float)CalibData[k+i*SPALTE+(unsigned short)PTATamount];
            common[1]/=(float)runtoPixCs;			//Pixeldata
            common[0]=common[1]-common[0];
            //now add the PixC compensation
            common[0]*=(float)PCSCALEVAL;
            common[0]/=(float)PixC[k+i*SPALTE];   
            common[2]+=common[0];
        }
        for(i=(ZEILE/2);i<(ZEILE/2+2);i++){
            AverageAmount++;
            common[0]=(float)ThGrad[k+(ZEILE+ZEILE/2-1-i)*SPALTE];
#ifdef SCALETOPTAT
            //scale Thermals to PTAT instead of Ta
            common[0]*=PTAT;
#else            
            common[0]*=TAMB;
#endif
            common[0]/=(float)pow(2.0,(float)gradScale);
            common[0]+=(float)ThOff[k+(ZEILE+ZEILE/2-1-i)*SPALTE];				//Thermals
            common[0]+=(float)((float)CalibData[(k+(ZEILE+ZEILE/2-1-i)*SPALTE)%ELAMOUNTHALF+(unsigned short)ELAMOUNTHALF+(unsigned short)PTATamount+(unsigned short)Pixel]/(float)runtoPixCs);
            VoltageLongLong=(signed long long)VddGrad[(k+(ZEILE+ZEILE/2-1-i)*SPALTE)%ELAMOUNTHALF+ELAMOUNTHALF];
            VoltageLongLong*=(signed long long)PTAT;
            //VoltageLongLong*=(signed long long)DeltaVdd;
            VoltageLongLong/=(signed long long)DividerVdd;
            VoltageLongLong+=(signed long long)VddOff[(k+(ZEILE+ZEILE/2-1-i)*SPALTE)%ELAMOUNTHALF+ELAMOUNTHALF];
            VoltageLongLong*=(signed long long)DeltaVdd;
            VoltageLongLong/=(signed long long)DividerVdd2;
            VoltageLong2=(signed long)VoltageLongLong;
			common[0]+=(float)VoltageLong2;			           
            common[1]=(float)CalibData[k+(ZEILE+ZEILE/2-1-i)*SPALTE+(unsigned short)PTATamount];
            common[1]/=(float)runtoPixCs;			//Pixeldata
            common[0]=common[1]-common[0];
            //now add the PixC compensation
            common[0]*=(float)PCSCALEVAL;
            common[0]/=(float)PixC[k+(ZEILE+ZEILE/2-1-i)*SPALTE];   
            common[2]+=common[0];
        }        
    }
    common[2]/=(float)AverageAmount;
	//now substract the Thermals and divide the amount of frames taken and do the average of the center Pixels.
    //Then multiply everything by the PixC and make an even picture by changing the Offset
	for(k=0;k<(unsigned short)Pixel;k++){		//substract the thermal Offsets
		common[0]=(float)ThGrad[k];
	//scale Thermals to PTAT instead of Ta
		common[0]*=PTAT;
		common[0]/=(float)pow(2.0,(float)gradScale);
		common[0]+=(float)ThOff[k];				//Thermals
		if(k>=Pixel/2){		//use right half of el. Offset
			common[0]+=(float)((float)CalibData[k%ELAMOUNTHALF+(unsigned short)ELAMOUNTHALF+(unsigned short)PTATamount+(unsigned short)Pixel]/(float)runtoPixCs);
		}
		else{				//use left half of el. Offset
			common[0]+=(float)((float)CalibData[k%ELAMOUNTHALF+(unsigned short)PTATamount+(unsigned short)Pixel]/(float)runtoPixCs);
		}			
		i=(unsigned short)(k/(Pixel/2));
		VoltageLongLong=(signed long long)VddGrad[k%ELAMOUNTHALF+i*ELAMOUNTHALF];
		VoltageLongLong*=(signed long long)PTAT;
		//VoltageLongLong*=(signed long long)DeltaVdd;
		VoltageLongLong/=(signed long long)DividerVdd;
		VoltageLongLong+=(signed long long)VddOff[k%ELAMOUNTHALF+i*ELAMOUNTHALF];
		VoltageLongLong*=(signed long long)DeltaVdd;
		VoltageLongLong/=(signed long long)DividerVdd2;
		VoltageLong2=(signed long)VoltageLongLong;
		common[0]+=(float)VoltageLong2;			       
		common[1]=(float)CalibData[k+(unsigned short)PTATamount];
		common[1]/=(float)runtoPixCs;			//Pixeldata
		common[0]=common[1]-common[0];
        //now add the PixC compensation
        common[0]*=(float)PCSCALEVAL;
        common[0]/=(float)PixC[k];        
        common[1]=common[2]-common[0];
        common[1]/=(float)PCSCALEVAL;
        common[1]*=(float)PixC[k]; 
        common[0]=(float)ThOff[k];
        common[0]-=common[1];
        common[0]+=0.5;
        ThOff[k]=(signed short)common[0];             
	}		

#ifdef COMPDEFECTIVEPIX
    //now mask out all dead/bad Pixel and replace them with a good Pixel from another block
    if(NrOfDefPix){
        for(i=0;i<NrOfDefPix;i++){
            if(DeadPixAdr[i]<Pixel/2){
                if(DeadPixAdr[i]<ELAMOUNTHALF){
                    //replace with a Pixel 128 further
                    ThOff[DeadPixAdr[i]]=ThOff[DeadPixAdr[i]+ELAMOUNTHALF];
                }
                else{
                    //replace with a Pixel 128 before
                    ThOff[DeadPixAdr[i]]=ThOff[DeadPixAdr[i]-ELAMOUNTHALF];
                }
            }
            else{
                if(DeadPixAdr[i]<ELAMOUNTHALF+Pixel/2){
                    //replace with a Pixel 128 further
                    ThOff[DeadPixAdr[i]]=ThOff[DeadPixAdr[i]+ELAMOUNTHALF];
                }
                else{
                    //replace with a Pixel 128 before
                    ThOff[DeadPixAdr[i]]=ThOff[DeadPixAdr[i]-ELAMOUNTHALF];
                }                            
            }
        }
    }
#endif  
	memcpy(eecpy,(char*)&ThOff,sizeof(short)*Pixel);
	HighDensPageWrite((unsigned short)AdrTh2,eecpy,sizeof(short)*Pixel);     

    sprintf(tempstring,"All Offsets are adapted to %d Digits\r\n",(signed short)common[2]);
    putstringUDPUART(tempstring);
    UDPClose(calibsocket);
	IEC0bits.INT1IE = 1;
	IEC0bits.T4IE=1;
	return;
}