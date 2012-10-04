//
//  ultrasn0w.c
//  ultrasn0w 1.2.7
//
//  Created by djayb6 on 9/26/12.
//  Copyright (c) 2012 djayb6. No rights reserved.
//

#include "ultrasn0w.h"


#ifdef MAIN
MAIN
#elif defined CTOR
CTOR
#endif
{
    struct utsname device_info;
    FILE * logfile;
    char date;
    char *full_date;
    
    uname(&device_info);
    
    if (strcmp("iPhone3,1", device_info.machine) != 0) {
        
        dlopen("/usr/share/ultrasn0w/ultrasn0w-xgold608.dylib", 9);
        
        #ifdef CTOR
            return;
        #elif defined MAIN
            return 1;
        #endif
    }
    
    logfile = fopen("/var/wireless/Library/Logs/ultrasn0w-dylib.log", "w");
    

    timestamp(&date);
    printf("%s", &date);
    full_date = &date;
    //fprintf(logfile, "%s ultrasn0w loaded via CommCenter", full_date);
    //fflush(logfile);

    slide = _dyld_get_image_vmaddr_slide(0);
    
    size_t xsimstate_string_offset = FindString("+xsimstate=1") - slide;
    
    printf("%zx", xsimstate_string_offset);
    
    size_t relative_date1_offset = FindStringN("Mar 18 2011", 0xB);
    
    if (relative_date1_offset != 0) {
        
        fw = 431;
    }
    
    size_t relative_date2_offset = FindStringN("Apr  4 2011", 0xB);
    
    if (relative_date2_offset != 0) {
        
        fw = 432;
    }
    
    size_t relative_date3_offset = FindStringN("Apr 28 2011", 0xB);
    
    if (relative_date3_offset != 0) {
        
        fw = 433;
    }
    
    if (xsimstate_string_offset == 0x11CF51) {
        
        fw = 500;
    }
    
    if (xsimstate_string_offset == 0x11CEF4) {
        
        fw = 501;
    }
    
    if (xsimstate_string_offset == 0x10FD38) {
        
        fw = 51;
    }
    
    if (xsimstate_string_offset == 0x10FB68) {
        
        fw = 511;
    }
    
    
    size_t xcgedpage_string_offset = FindString("+xcgedpage=0,1");
    
    size_t xcgedpage_reference_offset = FindReference(xcgedpage_string_offset);
    
    if (xcgedpage_reference_offset != 0) {
        
        #ifdef CTOR
            return;
        #elif defined MAIN
            return 1;
        #endif
    }
    
    size_t internal_notif_string_offset = FindString("Sending internal notification %s (%d) params={%d, %d, %p}\n");
    size_t activation_ticket_string_offset = FindString("activation ticket accepted... drive thru\n");
    
    size_t internal_notif_reference_offset = FindReference(internal_notif_string_offset);
    size_t activation_ticket_reference_offset = FindReference(activation_ticket_string_offset);
    
    if (internal_notif_reference_offset == 0) {

        #ifdef CTOR
            return;
        #elif defined MAIN
            return 1;
        #endif
    }
    
    size_t internal_notice_function = FindLastThumbFunction(internal_notif_reference_offset);
    
    if (activation_ticket_reference_offset == 0) {
        
        #ifdef CTOR
            return;
        #elif defined MAIN
            return 1;
        #endif
    }
    
    size_t activation_ticket_function = FindLastThumbFunction(activation_ticket_reference_offset);
    
    MSHookFunction((void*)internal_notice_function, (void*)my_InternalNotification, (void**)&InternalNotification);
    MSHookFunction((void*)activation_ticket_function, (void*)my_XLockHandler, (void**)&XLockHandler);
    MSHookFunction((void*)open, (void*)my_open, (void**)&s_orig_open);

}

static void timestamp(char *date) {
    
    time_t current_time;
    
    time(&current_time);
    ctime_r(&current_time, date);
    
}

