#define AdrPixCMin 0x00
#define AdrPixCMax 0x04
#define AdrGradScale 0x08
#define AdrTableNumber 0x0B 		//changed to 0x0B with Rev 0.14 and adpated TN readout
#define AdrEpsilon 0x0D

#define AdrMBITPixC 0x1A
#define AdrBIASPixC 0x1B
#define AdrCLKPixC 0x1C
#define AdrBPAPixC 0x1D
#define AdrPUPixC 0x1E

#define AdrVddMeasTh1 0x26
#define AdrPTATTh1 0x3C
#define AdrVddTh1 0x46

#define AdrPTATGrad 0x34

#define AdrVddScaling 0x4E
#define AdrVddScalingOff 0x4F

#define AdrMBITUser 0x60
#define AdrBIASUser 0x61
#define AdrCLKUser 0x62
#define AdrBPAUser 0x63
#define AdrPUUser 0x64

#define AdrDevID 0x74

#define AdrGlobalOffset 0x54
#define AdrGlobalGain 0x55

#define AdrVddCompValues2 0x340
#define AdrVddCompValues 0x540
#define AdrTh1 0x740
#define AdrTh2 0xF40
#define AdrPixC 0x1740

#define BIAScurrentDefault 0x05
#define CLKTRIMDefault 0x15 //0x20 to let it run with 10 Hz
#define BPATRIMDefault 0x0C
//#define MBITTRIMDefault 0x0C
#define PUTRIMDefault	0x88


//pixelcount etc. for 32x32d
#define Pixel 1024				//=32x32
#define PixelEighth 128
#define LINE 32
#define COLUMN 32
#define DATALength 1292//1098					//length of first packet
#define DATALength2 1288//1096					//lenght of second/last packet
#define DataLengthHalf 646
#define PTATamount 8
#define ELOFFSET 1024			//start address of el. Offset
#define ELAMOUNT 256
#define ELAMOUNTHALF 128
#define StackSize 16			//must be choosen by the user!
#define PTATSTARTADSRESS 1282
#define VDDADDRESS 1280

#define GetElEveryFrameX 10		//amount of normal frames to capture after which the el. Offset is fetched
#define STACKSIZEPTAT 30		//should be an even number
#define STACKSIZEVDD 50			//should be an even number
#define VddStackAmount 30

#define AdjustOffsetGain		//should be set to use the GlobaGain variable

