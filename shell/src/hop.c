#include"../include/shell.h"
#include"../include/hop.h"

// we have a last working directory
// now its just hop that we need to fix 
// and also we need to do some last storing command thing
// you also need to change current working everytime



// so this works properly the only thing that is there now to make that  

// this is working fine



void last_change(){
    if(last_working[0]=='\0'){
        return;
    }

    char temp_string[PATH_MAX];
    strcpy(temp_string,current_working);
    if(chdir(last_working)!=0){
        perror("Error changing directory\n");
        return;
    }
    strcpy(last_working,temp_string);
    getcwd(current_working,sizeof(current_working));

}

// working fine
void parent(){

    char str[]="..";
    strcpy(last_working,current_working);
    if(chdir(str)!=0){
        perror("Error changing directory\n");
    }

    getcwd(current_working,sizeof(current_working));

    /// so now its changing but the thing is the changes have to be made on the directory that is like 
}

// this is working fine just need to add the thing that is like what do we say no arguments
void default_home(){
        strcpy(last_working,current_working); // this is for storing to the last_working directory thing
        if(chdir(psuedo_home)!=0){
            perror("Error changing current directory\n");
        }
        getcwd(current_working,sizeof(current_working));


}
 // this is also working fine
void change_name(char* name){
    // we need to change to name of it
    // now what we will do i
    // and i guess the current working directory should be the onw u are working with that now
    strcpy(last_working,current_working);
    if(chdir(name)!=0){ // it will auto care of absolute and relative path
        // perror("No such directory!\n");
         fprintf(stderr, "No such directory!\n");
        return;
    }
    getcwd(current_working,sizeof(current_working));


}


/// ig we also need to change the default printing thing cause it can go to parent ig 

void hop(char* tokens[],int tokc){



// ok so first one willl be hop always
// just keep doing the thing sequentially

if(tokc==1) default_home(); // thats is there is not argument provided than we should go to home directly



for(int i=1;i<tokc;i++){
    // so waht we will do now is that we have to do the thing that is called 
    if(strcmp(tokens[i],"-")==0){
        // there is also a condition that first any one hop command have to succed for that we have NULL command ig
        last_change();
    }else if(strcmp(tokens[i],"..")==0){
        parent();
    }else if(strcmp(tokens[i],".")==0){
        // do noting
    }else if(strcmp(tokens[i],"~")==0){
        default_home();
    }else{
        // means it a name
        change_name(tokens[i]);

    }
    // gonna implement that later so we can just do that 
    //else if(strcmp(""))
// we will return the index from where we have to make the command working


}




}