#include "headers.h"

#define MAX_DEVICES     2      //2 devices

#define DEVICE_CLASS    "rbt530_DRV"
#define DEVICE         "rbt530_dev%d"
#define DEVICE_NAME_1 "rbt530_dev1"
#define DEVICE_NAME_2 "rbt530_dev2"
#define WR_DATA _IOW('a','a',int32_t *)  //command for writing data in to kernel space

static DEFINE_MUTEX(rb_mutex);


 // declarations
struct file *file;     
const char *buf;
size_t count;
loff_t *ppos;

struct inode *inode;
static dev_t rb_device_id; // Global variable for the rb_device_id device number 
static struct class *rb_class; // Global variable for the device class


struct rb_root;  //declaration of rb_root 
struct rb_object;  // declaration of rb_object 


struct cdev cdev;


struct rb_dev;  //declaration of rb_dev

int32_t val;

 int rb_open_1(struct inode *inode, struct file *file);
 int rb_release_1(struct inode *inode, struct file *file);
 ssize_t rb_write_1(struct file *file, const char *buf, size_t count, loff_t *ppos);
 long int rb_ioctl_1(struct file *file, unsigned int ioctl_num, unsigned long arg);
 ssize_t rb_read_1(struct file *file, char *buf, size_t count, loff_t *ppos);



 int rb_open_2(struct inode *inode, struct file *file);
 int rb_release_2(struct inode *inode, struct file *file);
 ssize_t rb_write_2(struct file *file, const char *buf, size_t count, loff_t *ppos);
 long int rb_ioctl_2(struct file *file, unsigned int ioctl_num, unsigned long arg);
 ssize_t rb_read_2(struct file *file, char *buf, size_t count, loff_t *ppos);

 int __init rbtree_init(void);
 void __exit rbtree_exit(void);
 char *set_devnode(struct device *dev, umode_t *mode);

struct rb_node *rb_search(struct rb_root *, int);   //declaration of rb_search
 int rb_insert(struct rb_root *, struct rb_object * );   //declaration of rb_insert






 
 


struct rb_dev {
	struct cdev cdev;	// kernel's internal structure that represents char devices
	struct rb_root rbt530;
    char name[20];                  /* Name of device*/
	char input[2];
	char output[100];
	struct rb_node *curr;  //currrent pointer holder
	long set_cmd;
} *rb_dev1, *rb_dev2;

typedef struct rb_object {     //structure definition for rb_object
	struct rb_node my_node;
	int key;
	char name[20];
	int data;
}rb_object_t;






// function to set the device default permission
 char *set_devnode(struct device *dev, umode_t *mode)
{
  if (!mode)
    return NULL;
  if (dev->devt == MKDEV(MAJOR(rb_device_id), 0) ||
            dev->devt == MKDEV(MAJOR(rb_device_id), 1))
    *mode = 0666;
  return NULL;
}

 int rb_open_1(struct inode *inode, struct file *file)   //open for devce 1
{ struct rb_dev *rb_dev1;
  printk(KERN_INFO "OPENING DEVICE 1");
  mutex_lock(&rb_mutex);  //locking device
  

	printk("RB: opening\n");
		
	// Get the per-device structure that contains this cdev */
	rb_dev1 = container_of(inode->i_cdev, struct rb_dev, cdev);   
		
	//Now we can access the driver struct elements.
	file->private_data = rb_dev1;
	printk("RB : %s is opening \n", rb_dev1->name);
  return 0;
}

 int rb_open_2(struct inode *inode, struct file *file)   //open for device 2
{  struct rb_dev *rb_dev2;
   printk(KERN_INFO "OPENING DEVICE 2");
   mutex_lock(&rb_mutex);  //locking device 2
  

	printk("RB: opening\n");
		
	// Get the per-device structure that contains this cdev */
	rb_dev2 = container_of(inode->i_cdev, struct rb_dev, cdev);
		
	//Now we can access the driver struct elements.
	file->private_data = rb_dev2;
	printk("RB : %s is opening \n", rb_dev2->name);
  return 0;
}


 int rb_release_1(struct inode *inode, struct file *file)  //relase for device 1
{	struct rb_dev *rb_dev1 = file->private_data;
	printk(KERN_INFO "RELEASING DEVICE 1");
	
	printk("%s is closing\n", rb_dev1->name);
	
	//release mutex lock.
	mutex_unlock(&rb_mutex);
 
  return 0;
}

 int rb_release_2(struct inode *inode, struct file *file) //release for device 2
{	struct rb_dev *rb_dev2 = file->private_data;
	printk(KERN_INFO "RELEASING DEVICE 2");
	
	printk("%s is closing\n", rb_dev2->name);
	
	//release mutex lock.
	mutex_unlock(&rb_mutex);
 
  return 0;
}


