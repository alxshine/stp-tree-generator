struct ethhdr *ethh = (struct ethhdr*) bytes;

char buffer[17];
sprintf(buffer,"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", 
        ethh->h_dest[0], ethh->h_dest[1], 
        ethh->h_dest[2], ethh->h_dest[3], 
        ethh->h_dest[4], ethh->h_dest[5]);
if(std::string(buffer, 17) != "01:80:C2:00:00:00")
    return;
