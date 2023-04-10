#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ql_oe.h>
#include "ql_cell_locator.h"

void usage()
{
    printf("usage: test_locator_api [options]\n"
            "      before test locator, You must confirm that you are connected.\n"
            "options:\n"
            " -s --server, Set query server domain or ip, default is www.queclocator.com.\n"
            " -p --port, Connect server use given port, default is 80.\n"
            " -t --token, Used to verify client privilege, length must be 16 bytes\n"
            " -o --timeout, value of query timeout, default is 10 seconds\n"
            " -m --mode, Locate method.\n"
            "           1: single BS\n"
            "           2: Multi BS (discard)\n"
            "           3: triangle locate (server)\n"
            "           4: triangle locate (server and third service)\n"
            "           5: hybrid position location(BS and WIFI)\n"
            "           6: get a location base on WIFI\n"
            " eg: ./locator -s www.queclocator.com -p 80 -t xxxxxxxxxxxxxxxx -o 10 -m 5\n"
          );
}

int main(int argc, char **argv)
{
    int opt, opt_index;
    int ret, i;
    ql_cell_resp resp;
    char server[255] = "www.queclocator.com", token[17] = {0};
    unsigned short port = 80, timeout = 10, mode = 5;
    
    static struct option long_option[] = {
		{"server", required_argument, NULL, 's'},
		{"port", required_argument, NULL, 'p'},
		{"token", required_argument, NULL, 't'},
		{"timeout", required_argument, NULL, 'o'},
        {"mode", required_argument, NULL, 'm'},
		{"help", no_argument, NULL, 'h'},
		{0, 0, 0, 0},
    };

    while((opt = getopt_long(argc, argv, "s:p:t:o:m:h", long_option, &opt_index)) != -1){
        switch(opt){
            case 'h':{
                usage();
                exit(0);
                break;
            }
        }
    }


    ret = ql_cell_locator_init();
    if(ret)
    {
        printf("ql_cell_locator_init error.\n");
        return -1;
    }
    optind = 1;
    while((opt = getopt_long(argc, argv, "s:p:t:o:m:h", long_option, &opt_index)) != -1)
    {
        switch(opt)
        {
            case 0:
				break;
            case 's':
            {
                if(optarg != NULL)
                {
                    strncpy(server, optarg, sizeof(server));
                    ret = ql_cell_locator_set_server(server, port);
                    if(ret)
                    {
                        printf("ql_cell_locator_set_server error.\n");
                        return -1;
                    }
                }
            }
            break;
            case 'p':
            {
                if(optarg != NULL)
                {
                    port = strtoul(optarg, NULL, 0);
                    ret = ql_cell_locator_set_server(server, port);
                    if(ret)
                    {
                        printf("ql_cell_locator_set_server error.\n");
                        return -1;
                    }
                }
                else
                {
                    printf("port is missing, use default port in /etc/loc_cfg.conf\n");
                }
                break;
            }
            case 't':
            {
                if(optarg != NULL)
                {
                    strncpy(token, optarg, sizeof(token));
                    ret = ql_cell_locator_set_token(token, strlen(token));
                    if(ret)
                    {
                        printf("ql_cell_locator_set_token error.\n");
                        return -1;
                    }
                }
                else
                {
                    printf("token is missing. use default token in /etc/loc_cfg.conf\n");
                }     
                break;
            }

            case 'o':
            {
                if(optarg != NULL)
                {
                    timeout = strtoul(optarg, NULL, 0);
                    ret = ql_cell_locator_set_timeout(timeout);
                    if(ret)
                    {
                        printf("ql_cell_locator_set_timeout error.\n");
                        return -1;
                    }
                }
                else
                {
                    printf("timeout is missing, use default value in /etc/loc_cfg.conf\n");
                }
                break;   
            }
            case 'm':
            {
                if(optarg != NULL)
                {
                    mode = strtoul(optarg, NULL, 0);                   
                    ret = ql_cell_locator_set_mode(mode);
                    if(ret)
                    {
                        printf("ql_cell_locator_set_mode error.\n");
                        return -1;
                    }
                }
                else
                {
                    printf("mode is missing, use default mode in /etc/loc_cfg.conf\n");
                }
                break;
            }
        }  
    }
    if(optind != argc) {
		fprintf(stderr, "Expected argument after options\n"); 
		usage();
		exit(1);
	}

    /*the frequency of access to the service is 10 seconds, so we can see  'perform fast, retry later'*/
    for(i = 0; i < 15; i++)
    {
        ret = ql_cell_locator_perform(&resp);
        printf("err_code: %d, err_msg: %s\n", resp.err.err_code, resp.err.err_msg);
        if(ret == 0)
        {
            printf("lon: %f, lat: %f, accuracy: %d\n", resp.lon, resp.lat, resp.accuracy);
            if(resp.lon > 0.0 || resp.lat > 0.0){
                break;
            }
        }
        sleep(1);
    }
    ql_cell_locator_deinit();

    return 0;
}
