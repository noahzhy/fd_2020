typedef union{
    unsigned short US;
    struct{
        unsigned sfrdata:8;
        unsigned read:1;
        unsigned adr:7;
		unsigned Config:8;
        };
}TAsicSFR;

unsigned int WriteConf(TAsicSFR reg);
unsigned int Read_RAM(unsigned char* data,TAsicSFR reg, unsigned char readnumber);
unsigned int Read_EEPROM(unsigned char* data,unsigned char ReadAdress, unsigned short readnumber,TAsicSFR reg);
unsigned int Write_EEPROMLH(unsigned short dataWrite,unsigned char WriteAdress,TAsicSFR reg);