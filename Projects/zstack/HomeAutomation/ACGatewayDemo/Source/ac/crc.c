
#define CRC_CCITT 0x1021 




/*CRC-CCITT 0x1021*/
unsigned int crc_cal_by_bit(const unsigned char* ptr, unsigned int len)
{
    unsigned long crc = 0;
    unsigned char i;
    while(len-- != 0)
    {
        for(i = 0x80; i != 0; i /= 2)
        {
            crc *= 2;
            if((crc&0x10000) !=0)
                crc ^= 0x11021;

            if((*ptr&i) != 0)    
                crc ^= CRC_CCITT;
        }
        ptr++;
    }
    return crc;
}


unsigned short crc16_ccitt(const unsigned char *buf, unsigned int len)
{
	return crc_cal_by_bit(buf,len);

}