ssize_t rb_write_1(struct file *file, const char *buf, size_t count, loff_t *ppos)  //write for device 1
{       struct rb_object *my_rbtree;
	struct rb_node *searchVal;
 	struct rb_dev *rb_dev1 = file->private_data;
	int wr_err=0; 
	int keyVal=0, dataVal=0;
	wr_err = copy_from_user(rb_dev1->input,buf,count);  //copying data from user
	
	if(wr_err!=0){
		printk(KERN_INFO "RB :Error while writing data.\n");
		return -1;
	}

	keyVal = rb_dev1->input[0];
	dataVal = rb_dev1->input[1];
	
	my_rbtree = (struct rb_object *)kmalloc(sizeof(struct rb_object), GFP_KERNEL);  //allocation of space
	my_rbtree->key = keyVal;
	my_rbtree->data = dataVal;
	
	printk(KERN_INFO "RB :keyval = %d, dataVal = %d",keyVal, dataVal);
	printk(KERN_INFO "RB :Search if keyval exists... ");

	//SEARCH IF THE KeyVALUE EXISTS.
	searchVal = rb_search(&rb_dev1->rbt530,keyVal);
	
        if(searchVal == NULL)
		printk("RB : Key was not found in the tree");	

	if(searchVal){
		printk(KERN_INFO "RB : Key was found in tree");
		if(dataVal == 1) {
			printk(KERN_INFO "RB : Node replaced");
			rb_replace_node(searchVal, &my_rbtree->my_node, &rb_dev1->rbt530);  //replace node
		}
		else{
			//Erase a node.
			rb_erase(searchVal,&rb_dev1->rbt530);   //earsing of node
			printk(KERN_INFO "RB :Node is erased.");
		}
	}
	else{
		printk(KERN_INFO "RB : Node inserting %d ... \n",keyVal);	
		rb_insert(&rb_dev1->rbt530, my_rbtree);
	}
 
	return 0;  
}



 ssize_t rb_write_2(struct file *file, const char *buf, size_t count, loff_t *ppos)  //write for device 2
{	struct rb_object *my_rbtree;
	struct rb_dev *rb_dev2 = file->private_data;
	struct rb_node *searchVal;
	int wr_err=0; 
	int keyVal=0, dataVal=0;
	wr_err = copy_from_user(rb_dev2->input,buf,count);  //copy from user
	
	if(wr_err!=0){
		printk(KERN_INFO "RB :Error while writing data.\n");
		return -1;
	}

	keyVal = rb_dev2->input[0];
	dataVal = rb_dev2->input[1];
	
	my_rbtree = (struct rb_object *)kmalloc(sizeof(struct rb_object), GFP_KERNEL);  //allocating of space 
	my_rbtree->key = keyVal;
	my_rbtree->data = dataVal;
	
	printk(KERN_INFO "RB :keyval = %d, dataVal = %d",keyVal, dataVal);
	printk(KERN_INFO "RB :Search if keyval exists... ");

	//SEARCH IF THE KeyVALUE EXISTS.
	searchVal = rb_search(&rb_dev2->rbt530,keyVal);
	
        if(searchVal == NULL)
		printk("RB : Key was not found in the tree");	

	if(searchVal){
		printk(KERN_INFO "RB : Key was found in tree");
		if(dataVal == 1) {
			printk(KERN_INFO "RB : Node replaced");
			rb_replace_node(searchVal, &my_rbtree->my_node, &rb_dev2->rbt530);  //replace node
		}
		else{
			//Erase a node.
			rb_erase(searchVal,&rb_dev2->rbt530);
			printk(KERN_INFO "RB :Node is erased.");
		}
	}
	else{
		printk(KERN_INFO "RB : Node inserting %d ... \n",keyVal);	
		rb_insert(&rb_dev2->rbt530, my_rbtree);
	}
 
	return 0;  

}




 ssize_t rb_read_1(struct file *file, char *buf, size_t count, loff_t *ppos)  //read function for device 1
{
       		struct rb_dev *rb_dev1 = file->private_data;
		rb_object_t *structdata;
		int result;
		printk(KERN_INFO "RB : Reading for set_cmd = %ld",rb_dev1->set_cmd);

		if(rb_dev1->set_cmd==1){
               
				if(rb_dev1->curr==NULL)
				{
					if(rb_last(&(rb_dev1->rbt530))==NULL)
					{
						printk("Reading failed! - tree empty");
						return -1;
					}
					rb_dev1->curr = rb_last(&(rb_dev1->rbt530));  //last node
				}
				else
				{
					if(rb_prev(rb_dev1->curr)==NULL)
					{
						printk("Reading failed! - next object is NULL");
						return -1;
					}
					rb_dev1->curr = rb_prev(rb_dev1->curr);    //previos node
				}
			//return its value
		structdata = container_of(rb_dev1->curr, rb_object_t, my_node);
		result = structdata->key;
		printk(KERN_INFO "RB :Read next value (descending order) = %d", result);

		}
		 else if(rb_dev1->set_cmd==0){   //ascending order
			if(rb_dev1->curr==NULL)
			{
				if(rb_first(&(rb_dev1->rbt530))==NULL)
				{
					printk("Reading failed! - tree empty");
					return -1;
				}
				rb_dev1->curr = rb_first(&(rb_dev1->rbt530));    //first node
			}
			else
			{ 	if(rb_next(rb_dev1->curr)==NULL)
				{
					printk("Reading failed! - next object is NULL");
					return -1;
				}
				rb_dev1->curr = rb_next(rb_dev1->curr);
			}

			structdata = container_of(rb_dev1->curr, rb_object_t, my_node);
			result = structdata->key;
			printk(KERN_INFO "RB :Read next value (ascending order) = %d", result);
		}


		else
			result = -1;
	return result;
}
	 
 


 ssize_t rb_read_2(struct file *file, char *buf, size_t count, loff_t *ppos)  //read function for device 2
{
	   	struct rb_dev *rb_dev2 = file->private_data;
		int result;
		rb_object_t *structdata;
		printk(KERN_INFO "RB : Reading for set_cmd = %ld",rb_dev2->set_cmd);

		if(rb_dev2->set_cmd==1){     //descending order
               
				if(rb_dev2->curr==NULL)
				{
					if(rb_last(&(rb_dev2->rbt530))==NULL)
					{
						printk("Reading failed! - tree empty");
						return -1;
					}
					rb_dev2->curr = rb_last(&(rb_dev2->rbt530));
				}
				else
				{
					if(rb_prev(rb_dev2->curr)==NULL)
					{
						printk("Reading failed! - next object is NULL");
						return -1;
					}
					rb_dev2->curr = rb_prev(rb_dev2->curr);
				}
			//return its value
		structdata = container_of(rb_dev2->curr, rb_object_t, my_node);
		result = structdata->key;
		printk(KERN_INFO "RB :Read next value (descending order) = %d", result);

		}
		 else if(rb_dev2->set_cmd==0){
			if(rb_dev2->curr==NULL)
			{
				if(rb_first(&(rb_dev2->rbt530))==NULL)
				{
					printk("Reading failed! - tree empty");
					return -1;
				}
				rb_dev2->curr = rb_first(&(rb_dev2->rbt530));
			}
			else
			{ 	if(rb_next(rb_dev2->curr)==NULL)
				{
					printk("Reading failed! - next object is NULL");
					return -1;
				}
				rb_dev2->curr = rb_next(rb_dev2->curr);
			}

			structdata = container_of(rb_dev2->curr, rb_object_t, my_node);
			result = structdata->key;
			printk(KERN_INFO "RB :Read next value (ascending order) = %d", result);
		}

	
		else
			result =-1;
	return result;
}


 long int rb_ioctl_1(struct file *file, unsigned int ioctl_num, unsigned long arg)  //ioctl for function 1
{
	struct rb_dev *rb_dev1 = file->private_data;
	int rc;
	printk(KERN_INFO "RB : IOCTL CALLED FROM 1 DEVICE...");
       
        
switch(ioctl_num){
	case WR_DATA:
		rc=copy_from_user(&val,(int32_t*)arg,sizeof(int32_t));  //copying data from user
		rb_dev1->set_cmd=val;
		break;
	default:
		return -EINVAL;
		
	}
	printk(KERN_INFO "RB :set_cmd FOR DEVICE 1 = %ld \n",rb_dev1->set_cmd);
	return 0;
}

