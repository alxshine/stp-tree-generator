#include "../inc/Sniffer.hpp"

Sniffer::Sniffer(const char *filename){
   if(!strcmp(filename, "stdout"))
       output = stdout;

   output = fopen(filename, "a+");
   if(output == NULL){
       perror("Could not open stream");
       exit(-1);
   }
}

Sniffer::~Sniffer(){
    if(output != stdout)
        fclose(output);
}

inline void Sniffer::ERRCHECK(int condition, const char *format, va_list arg){
    if(condition)
        vfprintf(output, format, arg);
}

void Sniffer::start(){
    fprintf(output, "sniffer starting\n");
    fprintf(output, "getting interfaces\n");
    pcap_if_t *alldevs;
    char err[100];
    if(pcap_findalldevs(&alldevs, err)){
        fprintf(output, "error while getting interfaces, error code: %s\n", err);
        exit(-1);
    }

    pcap_if_t *device;
    fprintf(output, "found devices:\n");
    for(device = alldevs; device != NULL; device = device->next)
        fprintf(output, "%s - %s", device->name, device->description);

    fprintf(output, "selecting first named device\n");
    for(device = alldevs; device != NULL && device->name == NULL; device = device->next)
        ;
    if(device == NULL)
        fprintf(output, "Could not find a named device");


}

