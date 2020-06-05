/********************************************************************
 * Function:        determineGlobalGain(unsigned short warmestPixel, unsigned short GlobalGain)
 *
 * Description:     This function compares the warmest pixel with the target temperatur to 
					determine a new global gain
 *
 * Dependencies:    warmestPixel - warmest pixel of the array (or the average of the few warmest)
					GlobalGain - global gain (stored in EEPROM)
 *******************************************************************/
unsigned short determineGlobalGain(unsigned short warmestPixel, unsigned short GlobalGain){ 

	float NewGlobalGain;
	unsigned short targetTemp;
	signed int warmestPixelDigit, warmestPixelDigit;

	// 1.) SET TEMPERATURES
	// here you have to set the final temperatur (the temperature of your blackbody)
	targetTemp=(unsigned short) 4232;  // in dK (here 150°C)

	// 2.) DIGITS
	// now use the function calcTableVal to calculate the value in digits (before lookup table) 
	warmestPixelDigit = (signed int)calcTableVal(TA, (unsigned int)warmestPixel); // warmest pixel in digits
	targetTempDigit = (signed int)calcTableVal(TA, (unsigned int)setTemp); // target temperature in digits

	// 3.) CALCULATE NEW GLOBALGAIN
	NewGlobalGain=(float)GlobalGain;			// use the old Global Gain (stored in EEPROM)
	NewGlobalGain*=(float)warmestPixelDigit;		// multiply by warmest pixel (in digits)
	NewGlobalGain/=(float)targetTempDigit;		// devide by target temperature (in digits)

	// 4.) CALC NEW PixC
	for(i=0;i<1024;i++){
		   PixC[i]=(unsigned long)((float)PixC[i]*NewGlobalGain/(float)GlobalGain);
	}

	// 5.) SAVE
	// save the new global gain in the EERPOM
	GlobalGain=(unsigned short)NewGlobalGain;	// override your global gain
	memcpy((char*)&eecpy,(char*)&GlobalGain,sizeof(short));
	HighDensPageWrite((unsigned short)0x0055,(char*)&eecpy,sizeof(short));  

	return GlobalGain;
}



/********************************************************************
 * Function:        calcTableVal(unsigned int TAmb, unsigned int TObj)
 *
 * Description:     This function calculates the digits values that the pixel had
					before using the lookup table
 *
 * Dependencies:    TAmb - ambient temperature (sensors own temperature)
					TObj - object temperature (in dK)
 *******************************************************************/
signed int calcTableVal(unsigned int TAmb, unsigned int TObj){
	signed int value;
	unsigned int x,y,dTA,vx,vy;
	float ydist;


	//first check the position in x-axis of table
	for(x=0;x<NROFTAELEMENTS;x++){
		   if((XTATemps[x]<=TAmb)&&(XTATemps[x+1]>TAmb))
				break;
	}


	//now calculate deltaTA, deltaTO
	dTA=TAmb-XTATemps[x];

	for(y=0;y<NROFADELEMENTS;y++){  //now search for matching temperature
		   
		   if(dTA){
				vx=(unsigned int)(((TempTable[y][x+1]-TempTable[y][x])*(signed int)dTA)/(signed int)TAEQUIDISTANCE)+TempTable[y][x];
				vy=(unsigned int)(((TempTable[y+1][x+1]-TempTable[y+1][x])*(signed int)dTA)/(signed int)TAEQUIDISTANCE)+TempTable[y+1][x];
		   }
		   else{
				vx=TempTable[y][x];
				vy=TempTable[y+1][x];
		   }
		   
		   
		   if((vx<=TObj)&&(vy>=TObj))
				break;
	}


	if((y==NROFADELEMENTS-1)||(x==NROFTAELEMENTS-1)||(!TempTable[y][x]))
		   return -10000;  //return a value, which could not fit into 2^12 to signify, that values was beyond table limit

				
	value=(signed int)((float)(TObj-vx)/(float)(vy-vx)*(float)ADEQUIDISTANCE+(float)YADValues[y]-(float)TABLEOFFSET);
	value=(signed int)(value+0.5);


	return value;
}