#define ReadToFromTable
#ifdef ReadToFromTable		//choose only one of them!
		//#define HTPA32x32dR1L1_6HiGe_Gain3k3
		//#define HTPA32x32dR1L2_1SiF5_0_N2
		//#define HTPA32x32dL2_1HiSiF5_0_Gain3k3			//is used for SensorRev. 1	
		//#define HTPA32x32dR1L1k8_0k7HiGe		
		//#define HTPA32x32dR1L2_1HiSiF5_0_Precise	
		//#define HTPA32x32dR1L2_1HiSiF5_0_Gain3k3_Extended
		//#define HTPA32x32dR1L2_85Hi_Gain3k3		
		//#define HTPA32x32dR1L3_6HiSi_Rev1_Gain3k3
		#define HTPA32x32dR1L3_6HiSi_Rev1_Gain3k3_TaExtended	//same like the above but with a larger working ambient temperature range
		//#define HTPA32x32dR1L5_0HiGeF7_7_Gain3k3	
		//#define HTPA32x32dR1L5_0HiGeF7_7_Gain3k3_TaExtended	//same like the above but with a larger working ambient temperature range
		//#define HTPA32x32dR1L7_0HiSi_Gain3k3

	
    #ifdef HTPA32x32dR1L5_0HiGeF7_7_Gain3k3
	 	#define TABLENUMBER		113
		#define PCSCALEVAL		100000000 //327000000000		//PixelConst scale value for table... lower 'L' for (long)
		#define NROFTAELEMENTS 	7
		#define NROFADELEMENTS 	1595	//130 possible due to Program memory, higher values possible if NROFTAELEMENTS is decreased
		#define TAEQUIDISTANCE	100		//dK
		#define ADEQUIDISTANCE	64		//dig
		#define ADEXPBITS		6		//2^ADEXPBITS=ADEQUIDISTANCE
		#define TABLEOFFSET		1024
		#define EQUIADTABLE		//if defined, ADELEMENTS have to be 2^N quantizied! else more CPU Power is needed
		#ifdef EQUIADTABLE
			#undef FLOATTABLE
		#endif   
		#define MBITTRIMDefault 0x2C
		#define SensRv 1
    #endif	
	
    #ifdef HTPA32x32dR1L5_0HiGeF7_7_Gain3k3_TaExtended
	 	#define TABLENUMBER		113
		#define PCSCALEVAL		100000000 //327000000000		//PixelConst scale value for table... lower 'L' for (long)
		#define NROFTAELEMENTS 	12
		#define NROFADELEMENTS 	1595	//130 possible due to Program memory, higher values possible if NROFTAELEMENTS is decreased
		#define TAEQUIDISTANCE	100		//dK
		#define ADEQUIDISTANCE	64		//dig
		#define ADEXPBITS		6		//2^ADEXPBITS=ADEQUIDISTANCE
		#define TABLEOFFSET		1024
		#define EQUIADTABLE		//if defined, ADELEMENTS have to be 2^N quantizied! else more CPU Power is needed
		#ifdef EQUIADTABLE
			#undef FLOATTABLE
		#endif   
		#define MBITTRIMDefault 0x2C
		#define SensRv 1
    #endif		

    #ifdef HTPA32x32dR1L1_6HiGe_Gain3k3
	 	#define TABLENUMBER		119
		#define PCSCALEVAL		100000000 //327000000000		//PixelConst scale value for table... lower 'L' for (long)
		#define NROFTAELEMENTS 	7
		#define NROFADELEMENTS 	1595	//130 possible due to Program memory, higher values possible if NROFTAELEMENTS is decreased
		#define TAEQUIDISTANCE	100		//dK
		#define ADEQUIDISTANCE	64		//dig
		#define ADEXPBITS		6		//2^ADEXPBITS=ADEQUIDISTANCE
		#define TABLEOFFSET		1024
		#define EQUIADTABLE		//if defined, ADELEMENTS have to be 2^N quantizied! else more CPU Power is needed
		#ifdef EQUIADTABLE
			#undef FLOATTABLE
		#endif   
		#define MBITTRIMDefault 0x2C
		#define SensRv 1
    #endif	

    #ifdef HTPA32x32dR1L2_1SiF5_0_N2
	 	#define TABLENUMBER		130
		#define PCSCALEVAL		100000000 //327000000000		//PixelConst scale value for table... lower 'L' for (long)
		#define NROFTAELEMENTS 	7
		#define NROFADELEMENTS 	1595	//130 possible due to Program memory, higher values possible if NROFTAELEMENTS is decreased
		#define TAEQUIDISTANCE	100		//dK
		#define ADEQUIDISTANCE	64		//dig
		#define ADEXPBITS		6		//2^ADEXPBITS=ADEQUIDISTANCE
		#define TABLEOFFSET		192
		#define EQUIADTABLE		//if defined, ADELEMENTS have to be 2^N quantizied! else more CPU Power is needed
		#ifdef EQUIADTABLE
			#undef FLOATTABLE
		#endif   
		#define MBITTRIMDefault 0x2C
		#define SensRv 1
    #endif	
	
    #ifdef HTPA32x32dL2_1HiSiF5_0_Gain3k3
	 	#define TABLENUMBER		114
		#define PCSCALEVAL		100000000 //327000000000		//PixelConst scale value for table... lower 'L' for (long)
		#define NROFTAELEMENTS 	7
		#define NROFADELEMENTS 	1595	//130 possible due to Program memory, higher values possible if NROFTAELEMENTS is decreased
		#define TAEQUIDISTANCE	100		//dK
		#define ADEQUIDISTANCE	64		//dig
		#define ADEXPBITS		6		//2^ADEXPBITS=ADEQUIDISTANCE
		#define TABLEOFFSET		1024
		#define EQUIADTABLE		//if defined, ADELEMENTS have to be 2^N quantizied! else more CPU Power is needed
		#ifdef EQUIADTABLE
			#undef FLOATTABLE
		#endif   
		#define MBITTRIMDefault 0x2C	//use REF_CAL=2 here. Table does not match, so GlobalGain ist set to 50 % to compensate this.
		#define SensRv 1				//Sensor Revision is set to 1 (Redesign)
    #endif	
	
    #ifdef HTPA32x32dR1L2_1HiSiF5_0_Gain3k3_Extended
	 	#define TABLENUMBER		114
		#define PCSCALEVAL		100000000 //327000000000		//PixelConst scale value for table... lower 'L' for (long)
		#define NROFTAELEMENTS 	12
		#define NROFADELEMENTS 	1595	//130 possible due to Program memory, higher values possible if NROFTAELEMENTS is decreased
		#define TAEQUIDISTANCE	100		//dK
		#define ADEQUIDISTANCE	64		//dig
		#define ADEXPBITS		6		//2^ADEXPBITS=ADEQUIDISTANCE
		#define TABLEOFFSET		1792
		#define EQUIADTABLE		//if defined, ADELEMENTS have to be 2^N quantizied! else more CPU Power is needed
		#ifdef EQUIADTABLE
			#undef FLOATTABLE
		#endif   
		#define MBITTRIMDefault 0x2C
		#define SensRv 1
    #endif	
 	
    #ifdef HTPA32x32dR1L2_1HiSiF5_0_Precise
	 	#define TABLENUMBER		116
		#define PCSCALEVAL		100000000 //327000000000		//PixelConst scale value for table... lower 'L' for (long)
		#define NROFTAELEMENTS 	22
		#define NROFADELEMENTS 	1000	//130 possible due to Program memory, higher values possible if NROFTAELEMENTS is decreased
		#define TAEQUIDISTANCE	50		//dK
		#define ADEQUIDISTANCE	32		//dig
		#define ADEXPBITS		5		//2^ADEXPBITS=ADEQUIDISTANCE
		#define TABLEOFFSET		1024
		#define EQUIADTABLE		//if defined, ADELEMENTS have to be 2^N quantizied! else more CPU Power is needed
		#ifdef EQUIADTABLE
			#undef FLOATTABLE
		#endif   
		#define MBITTRIMDefault 0x2C
		#define SensRv 1
    #endif	
	
    #ifdef HTPA32x32dR1L2_85Hi_Gain3k3
	 	#define TABLENUMBER		127
		#define PCSCALEVAL		100000000 //327000000000		//PixelConst scale value for table... lower 'L' for (long)
		#define NROFTAELEMENTS 	7
		#define NROFADELEMENTS 	1595	//130 possible due to Program memory, higher values possible if NROFTAELEMENTS is decreased
		#define TAEQUIDISTANCE	100		//dK
		#define ADEQUIDISTANCE	64		//dig
		#define ADEXPBITS		6		//2^ADEXPBITS=ADEQUIDISTANCE
		#define TABLEOFFSET		1024
		#define EQUIADTABLE		//if defined, ADELEMENTS have to be 2^N quantizied! else more CPU Power is needed
		#ifdef EQUIADTABLE
			#undef FLOATTABLE
		#endif   
		#define MBITTRIMDefault 0x2C
		#define SensRv 1
    #endif	

    #ifdef HTPA32x32dR1L3_6HiSi_Rev1_Gain3k3
	 	#define TABLENUMBER		117
		#define PCSCALEVAL		100000000		//PixelConst scale value for table... lower 'L' for (long)
		#define NROFTAELEMENTS 	7
		#define NROFADELEMENTS 	1595	//130 possible due to Program memory, higher values possible if NROFTAELEMENTS is decreased
		#define TAEQUIDISTANCE	100		//dK
		#define ADEQUIDISTANCE	64		//dig
		#define ADEXPBITS		6		//2^ADEXPBITS=ADEQUIDISTANCE
		#define TABLEOFFSET		1024
		#define EQUIADTABLE		//if defined, ADELEMENTS have to be 2^N quantizied! else more CPU Power is needed
		#ifdef EQUIADTABLE
			#undef FLOATTABLE
		#endif
		#define MBITTRIMDefault 0x2C
		#define SensRv 1
	#endif	
	
  #ifdef HTPA32x32dR1L3_6HiSi_Rev1_Gain3k3_TaExtended
	 	#define TABLENUMBER		117
		#define PCSCALEVAL		100000000		//PixelConst scale value for table... lower 'L' for (long)
		#define NROFTAELEMENTS 	12
		#define NROFADELEMENTS 	1595	//130 possible due to Program memory, higher values possible if NROFTAELEMENTS is decreased
		#define TAEQUIDISTANCE	100		//dK
		#define ADEQUIDISTANCE	64		//dig
		#define ADEXPBITS		6		//2^ADEXPBITS=ADEQUIDISTANCE
		#define TABLEOFFSET		1024
		#define EQUIADTABLE		//if defined, ADELEMENTS have to be 2^N quantizied! else more CPU Power is needed
		#ifdef EQUIADTABLE
			#undef FLOATTABLE
		#endif
		#define MBITTRIMDefault 0x2C
		#define SensRv 1
	#endif		
	
    #ifdef HTPA32x32dR1L7_0HiSi_Gain3k3
	 	#define TABLENUMBER		118
		#define PCSCALEVAL		100000000		//PixelConst scale value for table... lower 'L' for (long)
		#define NROFTAELEMENTS 	7
		#define NROFADELEMENTS 	1595	//130 possible due to Program memory, higher values possible if NROFTAELEMENTS is decreased
		#define TAEQUIDISTANCE	100		//dK
		#define ADEQUIDISTANCE	64		//dig
		#define ADEXPBITS		6		//2^ADEXPBITS=ADEQUIDISTANCE
		#define TABLEOFFSET		640
		#define EQUIADTABLE		//if defined, ADELEMENTS have to be 2^N quantizied! else more CPU Power is needed
		#ifdef EQUIADTABLE
			#undef FLOATTABLE
		#endif
		#define MBITTRIMDefault 0x2C
		#define SensRv 1
	#endif	
	
    #ifdef HTPA32x32dR1L1k8_0k7HiGe
	 	#define TABLENUMBER		115	
		#define PCSCALEVAL		100000000		//PixelConst scale value for table... lower 'L' for (long)
		#define NROFTAELEMENTS 	10
		#define NROFADELEMENTS 	471		//possible due to Program memory, higher values possible if NROFTAELEMENTS is decreased
		#define TAEQUIDISTANCE	100		//dK
		#define ADEQUIDISTANCE	64		//dig
		#define ADEXPBITS		6		//2^ADEXPBITS=ADEQUIDISTANCE
		#define TABLEOFFSET		1024
		#define EQUIADTABLE		//if defined, ADELEMENTS have to be 2^N quantizied! else more CPU Power is needed
		#ifdef EQUIADTABLE
			#undef FLOATTABLE
		#endif   
		#define MBITTRIMDefault 0x2C
		#define SensRv 1
    #endif	
#endif
