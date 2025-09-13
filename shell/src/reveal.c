#include"../include/shell.h"
#include"../include/reveal.h"

// now i guess the whole reveal is correct 
// bruh it was just this prasing and because of this i have to take so much time
// its okay gng


// likw the path of it need to check that for the path baby // only a if statement needs to be added
// still not correct also need to add the "-" thingy of the last command

int comp(const void* a, const void* b) {
    char* s1 = *(char**)a;
    char* s2 = *(char**)b;
    return strcmp(s1, s2);
}




void list(int a,int l,char* path){
/// this is for that flags are set or not

DIR* dir=opendir(path);
if(!dir){
    printf("No such directory!\n");
    return;
}

char* files[1024];
int i=0;
struct dirent* entry;

int max_len=0;
while((entry=readdir(dir))!=0){
    if(entry->d_name[0]=='.'&&a==0) continue;
    
    files[i++]=strdup(entry->d_name);
    int len=strlen(entry->d_name);
    if(len>max_len) max_len=len;
}
closedir(dir);



qsort(files,i,sizeof(char*),comp);

if(l==1){
    for(int j=0;j<i;j++){
        printf("%s\n",files[j]);
    }
}else{
    /// LLM GENERATED CODE STARTS ///
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int term_width = w.ws_col;
        int col_width = max_len + 2;

        if (col_width > term_width) {
            col_width = term_width;
        }

        int current_line_len = 0;
        for (int j = 0; j < i; j++) {
            if (current_line_len > 0 && current_line_len + col_width > term_width) {
                printf("\n");
                current_line_len = 0;
            }
            printf("%-*s", col_width, files[j]);
            current_line_len += col_width;
        }
        printf("\n");


        /// LLM GENERATED CODE ENDS ///
}

for(int j=0;j<i;j++){
    free(files[j]);
}

}

// see the thing is that there is diff b.w argments and input 
// you will get arguments from there only so chill input 
// input mai kabhi bhi arguments nahi aayenge


// also need to implement that if it is with too many arguments 



void reveal(char* tokens[],int tokc){


/// ohk so all we need to implement is that how are we gonna make this shit happen correct
// we did a lot of random shit that just dont matter
// bro things could have been god damn easy
int flag_a=0;
int flag_l=0;
char* path_final=NULL;
for(int i=1;i<tokc;i++){ // cause the first token will be command always
    if(tokens[i][0]=='-'&&strlen(tokens[i])>=2){ // this strlen condn is for what is - is there cause then we will consider it as previous directory
        // means this cant be the name and something else
       for(int j=0;j<strlen(tokens[i]);j++){
        if(tokens[i][j]=='a') flag_a=1;
        if(tokens[i][j]=='l') flag_l=1;
       }
       continue;
    }


    // now whatever is this we will get that // now we can just see 
    // but there is a problem and it is 
    // if it has come till here means there is something that is not being mentioned // but what if it is not that 
    // like then u wont be calling that 
    
    if(path_final!=NULL){
        printf("reveal: Invalid Syntax!\n");
        return;
    }
    path_final=tokens[i];
    // flag++;
    // index=i;
    // break;
}
// if(flag>=2){
//     printf("reveal: Invalid Syntax!\n");
// }else if(flag==1){
//     list(flag_a,flag_l,tokens[index]);
// }else{
//     list(flag_a,flag_l,".");
// }

if(path_final==NULL){
    list(flag_a,flag_l,".");
}else if(strcmp(path_final,"-")==0){
    if(last_working[0]=='\0') {
        printf("No such directory!\n");
        return;
    }
    list(flag_a,flag_l,last_working);

}else if(strcmp(path_final,"~")==0){
    list(flag_a,flag_l,psuedo_home);
}else{
    list(flag_a,flag_l,path_final); /// like name or something kind of shit
}

}