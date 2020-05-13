#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#define WR_DATA _IOW('a','a',int32_t *)  //for data transfer between kernel and user space 

int output1[100],output2[100];  //to store output values
int booleanfun(void);  //functio n decleration 
int ran(void); //functio n decleration 
static int j=0,k=0;

void* _inputData1(void* sleepTime){    //function for 1 and 2 threads
	int final_array[100],result=0,i,fd;
	char key_val_array[2];
	int num;
	
    fd = open("/dev/rbt530_dev1", O_RDWR);  //opening of device 1
	 if(fd <0) {
		printf("cannot open the device1 file...\n");
	 }
	
	 
 	for(i = 0; i<50; i++){
		key_val_array[0]=ran();
		key_val_array[1]=ran();
		write(fd, key_val_array, 2);   //writing of values in to tree
	}
 	for(i = 0; i<50; i++){
	num=booleanfun();
	ioctl(fd, WR_DATA,(int32_t *)&num);  //randomly choosing ascending order or descending order
	result = read(fd,final_array,100);   //reading values from tree
	output1[j]=result;
	j++;
	}  
close(fd);  //closing file pointer
pthread_exit(NULL);

}
void* _inputData2(void* sleepTime){   //function for 1 and 2 threads
	int final_array[100],result=0,fd,i;
	char key_val_array[2];
	int num;
	
    fd = open("/dev/rbt530_dev2", O_RDWR);  //opening of device 2
	 if(fd <0) {
		printf("cannot open the device2 file...\n");
	 }
	
	 
 	for(i = 0; i<50; i++){
		key_val_array[0]=ran();
		key_val_array[1]=ran();
		write(fd, key_val_array, 2); //writing of values in to tree
	}
 	for(i = 0; i<50; i++){
	num=booleanfun();
	ioctl(fd, WR_DATA,(int32_t *)&num); //randomly choosing ascending order or descending order
	 result = read(fd,final_array,100);  //reading values from tree
	output2[k]=result; 
k++;  	
	}  
close(fd);
pthread_exit(NULL);

}

int main(int argc, char* argv[]){


	int thread_id1,thread_id2,thread_id3,thread_id4,i;  
	pthread_t pthd1, pthd2, pthd3, pthd4;
	int sleep_id1 = 7, sleep_id2 = 13,sleep_id3 = 7, sleep_id4 = 13;   //assining different sleep times
	
	thread_id1 = pthread_create(&pthd1, NULL, _inputData1, (void*)&sleep_id1);     //creating threads
	thread_id2 = pthread_create(&pthd2, NULL, _inputData1, (void*)&sleep_id2);
	thread_id3 = pthread_create(&pthd3, NULL, _inputData2, (void*)&sleep_id3);
	thread_id4 = pthread_create(&pthd4, NULL, _inputData2, (void*)&sleep_id4);
	
	
	if(thread_id1!=0)printf("Thread 1 creation failed");
	if(thread_id2!=0)printf("Thread 2 creation failed");
	if(thread_id3!=0)printf("Thread 3 creation failed");
	if(thread_id4!=0)printf("Thread 4 creation failed");

	pthread_join(pthd1, NULL); //joining thread 1
	pthread_join(pthd2, NULL);  //joining thread 2
	pthread_join(pthd3, NULL); //joining thread 3
	pthread_join(pthd4, NULL);  //joining thread 4
	
	printf("OUTPUT OF FIRST DEVICE \n");  //total of 100 write and read operations each
    for(i=0;i<100;i++){
		printf("%d \n",output1[i]);
	}
	
	printf("OUTPUT OF SECOND DEVICE \n");  //total of 100 write and read operations each
	    for( i=0;i<100;i++){
		printf("%d \n",output2[i]);
	}


	return 0;
}

int ran(void){   //function to generate random values
	int upper=100,lower=1;
	int num = (rand()%(upper - lower + 1)) + lower; 
	return num;
           
}
int booleanfun(void){  //function to generate either 1 or 0 for ascending and descending
	   if(rand()%2==0) 
          return(1);
	   else
	      return(0);
}