static size_t FindString(const char * string) {
    
    char*       c_string_data = NULL;
    uint32_t    size;
    
    if (slide == 0) {
        
        const struct mach_header * header = (const struct mach_header *)0x1000;
        
        c_string_data = getsectdatafromheader(header, SEG_TEXT, "__cstring", &size);
        
    }
    
    else {

        c_string_data = getsectdata(SEG_TEXT, "__cstring", (unsigned long*)&size) + slide;
        
    }
    
    unsigned int * c_string_end = (unsigned int*)c_string_data + size;
    
    size_t string_length = strlen(string);
    
    unsigned int * c_string_fixed = (unsigned int*)c_string_end - string_length;
   
    // BCC
    while (c_string_data < (char*)c_string_fixed) {
        
        if (strcmp(c_string_data, string)==0) {
            
            return (size_t)c_string_data;
        }
        
        c_string_data++;
        
    }
    
    return 0;
    
    
}

static size_t FindStringN(const char *string, size_t lentgh) {
    
    char*       c_string_data = NULL;
    uint32_t    size;
    
    if (slide == 0) {
        
        const struct mach_header * header = (const struct mach_header *)0x1000;
        
        c_string_data = getsectdatafromheader(header, SEG_TEXT, "__cstring", &size);
        
    }
    
    else {
        
        c_string_data = getsectdata(SEG_TEXT, "__cstring", (unsigned long*)&size) + slide;
        
    }
    
    unsigned int * c_string_end = (unsigned int*)c_string_data + size;
    
    size_t string_length = strlen(string);
    
    unsigned int * c_string_fixed = (unsigned int*)c_string_end - string_length;
    
    // BCC
    while ((unsigned char*)c_string_data < (unsigned char*)c_string_fixed) {
        
        if (strncmp(c_string_data, string, lentgh)==0) {
            
            return (size_t)c_string_data;
        }
        
        c_string_data++;
        
    }
    
    return 0;
}

static size_t FindReference(size_t address) {
    
    char* data =  NULL;
    uint32_t size;
    char *data_end_fixed;
    
    if (slide == 0) {
        
        const struct mach_header * header = (const struct mach_header *)0x1000;
        
        data = getsectdatafromheader(header, SEG_TEXT, SECT_TEXT, &size);
        
    }
    
    else {
        
        data = getsectdata(SEG_TEXT, SECT_TEXT, (unsigned long*)&size) + slide;
    }
    
    data_end_fixed = data + size - 4;
    
    while (data < data_end_fixed) {
        
        if (memcmp((void*)data, (void*)&address, 4) == 0) {
            
            return (size_t)data;
        }
    
        data++;
    }
    
    if (address == 0xAFBB8) return (size_t)0x55230;
    
    if (address == 0xA8E3C) return (size_t)0x2AE8C;
    
    address -= slide;
    
    if (fw == 431) {
        
        if (address == 0xF47AC) return (0x6039C + slide);
        
        if (address == 0xED6C0) return (0x34114 + slide);
        
        if (address == 0xF02EC) return (0x458B4 + slide);
        
    }
    
    else if (fw == 432) {
        
        if (address == 0xF47AC) return (0x60370 + slide);
        
        if (address == 0xED6C0) return (0x340E8 + slide);
        
        if (address == 0xF02EC) return (0x45888 + slide);
        
    }
    
    else if (fw == 433) {
        
        if (address == 0xF4738) return (0x6030C + slide);
        
        if (address == 0xED64C) return (0x34110 + slide);
        
        if (address == 0xF0278) return (0x45824 + slide);
        
    }
    
    else if (fw == 500) {
        
        if (address == 0x124DD5) return (0x680B4 + slide);
        
        if (address == 0x11CEC9) return (0x35704 + slide);
        
        if (address == 0x11FEA1) return (0x4BB24 + slide);
        
    }
    
    else if (fw == 501) {
        
        if (address == 0x124DD0) return (0x67B54 + slide);
        
        if (address == 0x11CE6C) return (0x34FA8 + slide);
        
        if (address == 0x11FE60) return (0x4B504 + slide);
    }
    
    else if (fw == 51) {
        
        if (address == 0x124DD0) return (0x67B54 + slide);
        
        if (address == 0x11CE6C) return (0x34FA8 + slide);
        
        if (address == 0x11FE60) return (0x4B504 + slide);
    }
    
    else if (fw == 511) {
        
        if (address == 0x117BF8) return (0x6B2C0 + slide);
        
        if (address == 0x10FAE0) return (0x3838C + slide);
        
        if (address == 0x112AED) return (0x4EA78 + slide);
    }
    
    else if (fw == 60) {
        
        // lulz
        
        return 0;
    }
    
    else {
        
        return 0;
    }
    
    return 0;
}

