#include"../include/shell.h"
#include"../include/input.h"

void system_start_name(char* str_final){
    // we will just get host name by this 

    uid_t uid_system=getuid();
    struct passwd* pd=getpwuid(uid_system);
    char* username;
    if(pd){
        username=pd->pw_name;
            // char* home_dir=pd-> // you dont need shell and neither home directory cause the directory u are using is the home itself
    }else{
        perror("getpwuid\n");
    }


    char host_name[_SC_HOST_NAME_MAX+1];

    //char str_final[100];
    // here we will also need to get the current directory and have to remove psuedo directory accordingly 
            if(gethostname(host_name,sizeof(host_name))==0){
                    // we have the host name stored too
                    // now we have to 
                    // char str_final[100];
                    
                    
                    // we have to create a nee string out of the str_final cause then only we can use that 
                    char host_final[100];
                    int i=0;
                    for(i=0;i<strlen(host_name);i++){
                        // if(host_name[i]=='.') break;
                        host_final[i]=host_name[i];
                    }
                    host_final[i]='\0';
                    // printf("%s\n",host_final);
                   // printf("%s\n",str_final);
                    strcat(str_final,"<");
                    strcat(str_final,username);
                    strcat(str_final,"@");
                    strcat(str_final,host_final);
                    strcat(str_final,":");
                    // ohk so we haver a psuedo home and wht we can do is we can just check that the length of string is less
                    


                    if(getcwd(current_working,sizeof(current_working))!=NULL){
                        // now we have to make a new string for this why this shit is so tough
                        // anyways you have to write this logic once

                        char final_current[PATH_MAX];
                        // its nothing just start from the length of psuedo home directory
                        int a=strlen(psuedo_home);


                        if(a>strlen(current_working)){

                            strcat(str_final,current_working);
                            strcat(str_final,">");
                            return;

                            
                        }

                        int i=a;
                        for(i=a;i<strlen(current_working);i++){
                            // final_current[i-strlen()]// dont calcualte it again and again
                            final_current[i-a]=current_working[i];
                        }
                        final_current[i-a]='\0';
                        if(final_current[0]=='\0'){ // means this is a null string then what to do 
                            strcat(str_final,"~> ");
                        }else{
                           // strcat(str_final,"/");
                            strcat(str_final,final_current);
                            strcat(str_final,"> ");
                        }

                    }else{
                        perror("Error getting current directory\n");
                    }


                    // after this u also have to add the stuff for which directory u are in 
                   // printf("%s\n",str_final);
                    
                    // now we have the write name we just have to conctante now 
                
                    //printf("%s",str_final);
                    

            }else{
                perror("getting the host name\n");
            }

}

void default_state(){
   // while(wait(NULL)!=-1);// this is just used for thinking child that all the child processs is finished or not
    char final_name[PATH_MAX];
    final_name[0]='\0';
    system_start_name(final_name);
    printf("%s",final_name);

    // printf("<notamish@Amishs-MacBook-Air:~> ");
    fflush(stdout);
}