long int rb_ioctl_2(struct file *file, unsigned int ioctl_num, unsigned long arg) //ioctl for function 2
{	struct rb_dev *rb_dev2 = file->private_data;
	int rc;
	printk(KERN_INFO "RB : IOCTL CALLED FROM 2 DEVICE...");
        
        
 switch(ioctl_num){
	case WR_DATA:
		rc=copy_from_user(&val,(int32_t*)arg,sizeof(int32_t));  //copying data from user
		rb_dev2->set_cmd=val;
		break;
	default:
		return -EINVAL;
		
	}
	printk(KERN_INFO "RB :set_cmd FOR DEVICE 2 = %ld \n",rb_dev2->set_cmd);
	return 0;
}

 int rb_insert(struct rb_root *root, struct rb_object *structdata){  //inserting elements in to tree
	int cmpresult;
 	struct rb_node **new = &(root->rb_node), *parent=NULL;
	struct rb_object *this;

	while(*new){
	
		this = container_of(*new, rb_object_t, my_node);
		cmpresult= structdata->key > this->key ? 1 : 0;	
		parent = *new;
			
		if(cmpresult == 0)
			new =&((*new)->rb_left);
		else
			new= &((*new)->rb_right);
	
	}

	
	rb_link_node(&structdata->my_node, parent, new);
	rb_insert_color(&structdata->my_node,root);
	
	//printk("RB :Node inserted %d, parent is %d",structdata->key,this->key); 

	return 0;

}