static size_t FindLastThumbFunction(size_t address) {
    
    size_t          current_address;
    uint32_t        size;
    char*           data                  = NULL;
    unsigned char*  fixed_current_address = NULL;
    
    if (slide == 0) {
        
        const struct mach_header * header = (const struct mach_header *)0x1000;
        
        data = getsectdatafromheader(header, SEG_TEXT, SECT_TEXT, &size);
        
    }
    
    else {
        
        data = getsectdata(SEG_TEXT, SECT_TEXT, (unsigned long*)&size) + slide;
    }
    
    current_address = address &~(3);
    fixed_current_address = (unsigned char*)current_address;
    
    while (fixed_current_address >= (unsigned char*)data) {
        
        fixed_current_address -=2;
        
        // disassembly shows a second condition: MSB != 0xFO. this is a valid prologue though
        if ((fixed_current_address[0] & 0xFF) == 0xB5)
            return (size_t)fixed_current_address+1;
    }
    
    return 0;
}

static int my_open(char *filename, int flags) {
    
    int ret;
    ret = s_orig_open(filename, flags);
    
    if (strcmp(filename, "/dev/dlci.spi-baseband.low") != 0) {
        
        return ret;
    }
    
    if (fw != 431) {
        
        lowFD = ret;
        
        
    } else {
        
        lowFD = s_orig_open("/dev/dlci.spi-baseband.extra_0", flags);
    }
    
    return ret;
    
}

static int InitConn(speed_t speed) {
    
    int fd = open("/dev/tty.debug", 0x20002);
    
    if (fd < 0) {
        
        perror("open");
        return 1;
    }
    
    ioctl(fd, 0x2000740D);
    
    fcntl(fd, 4, 0);
    
    tcgetattr(fd, term);
    
    ioctl(fd, 0x8004540A, 0);
    
    cfsetspeed(term, speed);
    
    cfmakeraw(term);
    
    term->c_cc[17] = 5;
    
    term->c_iflag &= 0xFFFFF0C8;
    term->c_iflag |= 5;
    
    term->c_cc[16] = 0;
    
    term->c_oflag &= ~1;
    
    term->c_cflag |= 0x300;
    term->c_cflag &= ~0x1000;
    
    term->c_lflag &= 0xFFFFFA77;
    
    tcsetattr(fd, 0, term);
    
    ioctl(fd, 0x20007479);
    
    ioctl(fd, 0x20007478);
    
    ioctl(fd, 0x8004746D, 0);
    
    return fd;
    
}

static int CheckCompatibility() {
    
    int     max_round;
    int     i;
    char*   bb_response;
    bool    got_response = false;;
    
    if (fw >= 500)
        max_round = 2;
    else
        max_round = 5;
    
    
    for (i = 0; max_round > i; i++) {
        
        bb_response = BasebandVersion();
        
        if (bb_response != NULL && (bb_response[0] != '\0')) {
            
            got_response = true;
            break;
        }
    }
    
    if (got_response || ( (bb_response != NULL) && (bb_response[0] != '\0')) ) {
        
        if (strcmp(bb_response, "01.59.00") == 0) {
            
            return 1;
        }
        
        else {
            
            // fprintf error
            return 0;
        }
        
    }
    
    else {
        
        if (fw < 500) {
            
            // fprintf error
            return 0;
            
        }
        
        else {
            
            return 1;
            
        }
    }
}

static char* BasebandVersion() {
    
    char*   version = NULL; // ret
    int     ret;
    int     i       = 0;
    int     fd      = InitConn(B115200);
    
    AT(fd);
    
    while (i != 50) {
    
        SendStrCmd(fd, "AT+XGENDATA\r");
    
        ret = ReadResp(fd, 0, 100000);
    
        if (ret > 0) {
        
            if (strstr(read_buf, "ICE3_MODEM_") != NULL) {
                
                break;
            }
        }
        
        else if (ret < 0) {
            
            AT(fd);
        }
        
        i++;
        
        if (i == 50) {
            
            close(fd);
            return version;
        }
    }

    strncpy(version, read_buf+0xB, 0xF);
    
    i = 0;
    
    while (i < 0xF) {
        
        if (version[i] == '\0') {
            
            break;
        }
        
        else if (version[i] == 0x5F) {
            
            version[i] = '\0';
            break;
        }
    
        i++;
    }
    
    
    close(fd);
    return version;
}
 
static int AT(int fildes) {
    
    int i = 0;
    int ret;
    
    SendAT(fildes);
    
    while (i < 0xF) {
        
        ret = ReadResp(fildes, 0, 500000);
        
        if (ret == 0) return 1;
        
            SendAT(fildes);
            i++;
    }
    
    return ret;
    
}

static void SendAT(int fildes) {
    
    SendStrCmd(fildes, "AT\r");
}

static void SendStrCmd(int fildes, const char* command) {
    
    SendCmd(fildes, command, strlen(command));
    
}

static int SendCmd(int fildes, const char* command, size_t length) {
    
    ssize_t ret;
    
    ret = write(fildes, command, length);
    
    if (ret <= 0)
        return 1;
    else
        return 0;
}

static int ReadResp(int fildes, int a, int b) {
    
    fd_set          rfds;
    struct timeval  tv;
    int             ret;
    int             current_size;
    int             total_length = 0;
    int             read_bytes;
    
    memset(&rfds, '\0', 0x80);
    
    FD_SET(fildes, &rfds);
    
    tv.tv_sec = a;
    tv.tv_usec = b;
    
    // bad loop
    while (1) {
    
        ret = select(fildes+1, &rfds, NULL, NULL, &tv);
    
        if (ret <= 0) {
        
        return 0;
        
        }
        
        else {
        
            current_size = 0x10000 - total_length;
        
            read_bytes = read(fildes, read_buf, current_size);
            
            total_length += read_bytes;
        
            read_buf[total_length] = '\0';
        
            if (strstr(read_buf, "\r\nOK\r\n") != 0) {
            
                return total_length; }
        
            if (strstr(read_buf, "\r\nERROR\r\n") != 0) {
            
                return total_length; }
        }
    }
}

static int atprog(char** array, int length) {
    
    int i = 0;
    
    int fd = InitConn(B115200);
    
    if (AT(fd) == 0) return -1;
    
    while (i < length) {
        
        
        int string_length = strlen(array[i]);
        
        char* buffer = malloc(string_length+7);
        
        strcpy(buffer, array[i]);
        
        int buffer_length = strlen(buffer);
        
        if ((buffer-1)[buffer_length] == 0xA)
            (buffer-1)[buffer_length] = '\0';
        
        int buffer_length_new = strlen(buffer);
        
        memcpy((buffer+buffer_length_new), "\r", 2);
        
        if (buffer != 0) {
            
            int round = 0;
            
            while (round  != 3) {
                
                SendStrCmd(fd, buffer);
                
                if (ReadResp(fd, 0, 100000) > 0) {
                    
                    free(buffer);
                    break;
                }
                
                AT(fd);
                round++;
                AT(fd);
                
                if (round == 3) {
                    
                    if (buffer != NULL) free(buffer);
                    close(fd);
                    return 0;
                    
                }
            }
        }
    }
    
    close(fd);
    return  0;
    
}

static void my_XLockHandler(void* a, char** command, void* c, void* d) {
    
    pthread_mutex_lock(&at_mutex);
    
    if ( isCompatible && (strstr("+XLOCK: \"PN\",1,1", *command) != 0) ) {
    
    write(lowFD, "at+xpow=1\r", 0xA);
    
    atprog(activation_ticket, 6);
    
    write(lowFD, "at+xpow=0\r", 0xA);
    
    }
    
    pthread_mutex_unlock(&at_mutex);
    
    XLockHandler(a, command, c, d);
    
    
}

static void my_InternalNotification(char* a, int b, int c, int d, void* e) {
    
   pthread_mutex_lock(&at_mutex);
    
    if (b == 7) {
    
    write(lowFD, "at+xpow=1\r", 0xA);
    
    isCompatible = CheckCompatibility();
    
        if (isCompatible) {
        
            /*
         
             atprog(unlock_strings, 4);
         
             */
        }
    
    write(lowFD, "at+xpow=0\r", 0xA);
    
    }
    
    pthread_mutex_unlock(&at_mutex);
    
    InternalNotification(a, b, c, d, e);
    
    
    
}