struct rb_node *rb_search(struct rb_root *root, int keytoFind){  //function to search for node


	struct rb_node *walker = root->rb_node;

	while(walker){
	
		rb_object_t *structdata = container_of(walker, rb_object_t, my_node);
		
		if(keytoFind < structdata->key)
			walker = walker->rb_left;
		else if (keytoFind > structdata->key)
			walker = walker->rb_right;
		else
			return &structdata->my_node;
		}
	return NULL;
}








/* 
* file operation structure with desired file operations
* and to initialize the character device structure with that.
*/
static struct file_operations rb_fops_1 = {    //fops for device 1
	.owner            = THIS_MODULE,
	.open             = rb_open_1,
	.release          = rb_release_1,
	.write            = rb_write_1,
	.read             = rb_read_1,
	.unlocked_ioctl   = rb_ioctl_1,
};

static struct file_operations rb_fops_2 = {    //fops for device 2
	.owner            = THIS_MODULE,
	.open             = rb_open_2,
	.release          = rb_release_2,
	.write            = rb_write_2,
	.read             = rb_read_2,
	.unlocked_ioctl   = rb_ioctl_2,

};
 

 int __init rbtree_init(void) /* Constructor */
{
  int device_id;

  // create the character driver region
  if(alloc_chrdev_region(&rb_device_id, 0, MAX_DEVICES, DEVICE_CLASS) < 0) {
    return -1;
  }

  // create a device class
  if ((rb_class = class_create(THIS_MODULE, DEVICE_CLASS)) == NULL) {
    goto err_chrdev_reg;
    return -1;
  }

  // set the device class permission
  rb_class->devnode = set_devnode;

  // allocate memory inside the kernel for rb_dev
	rb_dev1 = kmalloc(sizeof(struct rb_dev), GFP_KERNEL);
	if(!rb_dev1) {
		printk("Couldn't allocate memory rb_dev1\n");
		return -ENOMEM;
	}

	// allocate memory inside the kernel for rb_dev
	rb_dev2 = kmalloc(sizeof(struct rb_dev), GFP_KERNEL);
	if(!rb_dev2) {
		printk("Couldn't allocate memory rb_dev2\n"); 
		return -ENOMEM;
	}

  // create the devices
  for(device_id=0; device_id< MAX_DEVICES;device_id++) {
    if (device_create(rb_class, NULL, MKDEV(MAJOR(rb_device_id), device_id), NULL, DEVICE, device_id+1) == NULL) {
      goto err_class_create;
      return -1;
    }
  }

  cdev_init(&rb_dev1->cdev, &rb_fops_1);
  cdev_init(&rb_dev2->cdev, &rb_fops_2);

  if (cdev_add(&rb_dev1->cdev, MKDEV(MAJOR(rb_device_id), 0), 1) == -1) {
    device_destroy(rb_class, MKDEV(MAJOR(rb_device_id), 0));
    goto err_class_create;
    return -1;
  }

  if (cdev_add(&rb_dev2->cdev, MKDEV(MAJOR(rb_device_id), 1), 1) == -1) {
    device_destroy(rb_class, MKDEV(MAJOR(rb_device_id), 1));
    goto err_class_create;
    return -1;
  }
  
  sprintf(rb_dev1->name, DEVICE_NAME_1);
  sprintf(rb_dev2->name, DEVICE_NAME_2);
  
  rb_dev1->rbt530=RB_ROOT;
  rb_dev2->rbt530=RB_ROOT;
  
  printk(KERN_INFO "rb_tree driver is registered\n");
  return 0;
err_class_create:
  class_destroy(rb_class);
err_chrdev_reg:
  unregister_chrdev_region(rb_device_id, MAX_DEVICES);
  return 0;
}
 
 void __exit rbtree_exit(void) /* Destructor */
{
  // destroy the created MAX_DEVICES
  device_destroy(rb_class, MKDEV(MAJOR(rb_device_id), 0));
  device_destroy(rb_class, MKDEV(MAJOR(rb_device_id), 1));

  class_unregister(rb_class);

  // destroy the created class
  class_destroy(rb_class);

  // delete the character dev class
  cdev_del(&rb_dev1->cdev);
  cdev_del(&rb_dev2->cdev);

  // free the allocated memory
  kfree(rb_dev1);
  kfree(rb_dev2);

  // unregister the character region
  unregister_chrdev_region(rb_device_id, MAX_DEVICES);

  printk(KERN_INFO "rb_tree driver is deregistered\n");
}

module_init(rbtree_init);
module_exit(rbtree_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Sri hari");
MODULE_DESCRIPTION("RBt530-tree driver");

